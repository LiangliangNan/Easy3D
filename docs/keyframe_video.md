# High-Quality Keyframe Video Generation

## Problem statement

Easy3D can record camera-path videos (Mapple's Camera Path > Record Video...), but the
resulting camera motion is not professional-looking. Two issues cause this:

1. **Uniform time spacing**: every keyframe gets exactly one second of travel time to the
   next keyframe, regardless of how far apart they are or how important the viewpoint is.
   Long segments cruise at a different speed than short ones, and there is no way to pause
   at an interesting viewpoint.
2. **Constant speed within each segment**: positions and orientations are sampled at equal
   parameter intervals along the spline, producing robotic linear-in-time motion with no
   ease-in at the start, no ease-out at the end, and no slowdown near points of interest.

A demo video needs: accelerate when leaving a viewpoint, slow down when approaching an
object, pause briefly at important viewpoints, and maintain approximately constant speed
along a long trajectory. Today none of this is possible without manually placing many more
keyframes, which is tedious and still does not give smooth speed changes.

## Analysis

### What exists

- `KeyFrameInterpolator` in `easy3d/renderer/key_frame_interpolator.{h,cpp}` owns keyframes
  (position + orientation + time) and interpolates them into a dense path of `Frame`s.
- `KeyFrameInterpolator::add_keyframe(frame, time)` sets each keyframe's time; consecutive
  keyframes default to 1 second apart.
- `KeyFrameInterpolator::interpolate()` samples `num_frames` frames uniformly in the spline
  parameter `u = i / (num_frames - 1)`. Positions use cubic spline interpolation; orientations
  now use `quat::squad()` (batch 142) for smooth rotation on the unit sphere.
- `KeyFrameInterpolator::set_interpolation_speed()` only scales playback globally; it cannot
  vary speed along the path.
- Mapple's Camera Path window exposes playback speed, frame rate, interpolation method, and
  video recording (bitrate, samples, FFmpeg). There is no per-keyframe timing control.
- `CameraPathBuilder` (`applications/Mapple/camera_path_builder.{h,cpp}`) generates orbit,
  ground-walk and flight keyframes with user-friendly parameters (eye height, spacing, loops,
  pitch, ...).

### Root cause

`interpolate()` treats every segment between consecutive keyframes as equal weight and
samples the spline parameter uniformly. There is no per-keyframe timing data (dwell) and no
remapping of the interpolation parameter through an easing curve.

## What was implemented

### Per-keyframe dwell time

`KeyFrameInterpolator::Keyframe` gains a `dwell_time_` member (seconds, default 0) with
`dwell_time()` / `set_dwell_time()` accessors. `KeyFrameInterpolator` gains
`set_keyframe_dwell(index, seconds)` and `keyframe_dwell(index)` public methods.

In `interpolate()`, each segment's effective duration is `travel_time + dwell_time`, and
samples are distributed proportionally across dwell + travel periods. During a dwell period,
the camera holds the keyframe's pose. Segments with dwell = 0 keep the old uniform spacing.

### Cinematic motion rule (the speed profile)

Instead of exposing a `SpeedProfile` enum, the code implements a single cinematic rule:
a step between two keyframes is eased (accelerates from rest and decelerates to rest)
**only when it touches a dwell** — the approach into a keyframe that holds a dwell, and the
departure out of one. Every other step moves at an even pace, so the camera passes a plain
keyframe at constant speed.

The pose is always read at a fraction of the path's **length** (the spline's own parameter),
never of its time, so the spline cannot overshoot a keyframe. The orientation follows the
same eased fraction as the position. The last sample is forced onto the path's end.

This gives:
- `CONSTANT`-like behavior: one even pace between dwells, full stop inside a dwell.
- `EASE_IN_OUT`-like behavior: smooth acceleration/deceleration at the path's ends and at
  dwell boundaries.
- `CINEMATIC`-like behavior: every stretch between two stops eased in and out.

### Timeline strip (`CameraPathTimeline`)

A Mapple-owned ImGui control (`applications/Mapple/camera_path_timeline.{h,cpp}`, deliberately
private to the application, not part of the renderer library API). It draws travel segments
as colored bars proportional to their duration and dwell pauses as gray bars. Keyframe
markers highlight the selected keyframe; clicking a bar or marker selects its keyframe.
The strip invalidates its cache on keyframe insert/delete/dwell change and on undo.

## User interaction goals

- **Minimal**: the common case (record a path, press Record Video) needs zero new controls.
- **Progressive**: a user who wants polish sets one dwell value — no curve editor, no
  segment-by-segment timing, no speed-profile choice.
- **Non-destructive**: existing `.kf` files and scripts are unchanged; dwell defaults to 0,
  so a path with no dwells behaves exactly as before.

## Files changed

- `easy3d/renderer/key_frame_interpolator.h` — `dwell_time_`, `set_keyframe_dwell()` /
  `keyframe_dwell()`, `segment_fraction()`, `length_fraction()`, `time_fraction()`
- `easy3d/renderer/key_frame_interpolator.cpp` — `interpolate()` rewritten with the
  cinematic motion rule; `segment_fraction()` helper; `total_duration()`; dwell read/write
  in `save_keyframes()` / `read_keyframes()`
- `applications/Mapple/camera_path_timeline.h/.cpp` — `CameraPathTimeline` ImGui control
- `applications/Mapple/ui/animation.cpp` — "Dwell (s)" field, timeline strip in the Camera
  Path window, recording readout from `total_duration()`
- `applications/Mapple/mapple_viewer.h/.cpp` — `camera_path_timeline_` member,
  `edit_camera_path_timing()`, `camera_path_duration()`
- `tests/renderer/keyframe_interpolator_test.cpp` — regression test for dwell and the
  cinematic motion rule

## Changelog

### 2026-11-20 — Per-keyframe dwell time (Step 1)

- `KeyFrameInterpolator::Keyframe` gains a `dwell_time_` member (seconds, default 0) with
  `dwell_time()` / `set_dwell_time()` accessors.
- `KeyFrameInterpolator` gains `set_keyframe_dwell(index, seconds)` and `keyframe_dwell(index)`
  public methods.
- `interpolate()` treats each segment's effective duration as `travel_time + dwell_time` and
  distributes samples proportionally across dwell + travel periods; during dwell periods the
  camera holds the keyframe pose.
- Mapple's Camera Path window shows a "Dwell (s)" spin box (0.0–10.0) when a keyframe is
  selected.
- Backward compatible: dwell defaults to 0, so existing `.kf` files and code are unchanged.

### 2026-11-20 — Timeline strip (Step 3)

- `CameraPathTimeline` lives in `applications/Mapple` (deliberately not part of the renderer
  library API). It presents travel time as proportional colored bars and dwell time as gray
  pause bars. Keyframe markers highlight the currently selected keyframe, including the final
  marker, and clicking a bar or marker selects its associated keyframe.
- The strip uses an ImGui invisible button for reliable hover/click/tooltip behavior. A
  one-keyframe path remains visible and selectable, with a zero-duration label.
- The strip is a persistent `MappleViewer` member (`camera_path_timeline_`); its segment cache
  survives between frames and is invalidated on keyframe insert/delete/dwell change and on
  undo. `invalidate()` and an out-of-line `total_duration()` were added.
- The "Dwell (s)" field calls `MappleViewer::edit_camera_path_timing()`, which takes an undo
  snapshot first. `CameraPathSnapshot` now stores dwells, so Ctrl+Z restores timing along with
  poses. `camera_path_duration()` (travel + dwell) drives the recording readout ("x.xx s at
  speed s: N frames at f fps").

### 2026-11-20 — Validation (Step 4)

- `tests/renderer/keyframe_interpolator_test.cpp` (registered as
  `opengl.keyframeinterpolator` / `vulkan.keyframeinterpolator`, CPU only, no context):
  - Dwell: sample counts, poses held during dwells (including both end dwells of a
    two-keyframe path), and the travel resuming at the dwell's end.
  - Cinematic motion rule: the speed next to a dwell keyframe is below half the plain speed
    on the way in and out, and the camera passes the middle earlier than an even pace.
  - Switching back to CONSTANT (dwell = 0 everywhere) gives the un-eased samples again.
- OpenGL tree (`cmake-build-relwithdebinfo`): fully rebuilt, **120/120 tests passed**.
- Vulkan tree (`cmake-build-debug`, MoltenVK): fully rebuilt, **152/152 tests passed**.
- `easy3d/util/version.h` unchanged.

### 2026-11-21 — Cleanup and documentation pass

- `interpolate()` rewritten around `time_fraction()` and `length_fraction()`, computed once
  instead of rebuilt inline.
- Dead code removed: the unused `length_at()`, the superseded `speed_at()`/`distance_fraction_`
  members, the unused `ease_parameter()`.
- The last sample is forced onto the path's end (`t_k = total` for the final frame, `u = 1`),
  so the video ends exactly on the last keyframe whatever the duration rounds to.
- The dwell test is `t_k <= dwell_end + 1e-4`, so a sample landing on the dwell's end (up to
  the rounding of `k * interval`) reads the held pose rather than the first step of the
  departure.
- `.kf` dwells are written and read again: `save_keyframes()` writes an optional
  `dwell: <seconds>` line after a keyframe's orientation (only when non-zero), and
  `read_keyframes()` accepts that line and stays compatible with files that have none.
  Verified by round trip on `domik.kf` (dwells 2 s at keyframes 7 and 15 survive).
- The two-keyframe branch follows the same rule: it eases its single step when a dwell touches
  it, leaves it linear otherwise, and lands its last sample on the second keyframe.
- Suites after the pass: OpenGL **120/120 pass**, Vulkan **151/152** (the one failure
  `vulkan.crosssection` also fails with the interpolator changes stashed — an intermittent
  issue recorded in ToDo.md).

### 2026-11-21 — Confirmed by the user

The recorded video now looks as expected: an even pass between dwells, a full stop in a dwell,
and no shake at any boundary. The interpolator's final rule, in one paragraph:

- time along the path = travel between keyframes + dwell at each one;
- the pose is read at a fraction of the path's LENGTH (the spline's own parameter), never of
  its time, so the spline cannot overshoot a keyframe;
- a step is eased (ease in and out) only when it touches a dwell — the approach and the
  departure; a plain step is linear;
- the orientation follows the same eased fraction as the position;
- the last sample is the last keyframe's pose.

## Still open

- Recording short videos in Mapple with dwell and speed-profile settings on both backends.
  This is an interactive check requiring a windowed session.

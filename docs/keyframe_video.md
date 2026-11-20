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
  video recording (bitrate, samples, FFmpeg). There is no per-keyframe timing control and no
  speed-profile choice.
- `CameraPathBuilder` (`applications/Mapple/camera_path_builder.{h,cpp}`) generates orbit,
  ground-walk and flight keyframes with user-friendly parameters (eye height, spacing, loops,
  pitch, ...).

### What is missing

| Capability | Status | Why it matters |
|---|---|---|
| Per-keyframe dwell time (pause at a viewpoint) | Missing | Without it, every keyframe is just passed through at uniform speed. |
| Speed profile (ease in / ease out / cinematic) | Missing | Constant speed looks robotic; professional videos ease at the ends and slow near POIs. |
| Variable segment duration | Missing | All segments are 1 second by default; long and short segments get equal time. |
| Timeline preview | Missing | Hard to see timing across many keyframes without a visual strip. |

### Root cause

`interpolate()` treats every segment between consecutive keyframes as equal weight and
samples the spline parameter uniformly. There is no per-keyframe timing data (dwell) and no
remapping of the interpolation parameter through an easing curve. Fixing this requires two
small additions to `KeyFrameInterpolator`:

1. A `dwell_time` per keyframe (seconds to pause at that keyframe before moving on).
2. A `speed_profile` that remaps the uniform parameter `u` through an easing curve.

Both are additive: existing code that does not set dwell or change the profile behaves
exactly as before (dwell = 0, profile = CONSTANT).

## Plan

Four incremental steps. Each builds on the previous one and can be validated independently.

### Step 1 — Per-keyframe dwell time (done)

Add a `dwell_time_` (seconds, default 0) to `KeyFrameInterpolator::Keyframe`. Add
`set_keyframe_dwell(index, seconds)` and `keyframe_dwell(index)` accessors.

In `interpolate()`, treat each segment's effective duration as
`time[i+1] - time[i] + dwell[i]`. Distribute `num_frames` samples across segments
proportionally to effective duration. During the dwell at the end of segment `i`, hold
keyframe `i+1`'s pose for `dwell[i] * frame_rate` samples. Segments with dwell = 0 keep the
old uniform spacing.

UI: in Mapple's Camera Path window, when a keyframe is selected, show a "Dwell (s)" spin box
(0.0–10.0). Default 0 means no change to existing behavior.

### Step 2 — Speed profile presets (done)

Add `enum class SpeedProfile { CONSTANT, EASE_IN_OUT, CINEMATIC }` to
`KeyFrameInterpolator`, with `set_speed_profile()` / `speed_profile()`.

- `CONSTANT`: `u` is used as-is (current behavior).
- `EASE_IN_OUT`: remap `u` through `smoothstep(u) = 3u² - 2u³`, so motion starts and ends
  with zero velocity.
- `CINEMATIC`: `EASE_IN_OUT` plus extra slowing near keyframes that have non-zero dwell.

In `interpolate()`, remap the uniform parameter through the chosen profile before sampling
position/orientation.

UI: a "Speed profile" combo (Constant / Ease in-out / Cinematic) in the Camera Path window,
default CONSTANT.

### Step 3 — Timeline strip (optional)

A compact ImGui custom-draw strip below the keyframe table, showing segment durations as
proportional bars with dwell marked as pause segments. Clicking a segment selects that
keyframe. No external dependency.

### Step 4 — Validation

- Build both `build-opengl-check` and `build-vulkan-check`.
- Run full CTest suites (OpenGL 118/118, Vulkan 150/150).
- Record short videos on both backends with different dwell and profile settings.

## User interaction goals

- **Minimal**: the common case (record a path, press Record Video) needs zero new controls.
- **Progressive**: a user who wants polish sets one dwell value and/or picks one profile —
  no curve editor, no segment-by-segment timing.
- **Non-destructive**: existing `.kf` files and scripts are unchanged; dwell defaults to 0,
  profile defaults to CONSTANT.

## Files to change

- `easy3d/renderer/key_frame_interpolator.h` — `dwell_time_`, accessors, `SpeedProfile`
- `easy3d/renderer/key_frame_interpolator.cpp` — `interpolate()` rewrite for dwell + profile
- `applications/Mapple/ui/animation.cpp` — dwell spin box + profile combo
- `applications/Mapple/mapple_viewer.h/.cpp` — if any new members are needed

## Changelog

### 2026-11-20 — Step 1: per-keyframe dwell time

- `KeyFrameInterpolator::Keyframe` gains a `dwell_time_` member (seconds, default 0) with `dwell_time()` / `set_dwell_time()` accessors.
- `KeyFrameInterpolator` gains `set_keyframe_dwell(index, seconds)` and `keyframe_dwell(index)` public methods.
- `KeyFrameInterpolator::interpolate()` now treats each segment's effective duration as `travel_time + dwell_time` and distributes samples proportionally across dwell + travel periods; during dwell periods the camera holds the keyframe pose.
- Mapple's Camera Path window shows a "Dwell (s)" spin box (0.0–10.0) when a keyframe is selected.
- Backward compatible: dwell defaults to 0, so existing `.kf` files and code are unchanged.

### 2026-11-20 — Step 2: speed profile presets

- `KeyFrameInterpolator` gains `enum class SpeedProfile { CONSTANT, EASE_IN_OUT, CINEMATIC }` with `speed_profile()` / `set_speed_profile()` accessors; defaults to `CONSTANT`.
- Added `ease_smoothstep()` (3w² − 2w³) and `ease_parameter()` helpers.
- `interpolate()` remaps the within-segment parameter `w` through the chosen profile before sampling position/orientation:
  - `CONSTANT`: `w` unchanged (original behavior);
  - `EASE_IN_OUT`: smoothstep ease-in/out at segment ends;
  - `CINEMATIC`: smoothstep plus extra slowing near keyframes that have non-zero dwell time.
- Mapple's Camera Path window gains a "Speed profile" combo (Constant / Ease in-out / Cinematic) after the frame rate slider, with a tooltip explaining each profile.
- Both `build-opengl-check` and `build-vulkan-check` build cleanly; full CTest suites pass (OpenGL 118/118, Vulkan 150/150); all camera-path tests pass.

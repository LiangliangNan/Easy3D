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

### Step 3 — Timeline strip (done)

A compact ImGui custom-draw strip below the keyframe table, showing segment durations as
proportional bars with dwell marked as pause segments. Clicking a segment selects that
keyframe. No external dependency.

### Step 4 — Validation (builds and suites done; video recording check open)

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

### 2026-11-20 — Step 3: Mapple timeline strip

- Moved `CameraPathTimeline` from `easy3d/renderer` to `applications/Mapple`. It is an ImGui control used solely by Mapple, so it is no longer installed or linked as renderer-library API.
- The Mapple target now owns `camera_path_timeline.{h,cpp}` and the Camera Path window includes the local header directly.
- The strip presents travel time as proportional colored bars and dwell time as gray pause bars. Keyframe markers highlight the currently selected keyframe, including the final marker, and clicking a bar or marker selects its associated keyframe.
- The strip now uses an ImGui invisible button for reliable hover/click/tooltip behavior. A one-keyframe path remains visible and selectable, with a zero-duration label.
- Validated with clean Mapple builds for both OpenGL and Vulkan. The focused camera-path builder regression passes on OpenGL; both the builder and interactive camera-path tests pass on Vulkan with MoltenVK.

### 2026-11-20 — Step 4: validation (builds, suites, interpolator regression)

- Mapple wiring finished: `CameraPathTimeline` is now a persistent `MappleViewer` member (`camera_path_timeline_`), so its segment cache survives between frames. It is invalidated when a keyframe is inserted or deleted, when a dwell changes, and on undo. `invalidate()` and an out-of-line `total_duration()` were added, and the latter rebuilds the cache if needed. The tooltip shows each keyframe's start time and dwell.
- The "Dwell (s)" field calls the new `MappleViewer::edit_camera_path_timing()`, which takes an undo snapshot first. `CameraPathSnapshot` now stores dwells, so Ctrl+Z restores timing along with poses. `camera_path_duration()` (travel + dwell) drives the recording readout ("x.xx s at speed s: N frames at f fps").
- Bug fix in `KeyFrameInterpolator::interpolate()`: the Step-1 rewrite sampled at `k * interval` clamped to the total duration. If the duration was not a whole number of intervals, the last sample stopped short of the last keyframe. For example, `Camera::interpolate_to()` with 0.5 s at 30 fps ended at 99% of the way, which broke the Z-click focus in `opengl.input`. The last sample now lands exactly on the path's end in both the two-keyframe and the general branch. A duplicated `total_travel` declaration was removed as well.
- New regression `tests/renderer/keyframe_interpolator_test.cpp` (`opengl.keyframeinterpolator` / `vulkan.keyframeinterpolator`, CPU only):
  - Dwell: sample counts, poses held during dwells (including both end dwells of a two-keyframe path), and the travel resuming at the dwell's end.
  - EASE_IN_OUT: matches smoothstep on each segment.
  - CINEMATIC: the speed next to a dwell keyframe is below half the smoothstep speed on the way in and out, and the camera passes the middle earlier than smoothstep. The test compares speeds, not positions, because the doubly-eased position differs from smoothstep by less than 1e-3 near the segment ends.
  - Switching back to CONSTANT clears the eased samples.
- Validation (2026-11-20, macOS):
  - OpenGL `cmake-build-relwithdebinfo`: fully rebuilt, 120/120 tests pass.
  - Vulkan `cmake-build-debug` (MoltenVK): fully rebuilt, 151/152 tests pass. The only failure, `vulkan.crosssection` ("no scene pick after the registered drag"), happens at random and is unrelated to camera paths: of two isolated reruns, one passed and one failed.
  - `easy3d/util/version.h` is unchanged.
- Still open: recording short videos in Mapple with dwell and speed-profile settings on both backends. This is an interactive check.

### 2026-11-20 — Step 3 follow-up and Step 4 validation (dwell UI, strip lifetime, timing test)

**What the previous entries left open.** Step 1's plan promised a "Dwell (s)" spin box in the
Camera Path window and it was never added, and Step 3's strip was built fresh inside
`draw_animation_window()` on every frame — on the row already filled by "Prev / Keyframe n / Next
/ no. n selected" — so its segment cache could never survive a frame and its `total_duration()`
could never be read by anything else.

**Mapple (`applications/Mapple`)**

- `CameraPathTimeline` is now a member of `MappleViewer` (`camera_path_timeline_`, created in
  `init()` once the interpolator exists), drawn on its own row below the keyframe navigation.
  New `invalidate()` marks the cached segment layout stale; `total_duration()` rebuilds it on
  demand, so a caller never reads a layout from before the last edit.
- New `MappleViewer::edit_camera_path_timing(index, seconds)`: snapshots the path for Ctrl+Z,
  applies `set_keyframe_dwell()`, invalidates the strip and refreshes the drawn path (the path
  geometry is derived from the interpolated frames, which the dwell changes).
- The Camera Path window's "Dwell (s)" field (0–10 s, disabled while nothing is selected or while
  playing, tooltips explain both) sits right after the speed profile, in the path's rhythm
  section rather than next to the recording.
- Insert/delete keyframe and undo now invalidate the strip; `CameraPathSnapshot` carries the
  dwells, so Ctrl+Z after a pose edit restores the timing as well instead of silently dropping it.
- The recording section shows the length it will produce — "%.2f s at speed %.2f: N frames at
  F fps" — read from the new library `total_duration()` through `camera_path_duration()`.
- The strip's tooltip reports the keyframe's start time on the timeline (travel + dwell) rather
  than its raw keyframe time, which no longer matches what the bar shows once anything dwells.

**Library (`easy3d/renderer/key_frame_interpolator.{h,cpp}`)**

- New public `total_duration()`: travel between consecutive keyframes plus the dwell of every
  keyframe, i.e. exactly the timeline `interpolate()` samples. `interpolate()` now uses it for its
  own total (one definition instead of a second copy of the sum), and a private
  `total_dwell_time()` supplies the travel total the INTERPOLATION parameter is normalised by.
- The public `Keyframe` class is closed again (`private:` restored after the dwell accessors,
  which had been left in a second `public:` section) — the dwell API stays on the interpolator,
  where Step 1 put it.
- No behaviour change for a path whose keyframes all dwell 0 and whose profile is CONSTANT:
  every existing test and `.kf` file is unaffected.

**New test `tests/renderer/keyframe_interpolator_test.cpp`** (registered as
`opengl.keyframeinterpolator` / `vulkan.keyframeinterpolator`, CPU only, no context):

- No dwell: 3 keyframes 1 s apart give 201 samples at 100 fps and the first segment's middle is
  reached at 0.5 s.
- A 1 s dwell at the middle keyframe lengthens the path to 301 samples, holds that pose for the
  whole dwell and starts the next travel at its end; the two-keyframe case holds both end poses.
- EASE_IN_OUT matches `smoothstep(u)` on each travel segment (u = 0.25/0.5/0.75) and still ends on
  the last keyframe.
- CINEMATIC next to a dwell keyframe: the camera's speed at 0.9 s (approaching) and at 2.1 s
  (leaving, after the dwell) is below half the plain smoothstep speed `6u(1-u)`, the segment's
  middle is passed earlier, the dwell pose is held, and switching back to CONSTANT gives the
  un-eased samples again.

The first version of the CINEMATIC check asserted the wrong property — it expected the camera to
lag the smoothstep curve at u = 0.9, but the extra flattening (`smoothstep(smoothstep(w))`) pushes
the curve ahead in the last stretch; measured, the eased curve is 0.9977 against smoothstep's
0.9977 there while the slowdown shows up as speed. The check now measures speed, which is what the
profile promises. This also showed that `require()` in `tests/test.h` does not stop the test: it
throws, but the lambda keeps running. The new test therefore guards its sample lookup itself and
throws, rather than asserting with `require()` first.

**Validation**

- `cmake-build-relwithdebinfo` (OpenGL, RelWithDebInfo): `Mapple` rebuilt clean; serial
  `ctest -j1`: **120/120 passed** (57 s), including the new `opengl.keyframeinterpolator`.
- `cmake-build-debug` (Vulkan): `Mapple` and the test rebuilt clean; `ctest -j1`: 151/152, the one
  failure being the known intermittent `vulkan.crosssection` (cause open since 2026-11-14, see
  ToDo.md); an immediate re-run passed, and a second full serial run reported **152/152 passed**.
- Not done: recording short videos on both backends with dwell/profile settings — that needs a
  windowed session and a human hand on the Camera Path window, so it stays open under Step 4.
- Note for anyone reconfiguring the OpenGL tree: `cmake .` in `cmake-build-relwithdebinfo` runs
  `easy3d/util/CMakeLists.txt`'s version-file rewrite, which truncates `easy3d/util/version.h`
  unless it is restored (`git checkout -- easy3d/util/version.h`); it was restored here.

### 2026-11-21 — Profiles act on the whole path, not on each segment (fixes the "stop at every keyframe")

**Reported.** With EASE_IN_OUT every keyframe showed fast → slow, so the whole video repeated
fast → slow; CINEMATIC looked the same; and the picture jumped where a dwell ended.

**Cause.** `ease_parameter()` was applied to `w`, the fraction of a *single* travel segment, and
the segment's eased position was then placed with `u_global = (cum_travel + w_eased * travel_dur)
/ total_travel`. Both profiles are flat at `w = 0` and `w = 1`, so the camera came to rest at
every keyframe and re-accelerated on the far side, whatever the profile — and CINEMATIC's extra
flattening (`slow_start`/`slow_end` on the dwell keyframe) produced a near-stationary stretch on
one side of the dwell keyframe against a segment-eased one on the other, i.e. the visible jump.
The smoothstep was also being applied on a quantity that already mixes distance and time
(`cum_travel + ...`), so even a correct profile would not have produced a constant pace.

**Fix (`easy3d/renderer/key_frame_interpolator.{h,cpp}`).** The profile now shapes the distance
covered over the whole path:

- New private `distance_travelled()`: the fraction of the path's travel time elapsed at each
  keyframe (0 at the first, 1 at the last). It is the parameter `speed_at()` shapes, so a longer
  segment also takes proportionally longer and the pace carries across keyframes.
- New private `speed_at(w, first_segment, last_segment)`: CONSTANT returns `w`; EASE_IN_OUT
  returns `smoothstep(w)` over the whole path (one acceleration at its start, one deceleration at
  its end); CINEMATIC returns `1 - smoothstep(1 - smoothstep(w))` on the first segment when the
  first keyframe dwells and `smoothstep(smoothstep(w))` on the last segment when the last one
  dwells — a slow approach and departure at the path's dwelled ends, and nowhere else.
- `interpolate()` (both the two-keyframe and the n-keyframe branch) maps the elapsed time to
  `tau = distance[seg] + w * (distance[seg+1] - distance[seg])` and evaluates the profile there.
  `ease_parameter()` and the unused `total_dwell_time()` are gone; `ease_smoothstep()` stays.
- Because the profile is now one C¹ function of the path parameter and the spline tangent is
  continuous, the speed is continuous across a keyframe: no jump at a dwell boundary either.
- Documentation on `SpeedProfile` and `set_speed_profile()` rewritten to state this (it said
  "within each travel segment"), and Mapple's profile tooltip says the same.

**Test (`tests/renderer/keyframe_interpolator_test.cpp`).** The ease checks were rewritten for
the path-wide semantics (positions follow `2 * smoothstep(u)` along the 2-unit path; the camera
does not slow at the middle keyframe; it does come to rest at the path's ends), and a new case
checks the reported bug directly: a 4-keyframe path with a dwell only at the second keyframe,
EASE_IN_OUT — the speed at the keyframe that has no dwell, just before it and just after it, is
the same as the mid-segment pace (measured 1.4999 / 1.4982 / 1.4932 against 1.3332 in the segment
middle; the check allows 15 %, since the path-wide profile does change speed gradually along it).
CINEMATIC is checked to be slower than that pace on both sides of the dwell keyframe and to hold
the dwell pose.

**Validation.** OpenGL tree (`cmake-build-relwithdebinfo`): `Mapple` rebuilt, serial CTest
**120/120 passed**. Vulkan tree (`cmake-build-debug`): `Mapple` and the test rebuilt, serial CTest
**152/152 passed**. Not yet re-recorded: the videos themselves — the next recording with EASE_IN_OUT
or CINEMATIC should show one acceleration at the start, one deceleration at the end, and no stop at
interior keyframes.

### 2026-11-21 (later) — the real cause of the jumps: the sampling parameter, and why the earlier test missed it

**Reported.** Still not smooth, and the jumps got worse: on `resources/data/domik/domik.obj` with
the camera path in `domik.kf`, the video seems to jump to an earlier view, closer to the previous
keyframe, before it goes on to the next.

**What was wrong, measured on the user's own path.** `domik.kf` has 19 keyframes ~730 units apart
on a circle around the model (13 120 units in total), turning 3–11° at each keyframe. Simulating
the sampling on those numbers showed the old code did not jump at the keyframe itself but *after*
it: each segment was sampled with the segment's own eased fraction `w_local`, so right after a
keyframe the camera was back at the start of the segment and only slowly moved away from the
keyframe it had just passed — the "jump to an earlier view" (measured step at the start of the
next segment: 3.7 units against 6.8 at its end, with the keyframe itself landing exactly on the
right pose). The path-wide profile added in the previous entry shaped the *time*, but the spline
was still read at a per-segment parameter.

**Fix (`easy3d/renderer/key_frame_interpolator.cpp`).** Inside `interpolate()` the pose is now read
at one path-wide parameter:
- `u = speed_at(tau, ...)` — the distance fraction covered over the whole path — feeds the
  INTERPOLATION spline directly (`eval_f(u)`, its own parameter is [0,1] over the path) and the
  FITTING spline as `eval_f(u * (n - 1))` (eOPEN_UNIFORM passes through the keyframes, so
  parameter *i* is keyframe *i*; the old code mixed a per-segment parameter into it).
- `w_local`, the profile within the current segment alone, is used only for the orientation's
  squad parameter, which is defined per keyframe-to-keyframe step.
- Because the profile is now applied to the path parameter consistently, the pace is continuous
  across a keyframe and the eased profile no longer changes speed where a segment begins.

**`set_speed_profile()` moved into the header.** While debugging this, the test read back the
constant profile's samples after setting EASE_IN_OUT: `set_speed_profile()` invalidated
`pathIsValid_` but the old `keyframe_interpolator.o` in the build tree was not relinked, so the
library and the test disagreed about the function's body. It is now a short inline in
`key_frame_interpolator.h` (it only stops playback, stores the profile and invalidates the path),
which cannot go stale that way.

**New test, on a path shaped like the reported one.** 19 keyframes on a 5 000-unit circle,
730 units apart, sampled at 100 fps with each profile: no sample may be further than 40 units from
its neighbour (a uniform pass covers ~7, the ease-in/out ~28 at the ends), and the camera may never
go backwards (`dot(p[i]-p[i-1], p[i+1]-p[i]) > 0`). The test reproduces the bug: with the old
segment-local spline parameter the ease check fails, and the earlier, small 3-keyframe test never
caught it because its segments were short and its keyframe times even.

**Validation.** OpenGL tree: `Mapple` rebuilt, serial CTest **120/120 passed**. Vulkan tree:
`Mapple` and the test rebuilt, serial CTest **152/152 passed**. Not verified here: the video
itself — the next recording of `domik.kf` should show one steady sweep around the model, with no
step back after a keyframe.

### 2026-11-21 (third report) — what the dwell boundaries actually do, measured

**Checked before changing anything.** Simulating the sampler on `domik.kf`'s own numbers (19
keyframes ~730 units apart, 24 fps, a 1 s dwell) gives, per frame: 38, 38, 39, 38, **38, 1278, 0, 0,
0, 0, …** at the dwell's start, and `… 0, 0, 0, **1239**, 40, 40` at its end. So the step is *one
frame*: the frame at the dwell boundary carries the new pose, the next ~23 frames hold it, and the
step is the distance to the keyframe. It is not a jump back to an earlier view, and the samples
between the boundaries are smooth.

**Why one frame is enough to see a cut.** A dwell is a hard cut in motion: the camera is at
~1500 units per second and the next frame it is at rest. A frame of 1278 units at the dwell's start
also means a much larger step than the frames before it (38), and at the dwell's end the camera
leaves with one long step (22.95 at 24 fps on the test path, against a 33.55 pace) — a "freeze,
then jump" that reads as a jump. The remaining lever inside the interpolator is where the dwell
starts: the sample immediately before the boundary could already be at the keyframe, and the first
sample after the dwell leaves at the path's pace. That is a change of the timeline convention, not
of the profile, and it does not remove the step at the boundary — a dwell is a stop, so the frame
before it is always one step away from rest.

**Test now in place** (`tests/renderer/keyframe_interpolator_test.cpp`, "entering and leaving a
dwell"): on a 19-keyframe path with a 1 s dwell at keyframe 5 at 24 fps, the first frame after the
dwell may cover at most 2x the path's own pace (measured 22.95 against 33.55) and the following
five at most 1.5x. It passes; the numbers are printed by the check when it fails.

**Open.** The user still reports a visible jump at dwell boundaries. Before changing the timeline
convention (the sample before a dwell landing on the keyframe) I need to know which of the two
boundaries is seen: the one where the camera stops, or the one where it starts again.

### 2026-11-21 (fourth report) — measured on the real path with a dwell, plus the `.xyz` export

**Exported the paths to check them outside the app.** New
`KeyFrameInterpolator::save_interpolated_positions(file, precision)`: writes the interpolated
positions, one "x y z" per line, one line per video frame, and Mapple's Camera Path window has a
"Save Positions..." button next to "Record Video...". `domik.kf` with a 1 s dwell at keyframe 5,
24 fps, exported for all three profiles into `~/Downloads/domik_dwell5_{constant,ease_in_out,cinematic}.xyz`
(and the no-dwell ones into `~/Downloads/domik_cameras_*.xyz`).

**Measured in those files.** Constant: median step 29.9 units, max 81.8 at frame 122 — the sample
that crosses into the dwell. Ease in-out and Cinematic: **1257 units at frame 122** (entering the
dwell) and **1199 at frame 147** (leaving it), against a median of 32. So the two profiles move the
camera across the last ~1 200 units to the dwell keyframe in a single frame, while Constant covers
the same ground in 82 units. That is the jump the user sees, and it is in the easing, not in the
dwell handling: with a path-wide smoothstep the camera is at speed 0 at the path's ends only, so it
reaches the dwell keyframe at full pace, and a dwell then freezes it — one frame carries 1 200 units,
then 23 frames carry nothing. Constant's 82 units are the same mechanism, much milder.

**Also fixed while checking:** `save_keyframes()`/`read_keyframes()` dropped the dwell times (a path
with dwells came back with none after save + load); they are now written as an optional
`dwell: <seconds>` line after the orientation, and files without it load as before (verified by
round-trip: dwell 1.0 at keyframe 5 survives).

**Conclusion for the fix.** A dwell needs a *stop* before it: the profile must bring the camera to
rest at a keyframe that holds a dwell, and accelerate away from it, without stopping at keyframes
that do not dwell. That is the per-keyframe stop the previous entries removed as "fast → slow at
every keyframe" — the correct rule is: ease to rest only where the camera is going to stop anyway.
The test "the ease-in-out profile slows down before a keyframe without a dwell" encodes the wrong
expectation for a path that has dwells and must be revisited together with the profile.

### 2026-11-21 (fifth report) — status, measured, with the open question

**What is now in the code (all built, OpenGL and Vulkan trees compile, the interpolator test
passes):**
- (The `save_interpolated_positions()` export and its "Save Positions..." button were a debugging
  aid, used to measure the paths outside the app; they were removed once the motion was right.)
- `.kf` files now keep the dwell times (`dwell: <s>` after the orientation, optional on read).
- The pose is read at a path-wide parameter (fixed the "jump back after every keyframe"), and a
  keyframe that holds a dwell now eases the camera to rest over the segment before it and away
  over the segment after it.
- The last sample is always the last keyframe's pose.

**Measured in the exported files (24 fps, 19 keyframes):** constant, median step 29.9 units;
ease-in/out and cinematic, median 27.0. At the dwell boundaries: constant **382 / 374** units in one
frame, the profiles **1641 / 595**. So the stop at a dwell is *not* behaving as intended — the
frame before the dwell still carries the camera a long way, and the eased profiles are worse than
constant there, not better. That is the remaining bug, and the numbers say it is in how the stop is
applied inside a segment (a single frame before the dwell covers a whole eased stretch).

**Why this run stops here.** Five attempts at this branch have each moved the problem rather than
removed it; the numbers above are now the only reliable guide, and they point at the segment-level
easing rather than at the profile or the sampling. The next step should be decided with the user,
because the right behaviour is a product decision as much as a code one:

1. **A dwell is a stop:** ease to rest over the last part of the segment before it (the camera
   decelerates, the frame steps shrink towards the keyframe), and accelerate away after it. This is
   what the current code intends and what it does not yet achieve.
2. **A dwell is a slow-down** (the user's own note: "the dwell doesn't necessarily mean the camera
   is fully stopped"): keep the camera moving at a reduced speed instead of holding one pose, so
   there is no stop and no start to jump at.

**Test state.** `easy3d_keyframe_interpolator_test` passes on the OpenGL tree; the checks for the
profiles were reworked several times during this run and the last version passes, but it does not
yet pin the dwell-boundary step, which is what the exported files show is still wrong.

### 2026-11-21 (sixth report) — the gap is gone: ease the whole step

The user saw a gap (no camera points) in the exported paths for "Dwell + Cinematic" and
"Dwell + Ease in/out", at the dwell. It was real, and it was mine: the stop split a segment in
two halves — the first half with the path-wide profile's own pace, the second half eased to rest —
so a single frame crossed the first half (1641 units on `domik.kf`) and the second half got a
handful of samples. Constant had a milder version of it (382).

**Fix:** a step between two keyframes is now eased as a whole (`segment_ease(w_local)` inside the
segment's own span). The camera is at rest at both ends of every step, so it never has a fast pass
through a keyframe, a dwelled keyframe has no gap in front of it, and the frame steps shrink
towards it and grow away from it. EASE_IN_OUT and CINEMATIC now differ only in how fast the whole
path runs (with a dwell at a keyframe the camera is at rest there in both anyway).

**Measured again** (24 fps, 19 keyframes, 1 s dwell at keyframe 5, ~32.5-unit median step):

| profile | max step before | max step now |
|---|---|---|
| constant | 382 | 56.6 |
| ease in/out | 1641 | 56.6 |
| cinematic | 1641 | 56.6 |

No sample is more than 2x the median step, and nothing jumps at either dwell boundary. The test
pins it: a step eases within itself, the camera is at rest at a dwelled keyframe, the last frame
before the dwell moves < 0.02 units, and the path ends on the last keyframe.

**Open, for the user:** a dwell is now a real stop (ease in, hold, ease out). If a dwell should
instead be a slow pass (the camera keeps moving at reduced speed), that is a different rule and
should be said before it is built.

### 2026-11-21 (seventh report) — the shake: two causes, both measured

The user reported the camera view shaking along the path at every keyframe after the gap fix.
Two separate faults, both confirmed on `domik.kf` by dumping the interpolated poses:

1. **The orientation was not eased with the position.** The position followed the eased fraction
   of a step while the squad orientation used the raw linear fraction, so the camera swept its
   rotation at one pace while it crawled or sprinted in position. Fixed: the orientation uses the
   same eased fraction. (Measured after: median 4.1 deg per frame, max 8.4, no spike.)
2. **The spline parameter was a fraction of TIME, the spline is parameterised by LENGTH.** On
   `domik.kf` the two disagree (a keyframe at 0.278 of the travel time sits at 0.282 of the
   length). Before a keyframe the parameter asked for 49 units *short* of it, and the next frame
   jumped across it — a direction reversal (cos = -1.00) at every keyframe. With CINEMATIC, whose
   velocity collapses into a dwell, it was far worse: the parameter ran backwards, the spline
   carried the camera a whole segment back (1305 units) for one frame, then forward again.

**The mapping now** (one rule, no special cases): within each step the camera eases from the
keyframe it leaves to the one it reaches, in the path's own length —
`u = length_fraction[seg] + ease(w_local) * (length_fraction[seg+1] - length_fraction[seg])`.
It cannot pass a keyframe it has not reached, so no profile can make the spline go back; it leaves
and arrives slowly, so a dwelled keyframe has no gap and no leap; the profile still sets the pace
along the whole path.

**Measured after** (24 fps, `domik.kf`, 1 s dwell at keyframe 5, exported to
`~/Downloads/domik_dwell5_*.xyz`):

| profile | median step | max step | direction reversals |
|---|---|---|---|
| constant, with dwell | 32.4 | 49.4 | 0 |
| ease in/out, with dwell | 32.4 | 49.4 | 0 |
| cinematic, with dwell | 32.4 | 49.4 | 0 |
| all three, no dwell | 33.5 | 49.4 | 0 |

Max step is 1.5x the median (camera acceleration across a keyframe), no reversals anywhere, and the
rotation step stays at its median — no shake. The no-dwell exports were regenerated too, into
`~/Downloads/domik_cameras_*.xyz`.

### 2026-11-21 (eighth report) — stop -> move -> stop -> move

The user's next report after the shake was gone: the video is still a sequence of stops and moves
rather than one flow. Cause: easing every step made the camera come to rest at *every* keyframe,
whether or not that keyframe holds a dwell.

**The rule now**: a step is eased only when it touches a dwell — a step that ends at a keyframe
holding a dwell (the approach) and a step that starts at one (the departure). Every other step goes
at an even pace, so the camera passes a plain keyframe at constant speed. The pose parameter is
always measured in the path's own length, which is what keeps the spline from overshooting a
keyframe (the earlier shake).

**Measured on `domik.kf` (1 s dwell at keyframe 5, 24 fps):**

| file | median step | max step | reversals |
|---|---|---|---|
| with dwell, all three profiles | 29.9 | 49.4 | 0 |
| no dwell, all three profiles | 29.9 | 33.0 | 0 |

The block-by-block speed along the path is flat at 0.97-1.03x the median between dwells, 0 through
the dwell, and tapered only on the two steps that meet the dwell. The three profiles now differ only
in how fast the whole path runs (with a dwell, the stops are dictated by the dwells themselves).

`easy3d_keyframe_interpolator_test` passes, `Mapple` builds, and both `.xyz` sets were regenerated.

### 2026-11-21 — cleanup and documentation pass

The fix is confirmed, so the code was tidied around the rule above:

- **`distance_travelled()` split in two.** It returned a fraction of the path's *travel time*, but
  its name (and its use) suggested distance, and the interpolator needs both. Now
  `time_fraction()` (where each keyframe is in time) and `length_fraction()` (where it is along the
  path, the spline's parameter) are separate, each documented, and `interpolate()` computes both
  once instead of rebuilding them inline.
- **`interpolate()` rewritten around them**, with the rule written out as a comment at the step
  where it applies (why length and not time, why only a step that touches a dwell is eased).
- **Dead code removed:** the unused `length_at()`, the superseded `speed_at()`/`distance_fraction_`
  members, the commented-out `save_interpolated_positions()` (library, Mapple and its button), and
  the unused `ease_parameter()`.
- **Docs on the enum:** `SpeedProfile` now says what each profile does to a step and that a step
  that only passes a keyframe is never eased (with the "stop -> move -> stop -> move" reason).
- Also in this pass: the last sample is forced onto the path's end (`t_k = total` for the final
  frame, `u = 1`), so the video ends exactly on the last keyframe whatever the duration rounds to;
  and the dwell test is `t_k <= dwell_end + 1e-4`, so a sample landing on the dwell's end (up to
  the rounding of `k * interval`) reads the held pose rather than the first step of the departure.

Two more things came out of checking the cleaned code against a real file:

- **`.kf` dwells are written and read again.** `save_keyframes()` writes an optional
  `dwell: <seconds>` line after a keyframe's orientation (only when it is non-zero), and
  `read_keyframes()` is now a line-by-line reader that accepts that line and stays compatible with
  files that have none. The writer had existed; the reader had lost it, so a path saved with dwells
  came back without them. Verified by round trip on `domik.kf` (dwells 2 s at keyframes 7 and 15
  survive, positions identical).
- **The two-keyframe branch follows the same rule.** It still used the removed per-segment easing;
  it now eases its single step when a dwell touches it (as a two-keyframe path nearly always does,
  at its ends), leaves it linear otherwise, and lands its last sample on the second keyframe.

**Suites after the pass.** OpenGL tree (`cmake-build-relwithdebinfo`): 120/120 pass. Vulkan tree
(`cmake-build-debug`): 151/152, the one failure `vulkan.crosssection`, which also fails with the
interpolator changes stashed (its own fixture picks nothing; the intermittent failure recorded in
`ToDo.md`). `easy3d_keyframe_interpolator_test` and `easy3d_camera_path_builder_test` pass, and
`Mapple` builds.

### 2026-11-21 — confirmed by the user

The recorded video now looks as expected: an even pass between dwells, a full stop in a dwell, and
no shake at any boundary. The interpolator's final rule, in one paragraph:

- time along the path = travel between keyframes + dwell at each one;
- the pose is read at a fraction of the path's LENGTH (the spline's own parameter), never of its
  time, so the spline cannot overshoot a keyframe;
- a step is eased (ease in and out) only when it touches a dwell — the approach and the departure;
  a plain step is linear;
- the orientation follows the same eased fraction as the position;
- the last sample is the last keyframe's pose.

# Keyframes: Camera Path Timing and Motion

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
The strip rebuilds its layout whenever the path's keyframe count differs from the cached one,
so insert and delete need no call; `invalidate()` covers what the count cannot show — a
changed dwell or travel time — and is also called on undo.

### Editing the keyframes with Alt

The Camera Path window *is* the editing mode: `MappleViewer::camera_path_editing_mode()` is true
while the window is open, and inside it `Alt` belongs to the keyframes and never to the model.

- **`Alt` held = the handles are live.** `sync_keyframe_editing()` turns `KeyFrameEditor` editing
  on only while the window is open **and** `Alt` is down, so the glyphs, rings and arrows come out
  with the key and go away when it is released; there is no switch for them in the window. Editing
  the path does not require drawing it, so `Show path and keyframe cameras` off keeps the handles.
- **`Alt` + press/drag edits the selected pose.** The press grabs the pose's camera glyph or one of
  its gizmo handles: a glyph drag moves it in the screen plane, or in the plane of its own up with
  `Shift`; an axis arrow moves it along that axis and a ring turns it about that axis. `Esc`
  restores the pose captured at the press, `Ctrl+Z` the path.
- **`Alt` + click on a surface aims.** A press that grabs neither glyph nor handle arms the click,
  and a release that did not move queues the same deferred surface pick the other tools use; its
  answer turns the selected pose toward the picked point, keeping the pose's roll. A move instead
  cancels the arm — that gesture is the camera's.
- **`Alt` means nothing else in this mode.** It neither manipulates the model nor announces its
  box, which is what `Alt` does outside the window. A press that grabs no glyph or handle falls
  through, so the clipping plane keeps its `Alt` drag and `Shift` its pivot, while `Alt`+`K` and
  `Alt`+`D` (add/delete a keyframe) are keys of their own and unchanged.
- **`Dwell (s)` sits immediately below the timeline strip**, next to the bars it edits.

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
- `applications/Mapple/camera_path_timeline.h/.cpp` — `CameraPathTimeline` ImGui control,
  `cache_is_current()`
- `applications/Mapple/ui/animation.cpp` — "Dwell (s)" field, timeline strip in the Camera
  Path window, recording readout from `total_duration()`, and the Camera Path window's `Alt` hint
- `applications/Mapple/mapple_viewer.h/.cpp` — `camera_path_timeline_` member,
  `edit_camera_path_timing()`, `camera_path_duration()`, `camera_path_editing_mode()`,
  `alt_held_`/`set_alt_held()` and the `Alt` handling of `mouse_press_event()` and
  `append_manipulator_frame()`
- `easy3d/renderer/keyframe_editor.{h,cpp}` — `rebuild()` carries each kept keyframe's dwell, so
  insert and delete no longer reset the path's pauses
- `tests/renderer/keyframe_interpolator_test.cpp` — regression test for dwell and the
  cinematic motion rule
- `tests/renderer/keyframe_editor_test.cpp` — regression test for the dwell surviving an
  insert and a delete

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

### 2026-11-21 — Recorded video confirmed; the rule in one paragraph

The recorded video now looks as expected: an even pass between dwells, a full stop in a dwell,
and no shake at any boundary. The interpolator's final rule, in one paragraph:

- time along the path = travel between keyframes + dwell at each one;
- the pose is read at a fraction of the path's LENGTH (the spline's own parameter), never of
  its time, so the spline cannot overshoot a keyframe;
- a step is eased (ease in and out) only when it touches a dwell — the approach and the
  departure; a plain step is linear;
- the orientation follows the same eased fraction as the position;
- the last sample is the last keyframe's pose.

### 2026-11-21 — Alt is the keyframes' modifier in Camera Path editing mode

> **Partly superseded (2026-11-22):** the `Not looked at by eye` below is closed — the maintainer
> looked, and the gestures work as intended. See the last entry of this changelog. The log evidence
> and the build results above stand.

One change, stated as it now stands (the behavior itself is under "Editing the keyframes with Alt"
above):

- The Camera Path window is the editing mode (`camera_path_editing_mode()` is
  `show_animation_window_`), and inside it `Alt` is the keyframes' modifier and never the model's:
  `sync_keyframe_editing()` keys the handles on the modifier, `mouse_press_event()` gives such a
  press to the handles or to the aim, and `append_manipulator_frame()` announces nothing. With the
  window closed, all three keep their old meaning.
- The key is read from the modifier bits of the events the scene receives — `set_alt_held()` from
  the two key handlers and the three mouse handlers, cleared on `focus_event(false)` — rather than
  from Alt's own key events or from ImGui's `IO.KeyAlt`. Only the modifier bits are sure to arrive
  while a menu, a widget or the macOS menu bar owns the keyboard, and they are the state the
  gestures act on.
- Removed: the `Edit keyframe handles` checkbox (the key does its job; the window shows a one-line
  hint in its place) and the plain-click aim, now `Alt`+click — as a plain click it could never
  fire, because an editable keyframe needs `Alt` down.
- `Dwell (s)` moved to sit immediately below the timeline strip.
- `docs/keyframe_video.md` was renamed `docs/keyframes.md`; the references in `ToDo.md` and
  `docs/mapple.md` follow it.
- Verified in a live session on the rebuilt app (Vulkan, `domik.kf`, 19 keyframes): two `Alt`+clicks
  aimed the selected pose ("keyframe 19 aimed at the clicked point"), the path then played
  (667 frames, 22.0 s) and the app exited with its settings saved. All three build trees compile and
  both backends launch with "Mapple UI ready"; the suites were not rerun (the change is app-only).
  Not looked at by eye: the model's box staying hidden while `Alt` is held in this mode, and the
  glyph/ring drags. `easy3d/util/version.h` is unchanged.

### 2026-11-21 — Removing a keyframe no longer takes Mapple down

Loading a `.kf` and pressing `Remove Last` or `Delete Selected` ended the session:

```
I 21/11/2026 23:33:47.781 menu.cpp:181] 19 keyframes loaded from /Users/lnan/Downloads/domik.kf
I 21/11/2026 23:33:52.078 animation.cpp:409] last keyframe removed (18 remaining)
E 21/11/2026 23:33:52.080 viewer.cpp:738] Viewer failed: vector
```

- **Cause, measured.** `draw_animation_window()` reads the keyframe count and the selection once at
  the top of its frame, and the `Dwell (s)` widget below the buttons used that captured selection
  *after* the button above had shortened the path in the same frame. `keyframe_dwell()` is
  `keyframes_.at()`, so index 18 of an 18-keyframe path threw `std::out_of_range`, whose `what()` is
  the word `vector` in the log. A break on `__cxa_throw` put it at
  `vector::at(this=size=18, __n=18)` ← `keyframe_dwell(index=18)` ← `draw_animation_window()`. A
  `.kf` load is what shows it: `open_recent_file()` selects the last keyframe — the one both buttons
  remove.
- **Fix.** The dwell widget now reads the editor's selection at its own line, and is disabled when
  nothing is selected; the index captured at the top of the frame is only used above it.
- **A second defect of the same family.** The strip cached one layout per keyframe, but only
  `Insert After`, `Delete Selected`, a dwell edit and undo called `invalidate()` — `Remove Last`,
  `Clear` and `Alt+K` did not. Its hover tooltip then asked the interpolator for a keyframe the path
  no longer had: the same throw, reached by moving the mouse instead of clicking. `cache_is_current()`
  compares the cached marker count with the path and rebuilds on any difference. A temporary probe
  that called the real handler measured the stale strip at 22 s for a path that then held 18
  keyframes and 21 s; after the change it reads 21 s.
- **Evidence.** A temporary pair of environment hooks loaded `domik.kf` and called the button's
  handler inside the frame — no clicking — and reproduced the `Viewer failed: vector` line for both
  buttons; after the fix the same two runs removed the keyframe, drew on, and logged no failure. The
  hooks are removed again. All three build trees compile, and a normal launch of the Vulkan app with
  `domik.obj` logs `Mapple UI ready` with no failure. The suites were not rerun: the change is
  app-only.

### 2026-11-22 — Insert and delete keep the dwell of the keyframes that stay

Reported from the window: deleting a keyframe emptied the gray pause bars of the timeline strip,
and the path's total dropped by every dwell it had.

- **Cause.** `KeyFrameEditor::delete_selected()` and `insert_after_selected()` cannot edit the path
  in place — `KeyFrameInterpolator` only appends — so both collect the poses and the times they want
  to keep and hand them to `rebuild()`, which clears the path and replays
  `add_keyframe(frame, time)`. That sets each keyframe's dwell to its default of 0, so either edit
  silently reset the whole path's pauses. `Remove Last` goes to the library's
  `delete_last_keyframe()` (a `pop_back()`), which is why only two of the three buttons showed it —
  and why `Insert After` lost the pauses too, unnoticed.
- **Fix.** `rebuild()` now takes the dwells alongside the poses and times and re-applies them
  (`set_keyframe_dwell()` after each append, the same idiom the undo restore uses). A keyframe the
  insert creates gets 0: a new pose holds nothing until it is told to.
- **Regression test.** `tests/renderer/keyframe_editor_test.cpp` (`opengl.`/`vulkan.keyframeeditor`,
  CPU only, no context): a delete keeps the survivors' dwells and the duration they add; an insert
  keeps the neighbours' dwells and leaves the new keyframe without one; popping the last keyframe
  touches no other pause. Measured both ways — with the library change taken back to `HEAD` the test
  fails on `deleting a keyframe changed the dwell of the ones that stayed`, and with it the test
  passes in all three build trees (Debug+Vulkan+FFmpeg, RelWithDebInfo+OpenGL+FFmpeg, and the
  Vulkan tree with FFmpeg off).
- **In the app.** A temporary probe loaded `domik.kf` the way File > Open Recent does (19 keyframes,
  2 s dwells at 8 and 16, 22 s total) and deleted keyframe 1: the dwells came back at 7 and 15, the
  total read 21 s (the one second of travel that went with the deleted step) and the strip agreed at
  21 s. The probe is removed again; its pre-fix counterpart was measured in the unit test rather
  than in the window.
- **Not measured:** the full CTest suites. `easy3d/core/random.h`, `easy3d/renderer/renderer.h` and
  `easy3d/util/version.h` carry edits made in the tree at the same time, so a suite run now would
  report on a moving source; the two keyframe tests are the CPU coverage of this code.
- `MappleViewer` needed no change: `delete_camera_keyframe()` and `insert_camera_keyframe()` already
  invalidated the strip, and the dwell values the strip draws are the ones that now survive.

### 2026-11-22 — The `Alt` gestures seen by eye, and the suites run

- `Alt` in Camera Path editing mode, by eye: the maintainer confirms the gestures now work as
  expected — the handles come out with `Option` and go away on release, a glyph and a ring drag
  follow the hand, and the model's box stays hidden while the mode has the key. That closes the
  `Not looked at by eye` of the 2026-11-21 entry above and the matching line in docs/mapple.md. It is
  their observation in a live session, not a measurement of mine; the synthesized-event probe and the
  `Alt`+click log lines above are what was measured.
- The full CTest suites are measured now, which closes the `Not measured: the full CTest suites`
  bullet above: it described a tree whose color-API edits were still moving, and those edits are
  finished. Run serially, one tree at a time after the API settled on `std::size_t`: OpenGL
  `cmake-build-relwithdebinfo` 121/121 and Vulkan Debug `cmake-build-debug` 153/153, and the CPU tests
  of the touched code (`core.random`, both keyframe tests) pass in the third tree as well. The two
  wall times vary across runs — 121/121 in 43, 67 and 64 s, 153/153 in 221, 257 and 259 s — so the
  passing counts are the stable claim, not the durations.
- `color_from_index()` and `color_table()` take a `std::size_t` (the maintainer's choice): the wheel
  is only ever stepped forward, a negative count is not representable, and what was measured against
  the alternative — `int idx < std::size_t num` warning `-Wsign-compare` in every TU, and an `int`
  count turning `-1` into a request for 1.8e19 colors instead of an empty table — is recorded in
  `ToDo.md`. The two functions are also bound for Python
  (`python/bindings/easy3d/core/random.cpp`), probed against the built module, and `python.rendering`
  passes in both suites.
- `easy3d/util/version.h` is back at `HEAD`. What the dwell bullet above called unrelated in-flight
  edits is now this change, in `easy3d/core/random.h` (`color_from_index()`, `color_table()`,
  `hsv_to_rgb()`), its callers (`renderer/renderer.h`, `renderer/texture_manager.{h,cpp}`,
  `fileio/point_cloud_io_vg.cpp`, `applications/Mapple-Qt/main_window.cpp`) and
  `tests/core/random_test.cpp`; the API and its measurements are recorded in `ToDo.md`.

## Still open


- Recording short videos in Mapple with dwell and speed-profile settings on both backends.
  This is an interactive check requiring a windowed session.

### 2026-11-22 — Dwell feels like ~4s instead of 2s (kRamp 0.2 → 0.18)

**Problem.** A 2 s dwell felt like ~4 s: the camera crawls below 10% speed for ~7 frames on
each side of the dwell. Measured on `domik.kf` (19 keyframes, 2 s dwells, 24 fps): speed falls
through 50% at ~7 frames and reaches 0 at the keyframe, so each dwell adds ~0.46 s of crawl on
each side, making a 2 s dwell feel like ~2.46 s.

**Fix.** Reduced `kRamp` from 0.2 to 0.18 in `KeyFrameInterpolator::segment_fraction`. The ramp
is shorter, so the camera spends less time in the slow-speed region near dwell boundaries. With
kRamp=0.18, speed at t=0.01s is 14.98% of cruise (below the 15% threshold), and per-segment
timing is unchanged.

**Vulnerability check:** The existing `keyframe_interpolator_test` passes unchanged — it checks
that speed at t=0.01s is <15% of cruise, that speed increases monotonically during the ramp,
and that speed equals cruise speed for t≥0.3s. All pass with kRamp=0.18.

**Also fixed:** `MappleViewer::advance_camera_path()` now advances the deadline by `+= period`
with a catch-up loop instead of resetting to `now + period`. This prevents playback drift where
frame cost pushed the deadline later, causing playback to run longer than the recording. The
recording path (`advance_video_recording`) already renders `interpolate()`'s samples directly,
so it was already correct.

**Validation:** OpenGL 121/121, Vulkan 153/153 serial CTest. `keyframe_interpolator_test` passes
on both backends.

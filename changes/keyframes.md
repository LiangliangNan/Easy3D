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

- `KeyframeInterpolator` in `easy3d/renderer/keyframe_interpolator.{h,cpp}` owns keyframes
  (position + orientation + time) and interpolates them into a dense path of `Frame`s.
- `KeyframeInterpolator::add_keyframe(frame, time)` sets each keyframe's time; consecutive
  keyframes default to 1 second apart.
- `KeyframeInterpolator::interpolate()` samples `num_frames` frames uniformly in the spline
  parameter `u = i / (num_frames - 1)`. Positions use cubic spline interpolation; orientations
  now use `quat::squad()` (batch 142) for smooth rotation on the unit sphere.
- `KeyframeInterpolator::set_interpolation_speed()` only scales playback globally; it cannot
  vary speed along the path.
- Mapple's Camera Path window exposes playback speed, frame rate, interpolation method, and
  video recording (bitrate, samples, FFmpeg). There is no per-keyframe timing control.
- `CameraPath` (`applications/Mapple/camera_path.{h,cpp}`) generates orbit,
  ground-walk and flight keyframes with user-friendly parameters (eye height, spacing,
  pitch, ...).

### Root cause

`interpolate()` treats every segment between consecutive keyframes as equal weight and
samples the spline parameter uniformly. There is no per-keyframe timing data (dwell) and no
remapping of the interpolation parameter through an easing curve.

## What was implemented

### Per-keyframe dwell time

`KeyframeInterpolator::Keyframe` gains a `dwell_time_` member (seconds, default 0) with
`dwell_time()` / `set_dwell_time()` accessors. `KeyframeInterpolator` gains
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

### Timeline strip (`KeyframeTimeline`)

A Mapple-owned ImGui control (`applications/Mapple/keyframe_timeline.{h,cpp}`, deliberately
private to the application, not part of the renderer library API). It draws travel segments
as colored bars proportional to their duration and dwell pauses as gray bars. Keyframe
markers highlight the selected keyframe; clicking a bar or marker selects its keyframe.
The strip rebuilds its layout whenever the path's keyframe count differs from the cached one,
so insert and delete need no call; `invalidate()` covers what the count cannot show — a
changed dwell or travel time — and is also called on undo.

### Editing the keyframes with Alt

The Camera Path window *is* the editing mode: `MappleViewer::camera_path_editing_mode()` is true
while the window is open, and inside it `Alt` belongs to the keyframes and never to the model.

- **`Alt` held = the handles are live.** `sync_keyframe_editing()` turns `KeyframeEditor` editing
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

- `easy3d/renderer/keyframe_interpolator.h` — `dwell_time_`, `set_keyframe_dwell()` /
  `keyframe_dwell()`, `segment_fraction()`, `length_fraction()`
- `easy3d/renderer/keyframe_interpolator.cpp` — `interpolate()` rewritten with the
  cinematic motion rule; `segment_fraction()` helper; `total_duration()`; dwell read/write
  in `save_keyframes()` / `read_keyframes()`
- `applications/Mapple/keyframe_timeline.h/.cpp` — `KeyframeTimeline` ImGui control,
  `cache_is_current()`
- `applications/Mapple/ui/animation.cpp` — "Dwell (s)" field, timeline strip in the Camera
  Path window, recording readout from `total_duration()`, and the Camera Path window's `Alt` hint
- `applications/Mapple/mapple_viewer.h/.cpp` — `keyframe_timeline_` member,
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

- `KeyframeInterpolator::Keyframe` gains a `dwell_time_` member (seconds, default 0) with
  `dwell_time()` / `set_dwell_time()` accessors.
- `KeyframeInterpolator` gains `set_keyframe_dwell(index, seconds)` and `keyframe_dwell(index)`
  public methods.
- `interpolate()` treats each segment's effective duration as `travel_time + dwell_time` and
  distributes samples proportionally across dwell + travel periods; during dwell periods the
  camera holds the keyframe pose.
- Mapple's Camera Path window shows a "Dwell (s)" drag field (0.0–10.0 s) when a keyframe is
  selected.
- Backward compatible: dwell defaults to 0, so existing `.kf` files and code are unchanged.

### 2026-11-20 — Timeline strip (Step 3)

- `KeyframeTimeline` lives in `applications/Mapple` (deliberately not part of the renderer
  library API). It presents travel time as proportional colored bars and dwell time as gray
  pause bars. Keyframe markers highlight the currently selected keyframe, including the final
  marker, and clicking a bar or marker selects its associated keyframe.
- The strip uses an ImGui invisible button for reliable hover/click/tooltip behavior. A
  one-keyframe path remains visible and selectable, with a zero-duration label.
- The strip is a persistent `MappleViewer` member (`keyframe_timeline_`); its segment cache
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
  - Setting every dwell back to 0 gives the un-eased samples again.
- OpenGL tree (`cmake-build-relwithdebinfo`): fully rebuilt, **120/120 tests passed**.
- Vulkan tree (`cmake-build-debug`, MoltenVK): fully rebuilt, **152/152 tests passed**.
- `easy3d/util/version.h` unchanged.

### 2026-11-21 — Cleanup and documentation pass

- `interpolate()` rewritten around `length_fraction()`, computed once
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

- **Cause.** `KeyframeEditor::delete_selected()` and `insert_after_selected()` cannot edit the path
  in place — `KeyframeInterpolator` only appends — so both collect the poses and the times they want
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

### 2026-11-22 — Dwell feels like ~4s instead of 2s (kRamp 0.2 → 0.18)

**Problem.** A 2 s dwell felt like ~4 s: the camera crawls below 10% speed on each side of the
dwell. Measured on `domik.kf` (19 keyframes, 2 s dwells, 24 fps): speed falls through 50% at ~7
frames and reaches 0 at the keyframe, so each dwell adds ~0.46 s of crawl on each side, making a
2 s dwell feel like ~2.9 s (2 s of dwell plus the ~0.92 s of crawl the ramp left on either side).

**Fix.** Reduced `kRamp` from 0.2 to 0.18 in `KeyframeInterpolator::segment_fraction`. The ramp
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

**Follow-up fix (2026-11-22, same day):** the deadline was still initialized once, at session
construction (`next_camera_path_frame_ = steady_clock::now()`), and `toggle_camera_path_playback()`
never reset it. Pressing Play therefore caught up from that stale reference: the first `pre_draw`
after the press fast-forwarded through every sample whose deadline had passed, so playback
started mid-path (the first keyframes were skipped) and only walked the tail of the path — at
~half the path duration for a playback started mid-session, which read as "playback ≈ 2× the
recording". The fix: `toggle_camera_path_playback()` now sets `next_camera_path_frame_ =
steady_clock::now()` before starting manual interpolation, so every Play begins at the first
sample with the catch-up loop stepping forward from there.

**Also added (2026-11-22, same day):** the Camera Path window shows the playback's elapsed time
on the Play line. `MappleViewer` owns it (`camera_path_elapsed_`, one sample interval - speed x
frame rate - per advance in `advance_camera_path()`, reset when the playback reaches its end) and
the Play/Stop row renders `playing... <elapsed> s` while playing, `Ctrl+K` while stopped.

**Also added (2026-11-22, same day):** the Camera Path timeline draws a red vertical line at
the playback's current position while playing, from the new `KeyframeInterpolator::playback_elapsed()`;
the placeholder stub that showed nothing is gone.

**Validation:** OpenGL 121/121, Vulkan 153/153 serial CTest. `keyframe_interpolator_test` passes
on both backends.

### 2026-11-23 — Recorded videos confirm the keyframe video generation

- The still-open interactive check is closed: the maintainer recorded videos in Mapple with the
  dwell and speed settings on both backends, and the keyframe-driven video generation behaved as
  expected - the cinematic motion and the dwells applied as designed, and playback and the
  recorded video agreed. No code change followed from the recordings.

**Validation (final code, after the `KeyframeTimeline` rename):** OpenGL `build-opengl-check` full
CTest **147/147** passed; Vulkan `build-vulkan-check` full CTest green apart from the pre-existing
`vulkan.keyframeeditor` (Not Run). The keyframe and camera-path suites pass on both backends.

### 2026-11-24 — `kRamp` 0.18 confirmed as the timing that reads right

Closes the doubt the 2026-11-22 entry left open: that entry claimed `kRamp` 0.2 → 0.18 was the
fix, while the numbers in it (about 7 frames of sub-10% crawl on each side of a dwell) are still
what 0.18 produces, so a 2 s dwell still measures about 2.46 s on screen.

- **The maintainer confirms the timing is good now** with `kRamp = 0.18f` in
  `KeyframeInterpolator::segment_fraction()`, in playback and in the recorded videos on both
  backends at x1.0 and x2.0 speed. Their observation in a live session, not a measurement of mine;
  no code change followed, and `kRamp` stays at 0.18.
- What was measured on the same code, for the record: on `domik.kf` (19 keyframes, 2 s dwells at
  keyframes 2, 5, 7 and 15) at 30 fps a dwell holds **60 frames = 2.00 s** at speed 1.0 and
  **30 frames = 1.00 s** at speed 2.0 - exactly what the `Dwell (s)` field was set to, so
  `interpolate()` is correct at both speeds. The rest of the perceived pause is the ease in and
  out: the last five frames before a dwell step down 95 %, 76 %, 58 %, 39 % and 21 % of cruise and
  the first frame after it leaves at 3 %, so the camera is slow but moving for about 0.23 s on
  each side. That ramp is the cinematic rule working as designed (the camera leaves and arrives at
  rest), not a timing error, which is why 0.18 is the value to keep.
- Also confirmed by the same probes: a dwell scales with `Speed (cinematic)` by design
  (`interpolation_period() * interpolation_speed()`), so 0.5x plays a 2 s dwell as 4 s and 2x as
  1 s - intended behaviour, not a bug - and the recording shows the same lengths because it renders
  `interpolate()`'s samples directly.
- No files changed in this entry. `easy3d/util/version.h` unchanged.

### 2026-11-24 — Proposal: one anti-gravity up for the Walk and Flight paths

**Status: implemented (2026-11-24); see "What landed" at the end of this entry for the places
where the code differs from the text below.** Written before any code change so the up rule had
one home to argue with, as the maintainer asked, and settled in the same session: the default
rule, the flight's default aim and the way the eye height follows the terrain are decided here,
not left to the code. The UI sketches are ASCII of what the Camera Path window shows; they are the
design as agreed, not a screenshot of the finished window (the field widths and the shared
"Keyframe spacing" row are the two visible differences -- see "What landed").

#### The problem

The Walk and Flight tools take two things from the drag: a **contact point** and the **surface
normal** there. `MappleViewer::route_point_at()` (`applications/Mapple/ui/animation.cpp`, the pick
branch) reads the normal from the hit face, and `CameraPathBuilder::resample_stroke()`
(`easy3d/renderer/camera_path_builder.cpp`) interpolates and smooths it along the stroke. The pose
up then comes from that smoothed normal, so three things are wrong with it:

1. **The normal is not always there.** Only a `SurfaceMesh` hit with `face_index >= 0` supplies
   one; a point cloud, a standalone drawable, or a pick without provenance falls back to
   `ground_up_direction()`. A point-cloud walk therefore follows a *different rule* from a mesh
   walk, with no sign of it in the window.
2. **Its sign is a guess.** `SurfaceMesh::compute_face_normal()` follows the winding, which is
   arbitrary (the class documents this at `surface_mesh.h:2385`). The code flips it with
   `dot(up, camera()->view_direction()) > 0` — "the side facing the camera". On a floor that is
   right; on a wall or a cliff the camera is beside the surface, so the "up" comes out horizontal
   and the walker leans onto the wall.
3. **It varies along curved surfaces.** Two smoothing passes cannot stop the up following a
   curved roof or terrain, so the camera **banks**: the horizon rolls with the surface. No person
   walking and no aircraft does that — the camera up stays with gravity while the ground rises and
   falls.

The fallback is no better than a guess either: `ground_up_direction()` is the **current view's up
vector**, so a rolled view tilts the whole path (batch 110 chose this deliberately, to keep a
rotated model upright on screen, but it is a view fact, not a world fact).

Flight is worse still: its default aim is the reverse of the lift, i.e. **at the foot it rose
from**, so a tilted normal tilts the *aim* as well as the roll — the camera looks down the surface
normal instead of at the ground ahead.

#### The decision

**The camera's up is one anti-gravity direction for the whole path, and the surface supplies only
where the foot is.**

- The up is **stored on the builder** (one vector for the path), resolved once from a rule the user
  can name, and never derived per foot or from the view at gesture time. The default rule is
  `Auto (box shortest)`: the scene box's shortest extent decides, so a building, a terrain or a
  scan that stands on its own shortest axis needs no setting at all, and `Up from the view` stays
  available for a model whose orientation is unknown and which the user has turned upright.
- The contact point is all the gesture needs from the surface. A pose is `foot + up * eye_height`,
  rising **straight up** from the contact point, so the eye can never land behind the surface and
  the sign-flip heuristic is not needed at all. This also fixes the point-cloud case for free: no
  face, no normal, same behaviour as a mesh.
- **The eye height is terrain-relative**: the pose keeps its *vertical* height above its own
  contact point, which is the same thing as measuring the height along the path's up. That is what
  makes the rule work on a slope -- the feet climb with the surface, the camera stays upright, and
  a pose is never closer to the ground it stands on than the eye height says.
- The normal therefore keeps **no** job at all, not even the lift: "straight up from the contact
  point" is both the anti-gravity rule and the terrain-relative height in one. Should a later step
  want the camera to rise along a slope's normal instead, that is a new decision with its own
  entry, not a leftover of this one.
- **Flight's default aim becomes the ground ahead**, not straight down at each station's foot:
  station *i* aims at the foot of the station about two stations further along the track (the last
  stations aim as far ahead as the track has), and each target is lowered by that station's own
  height above its foot, so a flight over a slope looks along the ground rather than into it. The
  camera flies where it is going, which is what a flight looks like; `Aim...` still overrides it.
- Walk, Flight and the Aim re-aim (`aimed_frames()`, which today passes
  `ground_up_direction()` as the roll reference for flights only) all use that same up, so
  re-aiming can never re-introduce a bank.

**Consequence to accept, and to say in the hint:** a walk on a *vertical* surface stops meaning
"standing on the wall" and becomes "a camera travelling along the wall, upright". That is the right
behaviour for a video and the wrong one for a ceiling walk; it is a visible change of intent, so it
belongs in the window's text, not only in this entry.

#### Where the up comes from

The app cannot know which way gravity points, so the user names the rule. The set is the one batch
109 had, plus the view rule batch 110 replaced it with:

| Rule | Resolves to | For |
|---|---|---|
| `Auto (box shortest)` | the world axis of the visible scene box's **shortest extent** | buildings, terrains, `domik` (+Y); **the default** |
| `+X` / `+Y` / `+Z` | that world axis | a standing figure, or a scan whose orientation is known |
| `Up from the view` | the camera's up, normalized | a model you have rotated and are looking at upright (batch 110's rule, kept as an option) |

`Auto` keeps batch 109's reasoning (a building's or a terrain's height is its shortest extent) and
batch 110's objection is answered by making the view rule a choice rather than the only rule. The
**resolved vector is shown next to the combo**, so the user sees what the app actually chose
instead of inferring it from the resulting tilt.

Deliberately **not** in the UI: a free-form vector picker, or a "drag to set the up" gizmo. Nobody
wants to author gravity with a gizmo, and it would fight the surface drag that already owns the
left button in these modes.

#### The UI

The combo belongs to **both** Walk and Flight, because both need the same reference; the value is
one member on `CameraPathBuilder`, so changing it in either mode re-frames what both make. It sits
**first** among the mode's parameters, above the heights that are measured along it. Window width is
`kCameraPathWindowWidth` (430 logical px); the sketches are the content area, not the frame.

Walk armed:

```
Camera Path
  Import...  Export...  .kf files                                     Clear
  ------------------------------------------------------------------------
  ( ) Free   (o) Orbit...   (*) Walk...   ( ) Flight...
  Drag over the ground surface to trace the route. Esc cancels.

  Up        [ Auto (box shortest)  v ]            ( 0.00, 1.00, 0.00)
  Eye height             [=====|--------]  0.20 x scene
  Keyframe spacing       [====|---------]  0.50 x height
  ------------------------------------------------------------------------
  ...
```

Flight armed:

```
  ( ) Free   ( ) Orbit...   ( ) Walk...   (*) Flight...
  Drag over the surface to trace the camera track. Esc cancels.

  Up        [ Auto (box shortest)  v ]            ( 0.00, 1.00, 0.00)
  Flight height          [=========|----]  0.50 x scene
  Keyframe spacing       [====|---------]  0.50 x height
```

The combo's tooltip, verbatim:

> The direction the camera's up points, for the whole path. `Auto` uses the scene's shortest
> extent (upright for a building or a terrain); the axes pin it to the world; `Up from the view`
> follows the camera, for a model you have rotated upright. Poses rise straight up from the traced
> points and keep their height above the ground, so a slope no longer banks the camera.

The hint line under the radios gains one clause, so the behaviour change is visible where the
gesture is described:

- Walk: *"Drag over the ground surface to trace the route. Poses rise straight up; Esc cancels."*
- Flight: *"Drag over the surface to trace the camera track. Poses rise straight up and look at
  the ground ahead; Esc cancels."*

`Free` and `Orbit...` are unchanged: an orbit's up is its own axis (the reversed picked line), which
is a turntable, not gravity.

#### What changes, when this is implemented

1. `easy3d/renderer/camera_path_builder.{h,cpp}` — bring back
   `enum class GroundUpAxis { Auto, X, Y, Z, View }` with `ground_up_axis()` /
   `set_ground_up_axis()`; `ground_up_direction()` resolves from it (shortest box extent / axis /
   view up) and stays the single place every caller already goes through. Add a way to read the
   resolved vector for the window (it is `ground_up_direction()` itself).
2. `resample_stroke()` — stop interpolating and smoothing the per-foot ups; return the feet only.
   Its signature loses the `ups` output. `route_point_at()` stops computing a normal, so a pick
   yields the contact point and nothing else.
3. `ground_frames(feet, up, eye_height)` — the up argument becomes the single path up (one vector),
   not one per foot; the per-foot heading stays (it is the aim along the route, which is correct).
   The eye height is measured along that up, so it is the pose's vertical height above its own
   contact point: terrain-relative by construction.
4. `flight_stations()` / `flight_frames()` — stations rise along the path up; the default aim
   becomes **the ground ahead**: station *i* aims at the foot of the station `look_ahead` (default
   2) further along the track, lowered by its own height above its foot, and the last stations aim
   as far ahead as the track has. The tilted-normal aim disappears with the normal.
5. `applications/Mapple/ui/animation.cpp` — the `Up` combo and the resolved-vector readout in both
   modes, the tooltips and hints above.
6. `applications/Mapple/settings.{h,cpp}` — persist the chosen rule, so the next session does not
   silently revert to `Auto` on a scene that needed `+Y`.
7. `tests/renderer/camera_path_builder_test.cpp` — a flat floor, a 45° slope and a point cloud
   (no faces) each produce a path whose pose up is identical for every pose and equal to the chosen
   rule; a `+Y` scene with the rule at `+X` banks nothing; the flight's default aim points along
   the stroke rather than at each foot.
8. `changes/keyframes.md` (this entry) gains a changelog section when the work lands, and
   `changes/mapple.md` a batch entry; `ToDo.md` line for the walk/flight up rule is closed there.

#### The principles to follow

Make it **as logical and as faithful to reality as possible** (one up per path, contact points
only, heights vertical); keep the **UI as simple and intuitive as possible** (one combo, first
among the mode's parameters, with the resolved direction beside it and no other new control); keep
the **code as clear and clean as possible** (the rule lives in `ground_up_direction()` alone, the
builders take one up instead of a per-foot list, and `resample_stroke()` returns feet only, so no
normal plumbing survives in either the builder or the gesture).

#### What landed (2026-11-24)

The entry above is the design; this is what the code does now, including the four places where it
went past or away from the text.

1. **Up rule** -- `enum class GroundUpAxis { Auto, X, Y, Z, View }` with
   `ground_up_axis()` / `set_ground_up_axis()` on `CameraPathBuilder`;
   `ground_up_direction()` resolves it (scene box's shortest extent along the rule, view up for
   `View`, with the view up as the fallback while the box is invalid). Item 1 of "What changes"
   is done as written, and the resolved vector is read straight from `ground_up_direction()`.
2. **The normal is gone from both the builder and the gesture.** `route_point_at()`
   (`applications/Mapple/ui/animation.cpp`) returns the contact point only, and the stroke is
   `std::vector<vec3>` end to end: the `RouteSample` struct that carried a point plus an up was
   deleted rather than left unused. `resample_stroke()` takes and returns feet only.
3. **`ground_frames(feet, up, eye_height)`** takes the one path up and is called as
   `ground_frames(lifted_feet(feet, eye), ground_up_direction(), 0.0f)`: the lift is applied to
   the feet first, so the eye height is measured along the path's up and is terrain-relative by
   construction.
4. **Flight** -- `flight_stations()` lifts along the path up and `flight_frames(..., look_ahead = 2)`
   aims each station at the foot of the station `look_ahead` further on, lowered by that station's
   own height, clamping to the last foot. Item 4 is done as written.
5. **Not in the design: "Follow slopes"** (added the same day, after the proposal was settled).
   Both ground tracks share one checkbox, drawn immediately under the `Up` combo, default **on**:
   on, every pose keeps its height above its own contact point (the terrain-relative rule above);
   off, the whole track keeps the first foot's altitude plus its height, measured along the path's
   up, so a walk up a hill flies level and passes through it. One switch for both modes because
   both modes have the same two behaviours; the trade-off was stated when it was added and the
   maintainer kept it shared.
6. **Not in the design: the keyframe spacing became a scene-relative value, and the two parameters
   became independent.** The design's sketches still show `0.50 x height`, i.e. spacing measured
   against the track's own height, which made the eye height and the flight height double as
   density knobs (halving either doubled the keyframe count and the duration). It is now
   `set_keyframe_spacing_in_scene_heights()` / `keyframe_spacing_in_scene_heights()`, resolved by
   `keyframe_spacing_in_world_units()` = scene height x the value, clamped to at least 1e-4, and
   **both** `generate_ground_route()` and `flight_stations()` resample at that one world spacing.
   The window shows one shared `Keyframe spacing` row (`draw_keyframe_spacing()`, drawn in Walk and
   Flight alike) instead of one per mode.
7. **Defaults at the end of the day:** eye height `0.2` scene heights, keyframe spacing `0.5`
   scene heights, flight height `4.0` scene heights, follow slopes on, up rule `Auto`. The spacing
   default is deliberately sparse; on a scene a few units tall the useful range sits lower, and the
   drag's lower bound is `0.01`.
8. **UI** -- the `Up` combo, the resolved-vector readout and the `Follow slopes` checkbox are
   drawn by `draw_ground_up_rule()`, shared by Walk and Flight; the rows of the mode's parameters
   line up because every widget in them keeps the window's default item width, as elsewhere. The
   hints carry the clauses above ("Poses rise straight up ...", "Poses rise straight up and look
   at the ground ahead ...").
9. **Persistence** -- `ground_up_axis` and `follow_ground_slopes` are both written to and read
   from `settings.ini` (`applications/Mapple/settings.{h,cpp}`), so neither rule reverts on the
   next launch.
10. **Tests** -- `tests/renderer/camera_path_builder_test.cpp` covers the rule set (Auto, View,
    `+Y`, tall and flat boxes, a point-cloud stroke with no faces), a route that crosses the up
    (no bank, identical up for every pose) and one that climbs along it (each pose over its own
    foot, then one shared altitude with the rule off, for the track and the flight alike), the
    flight's default aim (ahead, not straight down, with `look_ahead`), the tilted-lift roll and
    degenerate inputs. `easy3d_camera_path_builder_test` and `easy3d_keyframe_interpolator_test`
    pass; both Mapple trees build.

### 2026-11-24 — Proofread pass on the anti-gravity change, and the scene axes as a cue

Reading the ten files of the change back against the design above turned up seven places where
the code, the comments or the docs disagreed. All are fixed; nothing about the behaviour
changed.

- **The library docs claimed defaults the code never had.** The constructor's `\note` and both
  orbit accessor briefs said the orbit offsets start at 0; they start at 0.2 (radius) and 0.05
  (vertical), which is what the constructor initialises and what `orbit_frames()` reads.
- **`set_follow_ground_slopes()` described a lifted first foot.** "the camera flies level, at
  `foot.front() + up x height` for every pose" is not what `lifted_feet()` does: every pose keeps
  its own position across the up and gets the *first* foot's altitude along it. The doc now says
  the track holds one altitude, the first foot's, however far the ground climbs beneath it.
- **A Windows-hostile identifier.** `tests/renderer/camera_path_builder_test.cpp` named a local
  `near`, which the legacy `windef.h` macros (`near`/`far`) expand away before the compiler sees
  a declaration. Renamed to `one_ahead_target`; an audit of every `near`/`far` token in the repo
  found no other use as an identifier, only the prose of comments and of the documentation in
  `easy3d/renderer/camera.cpp`. Four tests that call `std::min`/`std::abs` while including only
  `<iostream>` also gained the header they use (`<algorithm>`/`<cmath>`), which MSVC needs.
- **Comments that pointed at the wrong up.** The flight blocks in that test still said "the view's
  up" in three places where the roll reference is `ground_up_direction()` (and not the `0,0,-1`
  one of the old normal-based rule), and the flat-route block claimed `Auto` stood the cube on
  `+X` on the line before pinning the rule to `+Y`. The same miss reached the app: the declaration
  of `commit_flight_track()` in `mapple_viewer.h` still promised stations that "rise along the
  surface normal under each foot" aiming "straight down" at it.
- **The test never tested the slope rule.** Its "slope" stroke climbs in Z while the path's up is
  `+Y`, i.e. across the up, so following the ground and holding one altitude give the *same*
  poses -- both the route and the flight assertions held under either setting, and one of them
  (`foot.y == -5 || foot.y > -5`) could not fail. The stroke is now named for what it is
  (`diagonal_stroke`), and a new `climb_stroke` climbs along `+Y`, where the rule shows: with it
  on, the poses rise from `y = -3` to `y = 1` over their own feet; with it off, all five keep
  `y = -3` while the flight feet under them still climb.
- **The changelog entry for the UI item named a constant that does not exist** ("the value fields
  share one `kFieldWidth`"). What actually lines the rows up is the default item width; the entry
  now says that, and `draw_keyframe_spacing()`'s comment said so before this pass.

**The scene axes as a cue.** The `Up` combo asks which way a path's up points, and until now the
answer had to be imagined: Mapple turns the Viewer's corner triad off, so nothing on screen said
where +X, +Y and +Z lie. `MappleViewer::append_scene_axes_overlay()`
(`applications/Mapple/mapple_viewer.cpp`) now draws the three axes through the scene's center
whenever a camera path is on screen -- the in-scene counterpart of that corner triad, at the
very box `ground_up_direction()` resolves `Auto` from. It goes through `Viewer::set_overlays()` as
a `graphics::shape::draw_lines_3d()` command rather than through `draw_corner_axes()`, so the
OpenGL and Vulkan backends need no separate code and the axes sit where the user is placing
keyframes: each axis is a full line through the center with the far half dimmed and an arrowhead
on the near one, in the keyframe handles' colours. It is deliberately thin (2.5 logical px) and
translucent (alpha 0.7), and it carries `overlay_depth_test`: the triad lies *in* the scene, occluded
by the model like any other geometry, rather than painted over the handles the user is working with.
The resolved-vector readout's tooltip now mentions it.

`easy3d_camera_path_builder_test` passes and both Mapple trees (OpenGL RelWithDebInfo, Vulkan
Debug) build; the overlay itself has an entry of its own in `changes/mapple.md`, where the Camera
Path window's history lives. One finding is recorded in `ToDo.md` rather than changed here: a
flight's last stations have nothing ahead of them, so the clamped aim points them straight down
along the up, where the roll is undefined -- against the level-horizon rule the rest of the track
follows.

### 2026-11-24 — The orbit axis is the Up rule too

Orbit was the one generated mode that ignored the shared `Up` combo: its axis came from a drag
across the scene, or from the view's vertical when the gesture was a click. It now takes the same
direction the window already resolves for walk and flight, and the click only says where the axis
stands -- one combo, one click, no gesture to learn. The drag state and the screen-space axis
preview went with it, and `Follow slopes`, which belongs to the two ground tracks alone, is now a
separate widget that Orbit does not draw. The details, and what is still unverified by eye, are in
the 2026-11-24 orbit entry of `changes/mapple.md`.

### 2026-11-24 — Reproducible camera-path input and parameter-driven rebuilds

A generated path used to be a one-shot result: its strokes, picked anchor and sampled aim
targets were reduced to keyframes, so changing Eye height, Flight height, spacing or an orbit
parameter could only affect the next gesture. `CameraPathBuilder` now separates that raw input
from the settings that shape it:

- `CameraPathBuilder::Source` records a Walk's separate ground strokes, a Flight's ground
  strokes, or an Orbit's picked anchor and center, plus the raw aim points. It stores no feet,
  lifted stations, resampled targets or derived orbit axis.
- `CameraPathBuilder::Settings` is the single value snapshot for every builder parameter.
  `settings()` returns a copy; `settings_changed_for(source, built)` compares only the fields
  that mode reads. Up affects all three modes; eye height only Walk; flight height only Flight;
  spacing and Follow slopes only Walk/Flight; radius, vertical offset, pitch and loop counts
  only Orbit. A setting moved away and back compares equal.
- `frames_of(source)` is the pure regeneration entry point. Walk resamples each stroke
  separately, Flight joins its strokes before resampling, and Orbit re-derives its axis from
  the anchor and current Up rule. Raw aims are sampled again from the source rather than kept
  as stale per-keyframe targets. Orbit's closing duplicate is removed before aim sampling,
  because the interpolator cannot store two coincident positions.
- Mapple owns the editable-path policy in `applications/Mapple/ui/path_generation.cpp`, not
  the renderer. Its `GeneratedPath` records pair a source and build-time settings with the
  keyframe range they own. Add/insert/remove/delete operations shift those ranges instead of
  discarding them. Insertions inside a range become manual gaps anchored by generated-pose
  ordinal; deleting a generated pose decrements later anchors, while deleting a manual pose
  removes only its gap. Rebuilding splices newly generated blocks around unowned keyframes and
  preserves manual poses, their times and dwells; the application's UI, aims and one-level undo
  are described in `changes/mapple.md`.

Measured: `easy3d_camera_path_builder_test`, `easy3d_keyframe_interpolator_test` and
`easy3d_generated_path_test` pass in both the Vulkan Debug and OpenGL RelWithDebInfo trees,
and both build `Mapple`. The complete non-desktop suites also pass: OpenGL 47/47 and Vulkan
66/66. The builder test covers reproduction of all three modes, raw aim
resampling and frame-level effects of the complete per-mode influence matrix. The separate
Mapple test (`opengl.generatedpath` / `vulkan.generatedpath`) covers edit-surviving provenance
and undo-snapshot restoration of an earlier generated path without making the renderer test
depend on application code. Not measured: interactive parameter dragging and keyframe editing
in Mapple; the app was not launched.
`easy3d/util/version.h` unchanged.

### 2026-11-25 — Flight tails inherit the previous roll

The default flight aim clamps its look-ahead to the final station. With the default two-station
look-ahead, the penultimate station aims down at the final foot and the final station aims
straight down at its own foot. That last view is parallel to the path's up, so the path up cannot
provide a roll reference.

- The aim direction is unchanged: stations still look at the clamped ahead foot lowered by their
  own height. An initial attempt to level the tail was rejected because it made a downward-looking
  flight suddenly look ahead.
- When a station's view is parallel to the path up, `flight_frames()` now uses the previous
  station's camera up as the roll reference. Consecutive aligned views keep carrying the last known
  roll; the first aligned station retains the deterministic fallback. If the previous camera up is
  itself parallel to the new view, the previous camera right supplies the reference instead.
- Explicit aims keep their targets. A target coincident with its station also keeps the documented
  straight-down fallback rather than being treated as an aligned view.
- `tests/renderer/camera_path_builder_test.cpp` checks the unchanged downward tail aim, inherited
  camera up, custom one-station look-ahead, tilted lift, consecutive vertical views, explicit
  aligned aims, and coincident-target and degenerate-up fallbacks.
- Validation: OpenGL RelWithDebInfo and Vulkan Debug both build Mapple. The complete non-desktop
  suites pass: OpenGL 47/47 and Vulkan 66/66. Interactive Mapple playback was not visually checked.

### 2026-11-25 — Stream keyframe I/O and reproducible camera-path files

- `KeyframeInterpolator::save_keyframes()` now writes to `std::ostream&` instead of opening a
  filename, and `read_keyframes()` is replaced by `load_keyframes(std::istream&)`. File ownership
  stays with callers; Mapple can therefore write ordinary keyframes and append provenance in one
  file without making the renderer aware of application-level generation records.
- The ordinary stream format keeps its existing position/orientation records and optional dwell,
  and adds explicit `time:` plus `periodic: 0|1` metadata. Floating-point output uses maximum
  round-trip precision. Readers still accept the original format, assigning one-second intervals,
  zero dwells and open interpolation when those fields are absent. Loading parses into temporary
  keyframes before replacing the path, preserving coincident positions and nonmonotone times that
  can be produced by the editing setters without dropping poses or shifting their orientations.
- Interpolated SQUAD orientations are now normalized before storage, matching the existing
  two-keyframe path. Previously the composed quaternion could drift slightly away from unit
  length; that changed the rotation matrix's scale and made a round-trip quaternion-dot check
  report a false orientation change. The interpolator regression now checks unit length.
- Mapple's app-level format appends versioned JSON provenance after the ordinary keyframes. It
  restores generated-path sources/settings/build context and playback parameters only after
  regenerating and validating the owned positions. The exact ordinary poses remain authoritative for
  rendering, including manually edited orientations; provenance supplies parameter-driven
  regeneration and interpolation policy. Orbit's periodic flag survives reload, while Walk and
  Flight stay open.
- Positive-corner `GenericBox` construction now starts from an invalid box before growing to its two
  corners, rather than expanding positive minima to the origin. Early version-1 provenance may omit
  playback metadata; absent playback keeps interpolator defaults, while malformed present playback
  still falls back.
- `tests/renderer/camera_path_file_test.cpp` (`opengl.camerapathfile` /
  `vulkan.camerapathfile`) covers stream I/O, edited coincident/nonmonotone keyframes, all generation
  modes, multiple blocks, manual gaps, timing/dwells, periodic interpolation, non-default playback
  settings, orientation-only pose edits, legacy files and invalid or stale provenance fallback.
- Validation: both Mapple targets build; the complete non-desktop suites pass (Vulkan Debug
  67/67, OpenGL RelWithDebInfo 48/48). Interactive playback and recorded-video equivalence were
  not visually checked.

### 2026-11-25 — seek playback to a selected keyframe

- `KeyframeInterpolator` now owns its playback position. `playback_elapsed()` reports the
  timeline position of the applied sample, including dwell pauses, and preserves that position
  while paused. Editing the path resets it; completing playback resets it for the next session.
- `seek_to_keyframe()` stops playback, applies the exact stored pose, and reports the exact
  keyframe timeline position: the travel times and dwell pauses of all preceding keyframes.
  Manual playback resumes at the first interpolation sample at or after that position. This
  keeps an unaligned keyframe's indicator exact without resuming before it.
- Timer and manual playback now update the same position, so hosts no longer need a second
  elapsed-time clock. `start_interpolation()`, `start_manual_interpolation()`, and
  `stop_interpolation()` retain the next sample or paused position as before.
- `tests/renderer/keyframe_interpolator_test.cpp` covers dwell-aware seeking, exact poses and
  elapsed times, resume and pause continuity, seeking during playback, and a keyframe between
  interpolation samples.
- Validation: Vulkan Debug and OpenGL RelWithDebInfo rebuild; the complete non-desktop suites
  pass (Vulkan 67/67, OpenGL 48/48), and `vulkan.camerapath` passes. Interactive Mapple timeline
  selection and playback were not visually checked.

### 2026-11-25 — keyframe naming is consistent

- The interpolator is renamed `KeyframeInterpolator`, and
  `easy3d/renderer/keyframe_interpolator.{h,cpp}` replaces the former split-word filenames.
  Camera accessors, forward declarations, include guards, bindings, tests, tutorials and build
  metadata use the same spelling. `KeyframeEditor` and `KeyframeGizmo` follow the same convention.
- Newly written `.kf` files use `num_keyframes:`; the bundled `bunny.kf` and `domik.kf` are
  updated too. Loading still skips the informational count header, so files with the legacy
  header remain readable. The camera-path file regression now covers that legacy header as well
  as the original format without explicit times, dwells or periodicity.
- Validation: Vulkan Debug and OpenGL RelWithDebInfo rebuild; the complete non-desktop suites
  pass (Vulkan 67/67, OpenGL 48/48), and `vulkan.camerapath` passes. Interactive Mapple UI
  behavior was not visually checked.

### 2026-11-25 — CameraPath naming matches its files

- Mapple's authoring class is renamed `CameraPath` and now matches
  `applications/Mapple/camera_path.{h,cpp}`; `.kf` composition lives in
  `camera_path_io.{h,cpp}`. The free save/read API, provenance format and interpolation
  behavior are unchanged.
- The generation and I/O tests are `camera_path_test.cpp` and `camera_path_io_test.cpp`,
  registered as `camerapathgeneration` and `camerapathio` for both backends.

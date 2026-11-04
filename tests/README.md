# Easy3D tests

`Easy3D_BUILD_TESTS` (default `OFF`) is the only test option. `ON` builds and registers
all tests supported by the configured backend and optional dependencies. Vulkan
regression tests request Khronos and synchronization validation; there is no
separate validation switch. Python package tests are included when Python bindings
are built. No test executable or CTest registration is generated when tests are off.
Existing build trees also lose their obsolete test cache switches and stale root
CTest registration when switched off.

```sh
cmake -S . -B build-vulkan -DEasy3D_ENABLE_VULKAN=ON -DEasy3D_BUILD_TESTS=ON
cmake --build build-vulkan --parallel 4
ctest --test-dir build-vulkan --output-on-failure

cmake -S . -B build-opengl -DEasy3D_ENABLE_VULKAN=OFF -DEasy3D_BUILD_TESTS=ON
cmake --build build-opengl --parallel 4
ctest --test-dir build-opengl --output-on-failure
```

Use `-LE desktop` for display-free tests and `-L desktop` for native-window tests.
Desktop tests run serially even with `ctest --parallel`, avoiding competing windows
and focus changes. Run backend desktop suites one at a time. Artifacts are isolated under `BUILD/tests/artifacts/TEST_NAME`.
All test implementations, fixtures, harnesses and registrations are in this folder:

| Directory | Coverage |
| --- | --- |
| `core` | Model construction, properties, connectivity, deletion, I/O, linear solvers and splines |
| `algo` | Each retained point-cloud/surface-mesh algorithm case, plus synthetic HRBF interpolation |
| `kdtree` | All four kd-tree implementations against known nearest/k/radius results |
| `util` | Assertions for signals, bounded asynchronous timers, and joined logging workers |
| `renderer` | Headless Vulkan rendering, text, depth annotations/textures and GPU selection |
| `viewer` | Backend/shared viewer regressions, input, picking, captures, GUI and resource lifetime |
| `viewer/installed` | SDK-free consumers of installed renderer/viewer/gui components |
| `native_tutorials` | Bounded drivers generated from original tutorial scenes, with test-only entry-point renaming in test CMake |
| `python` | Regression against the deployed Python package |

The removed `visualization_*` copies map to the original tutorial scenes:
202, 205–207, 301–311, 401–403, 501–506 and 601–602. Offscreen behavior is covered
by the shared offscreen regression and Tutorial 208; the old timer-driven threading
demo is covered by Tutorial 312 and the joined-worker viewer regression. Dedicated
viewer/renderer tests additionally assert pixels, depth, IDs and resource sharing.
The five effect scenes (501–505) run on OpenGL; their Vulkan implementations remain
migration work. Optional CGAL algorithm cases are registered only when algo_ext is
built; RANSAC cases require the compiled Boost support. No interactive aggregate `Tests` executable or intentional logging crash
is retained. The former Python tutorial-loop runner is replaced by CTest and the
packaged Python regression.

New tests should use small deterministic fixtures, explicit checks that work in
Release builds, bounded waits/frames, nonzero failure exit codes, and no manual
inspection or private machine data. Existing algorithm fixtures use shipped
resources; every case has its own CTest entry and checks output invariants.

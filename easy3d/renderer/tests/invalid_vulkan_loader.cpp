// SPDX-License-Identifier: GPL-3.0-or-later
// Deliberately loadable, but not a Vulkan loader.
extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
int easy3d_invalid_loader_fixture() { return 1; }

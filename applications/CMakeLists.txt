if (EASY3D_ENABLE_QT)
    add_subdirectory(Mapple)
endif ()



# Liangliang: I intentionally make these hidden (NOT finished; NOT stable)
set (BUILD_TEXT3D ON)
set (BUILD_VULKAN ON)


if (BUILD_TEXT3D)
    add_subdirectory(Text3D)
endif ()

# Liangliang: I intentionally make this hidden (NOT finished; NOT stable)
if (BUILD_VULKAN)
    add_subdirectory(Vulkan)
endif ()
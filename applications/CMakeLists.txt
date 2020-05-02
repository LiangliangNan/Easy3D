if (EASY3D_ENABLE_QT)
    add_subdirectory(Mapple)
endif ()



# Liangliang: I intentionally make these hidden (NOT finished; NOT stable)
set (BUILD_TEXT_MESHER      ON)
set (BUILD_VULKAN_VIEWER    ON)


if (BUILD_TEXT_MESHER)
    add_subdirectory(TextMesher)
endif ()

# Liangliang: I intentionally make this hidden (NOT finished; NOT stable)
if (BUILD_VULKAN_VIEWER)
    add_subdirectory(VulkanViewer)
endif ()
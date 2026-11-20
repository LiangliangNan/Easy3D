if (Easy3D_ENABLE_QT AND NOT Easy3D_ENABLE_VULKAN)  # Mapple-Qt is OpenGL only
    add_subdirectory(Mapple-Qt)
endif ()

add_subdirectory(Mapple)

add_subdirectory(FigureMaker)

if(Easy3D_ENABLE_VULKAN)
    add_subdirectory(VulkanViewer)
endif()

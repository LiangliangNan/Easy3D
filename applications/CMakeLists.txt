if (Easy3D_ENABLE_QT AND NOT Easy3D_ENABLE_VULKAN) # Mapple is OpenGL only
    add_subdirectory(Mapple)
endif ()

add_subdirectory(FigureMaker)

if(Easy3D_ENABLE_VULKAN)
    add_subdirectory(VulkanViewer)
endif()

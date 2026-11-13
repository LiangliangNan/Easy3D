if (Easy3D_ENABLE_QT)
    if (Easy3D_ENABLE_VULKAN) # Mapple is OpenGL only
        message(WARNING "Vulkan is enabled but Mapple currently only supports OpenGL. To built Mapple with OpenGL, set 'Easy3D_ENABLE_VULKAN' to OFF.")
    else ()
        add_subdirectory(Mapple)
    endif ()
endif ()

add_subdirectory(FigureMaker)

if(Easy3D_ENABLE_VULKAN)
    add_subdirectory(VulkanViewer)
endif()

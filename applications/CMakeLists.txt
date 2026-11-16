if (Easy3D_ENABLE_QT)
    if (Easy3D_ENABLE_VULKAN) # Mapple-Qt is OpenGL only
        message(WARNING "Vulkan is enabled but Mapple-Qt only supports OpenGL. To build Mapple-Qt with OpenGL, set 'Easy3D_ENABLE_VULKAN' to OFF.")
    else ()
        add_subdirectory(Mapple-Qt)
    endif ()
endif ()

add_subdirectory(Mapple)

add_subdirectory(FigureMaker)

if(Easy3D_ENABLE_VULKAN)
    add_subdirectory(VulkanViewer)
endif()

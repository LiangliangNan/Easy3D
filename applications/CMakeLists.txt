if (EASY3D_ENABLE_QT)

    add_subdirectory(Mapple)

    if(EASY3D_BUILD_EASYDEPLOY)
        add_subdirectory(EasyDeploy)
    endif ()

endif ()
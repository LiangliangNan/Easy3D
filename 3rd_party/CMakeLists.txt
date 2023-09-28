function(add_3rdparty_module module sources headers)
    add_library(${module} STATIC ${sources} ${headers})
    set_target_properties(${module} PROPERTIES FOLDER "3rd_party")
    set_target_properties(${module} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    if (MSVC)
        target_compile_definitions(${module} PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
    endif()
endfunction()


add_subdirectory(glfw)
add_subdirectory(imgui)

add_subdirectory(rply)
add_subdirectory(lastools)

add_subdirectory(kdtree)

add_subdirectory(ransac)
add_subdirectory(poisson)

add_subdirectory(triangle)
add_subdirectory(tetgen)
add_subdirectory(glutess)
add_subdirectory(opcode)
add_subdirectory(polypartition)




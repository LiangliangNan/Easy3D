if(Easy3D_BUILD_PYTHON_BINDINGS)
    add_test(NAME python.rendering
        COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/rendering.py" "${Easy3D_PYTHON_PACKAGE_DIR}")
    set_tests_properties(python.rendering PROPERTIES TIMEOUT 120 LABELS desktop)
endif()

include(../properties.cmake)

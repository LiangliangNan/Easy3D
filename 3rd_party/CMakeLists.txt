cmake_minimum_required(VERSION 3.10)

project(3rd_party LANGUAGES C CXX)

#glfw
add_subdirectory(glfw)


# tinyobjloader
add_library(tinyobjloader INTERFACE)
set(TINYOBJ_DIR ${CMAKE_CURRENT_LIST_DIR}/tinyobjloader)
target_include_directories(tinyobjloader INTERFACE ${TINYOBJ_DIR})


# glm
add_library(glm INTERFACE)
set(GLM_DIR ${CMAKE_CURRENT_LIST_DIR}/glm)
target_include_directories(glm INTERFACE ${GLM_DIR})
target_compile_definitions(glm INTERFACE
        GLM_FORCE_SWIZZLE
        GLM_FORCE_RADIANS
        GLM_FORCE_CTOR_INIT
        GLM_ENABLE_EXPERIMENTAL
        )

# imgui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_FILES
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imconfig.h"
        "${IMGUI_DIR}/imgui.h"
        "${IMGUI_DIR}/imgui_internal.h"
        "${IMGUI_DIR}/imstb_rectpack.h"
        "${IMGUI_DIR}/imstb_textedit.h"
        "${IMGUI_DIR}/imstb_truetype.h")
add_library(imgui STATIC ${IMGUI_FILES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
set_property(TARGET imgui PROPERTY FOLDER "3rd_party")


# spirv_reflect
#add_subdirectory()
set(SPIRV_REFLECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect")
set(SPIRV_REFLECT_FILES "${SPIRV_REFLECT_DIR}/spirv_reflect.h" "${SPIRV_REFLECT_DIR}/spirv_reflect.cpp")
add_library(spirv_reflect STATIC ${SPIRV_REFLECT_FILES})
target_include_directories(spirv_reflect PUBLIC ${SPIRV_REFLECT_DIR})
set_property(TARGET spirv_reflect PROPERTY FOLDER "3rd_party")


#spdlog
add_subdirectory(spdlog)

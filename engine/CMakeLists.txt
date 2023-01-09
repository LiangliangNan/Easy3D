set(module engine)

set(private_dependencies glfw imgui spirv_reflect tinyobjloader)
set(public_dependencies glm spdlog Vulkan::Vulkan)

set(${module}_headers
        application.hpp
        camera.hpp
        events.hpp
        frame_rate.h
        logging.h
        lve_buffer.hpp
        lve_camera.hpp
        lve_device.hpp
        lve_frame_info.hpp
        lve_game_object.hpp
        lve_model.hpp
        lve_pipeline.hpp
        lve_renderer.hpp
        lve_swap_chain.hpp
        lve_utils.hpp
        lve_window.hpp
        vk_application.hpp
        vk_common.h
        vk_descriptors.h
        vk_initializers.h
        vk_shaders.h
        vk_ui_overlay.h
        systems/point_light_system.hpp
        systems/simple_render_system.hpp
        )

set(${module}_sources
        application.cpp
        events.cpp
        lve_buffer.cpp
        lve_camera.cpp
        lve_device.cpp
        lve_game_object.cpp
        lve_model.cpp
        lve_pipeline.cpp
        lve_renderer.cpp
        lve_swap_chain.cpp
        lve_utils.cpp
        lve_window.cpp
        vk_application.cpp
        vk_common.cpp
        vk_descriptors.cpp
        vk_initializers.cpp
        vk_shaders.cpp
        vk_ui_overlay.cpp
        systems/point_light_system.cpp
        systems/simple_render_system.cpp
        )

add_library(${module} STATIC ${${module}_headers} ${${module}_sources})

target_include_directories(${module} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${module} PRIVATE ${private_dependencies})
target_link_libraries(${module} PUBLIC ${public_dependencies})

target_compile_definitions(${module} PUBLIC "RESOURCE_DIR=\"${CMAKE_SOURCE_DIR}/\"")
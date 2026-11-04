add_executable(VulkanViewer main.cpp)
target_link_libraries(VulkanViewer PRIVATE easy3d::viewer)
set_target_properties(VulkanViewer PROPERTIES FOLDER "applications")
install(TARGETS VulkanViewer RUNTIME DESTINATION "${Easy3D_INSTALL_BIN_DIR}")

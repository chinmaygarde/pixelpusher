
add_executable(machine
  main.cc
  surface.cc
  surface.h
  vulkan_connection.cc
  vulkan_connection.h
  vulkan_swapchain.cc
  vulkan_swapchain.h
  shader_loader.h
  shader_loader.cc
)

compile_shaders(machine
  shaders/triangle.frag
  shaders/triangle.vert
)

target_link_libraries(machine
  PRIVATE
    glfw
    core
)

target_include_directories(machine
  PRIVATE
    /usr/local/include
)

get_filename_component(PIXEL_SHADERS_LOCATION ${CMAKE_BINARY_DIR}/shaders ABSOLUTE)
configure_file(shader_location.h.in shader_location.h @ONLY)
target_include_directories(machine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

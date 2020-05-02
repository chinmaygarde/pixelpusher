
add_executable(machine
  main.cc
  pipeline_builder.cc
  pipeline_builder.h
  pipeline_layout.cc
  pipeline_layout.h
  render_pass.cc
  render_pass.h
  renderer.cc
  renderer.h
  shader_loader.cc
  shader_loader.h
  surface.cc
  surface.h
  vulkan.h
  vulkan_connection.cc
  vulkan_connection.h
  vulkan_swapchain.cc
  vulkan_swapchain.h
)

compile_shaders(machine
  shaders/triangle.frag
  shaders/triangle.vert
)

target_link_libraries(machine
  PRIVATE
    glfw
    core
    Vulkan::Headers
    glm
)

get_filename_component(PIXEL_SHADERS_LOCATION ${CMAKE_BINARY_DIR}/shaders ABSOLUTE)
configure_file(shader_location.h.in shader_location.h @ONLY)
target_include_directories(machine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

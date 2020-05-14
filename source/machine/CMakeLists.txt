
add_executable(machine
  command_buffer.cc
  command_buffer.h
  command_pool.cc
  command_pool.h
  fence_waiter.cc
  fence_waiter.h
  image_decoder.cc
  image_decoder.h
  imgui_connection.cc
  imgui_connection.h
  main.cc
  memory_allocator.cc
  memory_allocator.h
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
  shaders/triangle.h
  vulkan.h
  vulkan_connection.cc
  vulkan_connection.h
  vulkan_swapchain.cc
  vulkan_swapchain.h
)

target_include_directories(machine
  PRIVATE
    .
    ../../third_party/vulkan_memory_allocator/src
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
    stb
    imgui
)

get_filename_component(PIXEL_SHADERS_LOCATION ${CMAKE_BINARY_DIR}/shaders ABSOLUTE)
get_filename_component(PIXEL_ASSETS_LOCATION assets ABSOLUTE)
configure_file(shader_location.h.in shader_location.h @ONLY)
configure_file(assets_location.h.in assets_location.h @ONLY)
target_include_directories(machine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_library(machine_lib
  asset_loader.cc
  asset_loader.h
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
  tiny_gltf.h
  tiny_gltf.cc
)

target_include_directories(machine_lib
  PUBLIC
    .
    ../../third_party/vulkan_memory_allocator/src
)

compile_shaders(machine_lib
  shaders/triangle.frag
  shaders/triangle.vert
)

target_link_libraries(machine_lib
  PUBLIC
    glfw
    core
    Vulkan::Headers
    glm
    stb
    imgui
    tiny_gltf
)

get_filename_component(PIXEL_SHADERS_LOCATION ${CMAKE_BINARY_DIR}/shaders ABSOLUTE)
get_filename_component(PIXEL_ASSETS_LOCATION assets ABSOLUTE)
get_filename_component(PIXEL_GLTF_MODELS_LOCATION ../../third_party/gltf_sample_models/gltf_sample_models/2.0 ABSOLUTE)
configure_file(shader_location.h.in shader_location.h @ONLY)
configure_file(assets_location.h.in assets_location.h @ONLY)
target_include_directories(machine_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_executable(machine
  main.cc
)

target_link_libraries(machine
  PRIVATE
    machine_lib
)

add_executable(machine_unittests
  asset_loader_unittests.cc
)

target_link_libraries(machine_unittests
  PRIVATE
    machine_lib
    gtest
    gtest_main
)

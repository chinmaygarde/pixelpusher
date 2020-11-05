add_library(machine_lib
  runtime/application.cc
  runtime/application.h
  runtime/pixel.cc
  runtime/pixel.h
  runtime/scene.cc
  runtime/scene.h
  asset_loader.cc
  asset_loader.h
  command_buffer.cc
  command_buffer.h
  command_pool.cc
  command_pool.h
  descriptor_pool.cc
  descriptor_pool.h
  fence_waiter.cc
  fence_waiter.h
  glfw.h
  image.cc
  image.h
  image_decoder.cc
  image_decoder.h
  imgui_renderer.cc
  imgui_renderer.h
  key_input.cc
  key_input.h
  key_input_glfw.cc
  key_input_glfw.h
  main_renderer.cc
  main_renderer.h
  matrix_simulation.cc
  matrix_simulation.h
  memory_allocator.cc
  memory_allocator.h
  model.cc
  model.h
  model_draw_data.cc
  model_draw_data.h
  model_renderer.cc
  model_renderer.h
  pipeline_builder.cc
  pipeline_builder.h
  pipeline_layout.cc
  pipeline_layout.h
  pixel_renderer.cc
  pixel_renderer.h
  pointer_input.cc
  pointer_input.h
  queue_selection.cc
  queue_selection.h
  renderer.cc
  renderer.h
  rendering_context.cc
  rendering_context.h
  shader_library.cc
  shader_library.h
  shader_module.cc
  shader_module.h
  shaders/triangle.h
  tiny_gltf.cc
  tiny_gltf.h
  tutorial_renderer.cc
  tutorial_renderer.h
  uniform_buffer.cc
  uniform_buffer.h
  vulkan.h
  vulkan_connection.cc
  vulkan_connection.h
  vulkan_swapchain.cc
  vulkan_swapchain.h
)

target_include_directories(machine_lib
  PUBLIC
    .
    runtime
    ../../third_party/vulkan_memory_allocator/src
)

compile_shaders(machine_lib
  shaders/model_renderer.frag
  shaders/model_renderer.vert
  shaders/triangle.frag
  shaders/triangle.vert
)

epoxy(machine_lib
  ../../idl/cxx_interface.template.epoxy
  runtime/pixel.epoxy
  pixel_c_bindings.h
)

epoxy(machine_lib
  ../../idl/cxx_impl.template.epoxy
  runtime/pixel.epoxy
  pixel_c_bindings.cc
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
    shaderc
    runtime
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
  model_unittests.cc
)

target_link_libraries(machine_unittests
  PRIVATE
    machine_lib
    gtest
    gtest_main
)

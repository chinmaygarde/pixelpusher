
add_executable(machine
  main.cc
  surface.cc
  surface.h
  vulkan_connection.cc
  vulkan_connection.h
  vulkan_swapchain.cc
  vulkan_swapchain.h
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

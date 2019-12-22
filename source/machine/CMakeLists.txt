
add_executable(machine
  main.cc
  surface.h
  surface.cc
  vulkan_connection.cc
  vulkan_connection.h
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

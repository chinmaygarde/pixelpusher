
add_executable(machine
  main.cc
  surface.h
  surface.cc
  vulkan.cc
  vulkan.h
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

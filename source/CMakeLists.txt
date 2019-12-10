
add_executable(pixelpusher
  main.cc
)

target_link_libraries(pixelpusher
  PRIVATE
    glfw
    vk_swiftshader
)

target_include_directories(pixelpusher
  PRIVATE
    ../third_party/swiftshader/include
)

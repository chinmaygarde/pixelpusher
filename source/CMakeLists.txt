
add_executable(pixelpusher
  main.cc
)

target_link_libraries(pixelpusher
  PRIVATE
    glfw
)

target_include_directories(pixelpusher
  PRIVATE
    /usr/local/include
)

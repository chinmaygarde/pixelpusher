
add_executable(pixelpusher_unittests
  main.cc
)

target_link_libraries(pixelpusher_unittests
  PRIVATE
    gtest
    gtest_main
)

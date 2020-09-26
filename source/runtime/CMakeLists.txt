
add_library(runtime
  runtime.h
  runtime.cc
  object.h
  object.cc
)

target_include_directories(runtime
  PUBLIC
    .
)

target_link_libraries(runtime
  PUBLIC
    core
)

depend_on_flutter_engine(runtime)

# Unit tests.
add_executable(runtime_unittests
  object_unittests.cc
)

target_link_libraries(runtime_unittests
  PRIVATE
    runtime
    gtest
    gtest_main
)

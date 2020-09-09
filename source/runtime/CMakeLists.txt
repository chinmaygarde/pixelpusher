
add_library(runtime
  runtime.h
  runtime.cc
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

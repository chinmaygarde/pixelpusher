
add_library(core
  macros.h
  macros.cc
  logging.cc
  logging.h
)

target_include_directories(core PUBLIC .)

target_link_libraries(core PUBLIC absl)


add_library(core
  auto_closure.h
  logging.cc
  logging.h
  macros.cc
  macros.h
)

target_include_directories(core PUBLIC .)


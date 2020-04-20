
add_library(core
  auto_closure.h
  file.cc
  file.h
  logging.cc
  logging.h
  macros.cc
  macros.h
  mapping.cc
  mapping.h
  unique_object.h
  platform.h
)

target_include_directories(core PUBLIC .)


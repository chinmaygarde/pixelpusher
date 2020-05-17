
# Core Library
add_library(core
  closure.h
  event_loop.cc
  event_loop.h
  file.cc
  file.h
  geometry.h
  logging.cc
  logging.h
  macros.cc
  macros.h
  mapping.cc
  mapping.h
  platform.h
  thread.cc
  thread.h
  unique_object.h
  hash.h
  hash.cc
)

target_include_directories(core
  PUBLIC
    .
)

# Core Library Unit-Tests

get_filename_component(FIXTURES_DIRECTORY fixtures ABSOLUTE)
set(P_FIXTURES_LOCATION ${FIXTURES_DIRECTORY})
configure_file(fixture.h.in fixture.h @ONLY)

add_executable(core_unittests
  file_unittests.cc
  event_loop_unittests.cc
)

target_include_directories(core_unittests
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(core_unittests
  PRIVATE
    core
    gtest
    gtest_main
)

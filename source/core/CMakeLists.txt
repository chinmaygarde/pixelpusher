
# Core Library
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

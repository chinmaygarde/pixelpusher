
# Core Library
add_library(core
  closure.cc
  closure.h
  event_loop.cc
  event_loop.h
  file.cc
  file.h
  filesystem_watcher.cc
  filesystem_watcher.h
  filesystem_watcher_win.cc
  filesystem_watcher_win.h
  geometry.h
  hash.cc
  hash.h
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
  unshared_weak.cc
  unshared_weak.h
  win_utils.h
  win_utils.cc
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
  event_loop_unittests.cc
  file_unittests.cc
  unshared_weak_unittests.cc
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

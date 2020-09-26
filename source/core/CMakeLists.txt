
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
  string_utils.cc
  string_utils.h
  thread.cc
  thread.h
  unique_object.h
  unshared_weak.cc
  unshared_weak.h
)

if(WINDOWS)
  target_sources(core PRIVATE
    filesystem_watcher_win.cc
    filesystem_watcher_win.h
    win_utils.cc
    win_utils.h
  )
endif()

if(APPLE)
  set_source_files_properties(file.cc PROPERTIES LANGUAGE OBJCXX)

  target_sources(core PRIVATE
    filesystem_watcher_darwin.cc
    filesystem_watcher_darwin.h
  )

  target_link_libraries(core
    "-framework Foundation"
  )
endif(APPLE)

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
  string_utils_unittests.cc
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

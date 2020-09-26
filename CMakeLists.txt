cmake_minimum_required(VERSION 3.13)

project(pixel_pusher
  LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools")

include(spirv)
include(platform_selection)
include(flutter_engine)
include(pixel_app)
include(epoxy)

detect_platform()

if(WINDOWS AND NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_compile_options(
    -Xclang
    -fno-rtti-data
    -D_HAS_STATIC_RTTI=0
    -D_CRT_SECURE_NO_WARNINGS=0
  )
endif()

set(FLUTTER_ENGINE_DIR "" CACHE PATH "Path to the Flutter Engine Host Debug Output")

if(NOT EXISTS ${FLUTTER_ENGINE_DIR})
  message(FATAL_ERROR "Flutter engine artifacts not specified. Use the FLUTTER_ENGINE_DIR flag.")
endif()

# execute_process(
#   COMMAND ./utils/git-sync-deps
#   WORKING_DIRECTORY third_party/shaderc
# )

add_subdirectory(third_party/glfw           EXCLUDE_FROM_ALL)
add_subdirectory(third_party/vulkan_headers EXCLUDE_FROM_ALL)
add_subdirectory(third_party/glm            EXCLUDE_FROM_ALL)
set(SHADERC_SKIP_TESTS YES)
add_subdirectory(third_party/shaderc        EXCLUDE_FROM_ALL)
add_subdirectory(third_party/googletest     EXCLUDE_FROM_ALL)
add_subdirectory(third_party/stb            EXCLUDE_FROM_ALL)
add_subdirectory(third_party/imgui          EXCLUDE_FROM_ALL)

set(EPOXY_BUILD_EXAMPLES NO)
set(EPOXY_BUILD_TESTS NO)
add_subdirectory(third_party/epoxy          EXCLUDE_FROM_ALL)
add_subdirectory(third_party/tinygltf       EXCLUDE_FROM_ALL)

add_subdirectory(source)
add_subdirectory(dart/runtime)

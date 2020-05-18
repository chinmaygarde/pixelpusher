cmake_minimum_required(VERSION 3.0)

project(pixel_pusher)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools")

include(spirv)
include(platform_selection)

detect_platform()

if(WINDOWS AND NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_compile_options(
    -Xclang
    -fno-rtti-data
    -D_HAS_STATIC_RTTI=0
    -D_CRT_SECURE_NO_WARNINGS=0
  )
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
add_subdirectory(third_party/tinygltf       EXCLUDE_FROM_ALL)

add_subdirectory(source)

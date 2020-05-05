cmake_minimum_required(VERSION 3.0)

project(pixel_pusher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools")

include(spirv)
include(platform_selection)

detect_platform()

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

add_subdirectory(source)

cmake_minimum_required(VERSION 3.0)

project(pixel_pusher)

set (CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools")

include(spirv)
include(platform_selection)

detect_platform()

set(GLFW_BUILD_EXAMPLES   OFF CACHE BOOL "GLFW_BUILD_EXAMPLES" FORCE)
set(GLFW_BUILD_TESTS      OFF CACHE BOOL "GLFW_BUILD_TESTS" FORCE)
set(GLFW_BUILD_DOCS       OFF CACHE BOOL "GLFW_BUILD_DOCS" FORCE)

add_subdirectory(third_party/glfw EXCLUDE_FROM_ALL)
# add_subdirectory(third_party/googletest)
add_subdirectory(third_party/vulkan_headers EXCLUDE_FROM_ALL)
add_subdirectory(third_party/glm EXCLUDE_FROM_ALL)

# execute_process(
#   COMMAND ./utils/git-sync-deps
#   WORKING_DIRECTORY third_party/shaderc
# )
set(SHADERC_SKIP_TESTS    ON CACHE BOOL "SHADERC_SKIP_TESTS" FORCE)
add_subdirectory(third_party/shaderc EXCLUDE_FROM_ALL)

add_subdirectory(source)

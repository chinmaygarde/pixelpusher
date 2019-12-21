cmake_minimum_required(VERSION 3.0)

project(pixel_pusher)

set (CMAKE_CXX_STANDARD 17)

add_subdirectory(third_party/googletest)
add_subdirectory(third_party/abseil)

set(GLFW_BUILD_EXAMPLES   OFF CACHE BOOL "GLFW_BUILD_EXAMPLES" FORCE)
set(GLFW_BUILD_TESTS      OFF CACHE BOOL "GLFW_BUILD_TESTS" FORCE)
set(GLFW_BUILD_DOCS       OFF CACHE BOOL "GLFW_BUILD_DOCS" FORCE)
add_subdirectory(third_party/glfw)

add_subdirectory(source)

cmake_minimum_required(VERSION 3.0)

project(pixel_pusher)

add_subdirectory(third_party/googletest)

set(GLFW_BUILD_EXAMPLES   OFF CACHE BOOL "GLFW_BUILD_EXAMPLES")
set(GLFW_BUILD_TESTS      OFF CACHE BOOL "GLFW_BUILD_TESTS")
set(GLFW_BUILD_DOCS       OFF CACHE BOOL "GLFW_BUILD_DOCS")
set(GLFW_VULKAN_STATIC    ON  CACHE BOOL "GLFW_VULKAN_STATIC")
add_subdirectory(third_party/glfw)

set(BUILD_SAMPLES      OFF CACHE BOOL "BUILD_SAMPLES")
set(BUILD_TESTS        OFF CACHE BOOL "BUILD_TESTS")
set(BUILD_GLESv2       OFF CACHE BOOL "BUILD_GLESv2")
set(BUILD_GLES_CM      OFF CACHE BOOL "BUILD_GLES_CM")
set(SPIRV_SKIP_TESTS   ON  CACHE BOOL "SPIRV_SKIP_TESTS")
add_subdirectory(third_party/swiftshader)

add_subdirectory(source)
add_subdirectory(test)

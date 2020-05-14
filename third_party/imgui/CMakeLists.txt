add_library(imgui
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui.h
  imgui/examples/imgui_impl_vulkan.h
  imgui/examples/imgui_impl_vulkan.cpp
  imgui/examples/imgui_impl_glfw.cpp
  imgui/examples/imgui_impl_glfw.h
)

target_compile_definitions(imgui
  PRIVATE
    VK_NO_PROTOTYPES
)

target_include_directories(imgui
  PUBLIC
    imgui
    imgui/examples
)

target_link_libraries(imgui
  PRIVATE
    Vulkan::Headers
    glfw
)

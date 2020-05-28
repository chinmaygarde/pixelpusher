add_library(imgui
  imgui/examples/imgui_impl_glfw.cpp
  imgui/examples/imgui_impl_glfw.h
  imgui/examples/imgui_impl_vulkan.cpp
  imgui/examples/imgui_impl_vulkan.h
  imgui/imgui.cpp
  imgui/imgui.h
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
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

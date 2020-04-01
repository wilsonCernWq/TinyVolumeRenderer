# ImGui
include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/examples/opengl3_example)
add_library(imgui
  imgui.cpp
  imgui_draw.cpp
  imgui_demo.cpp
  imgui_widgets.cpp
  examples/imgui_impl_glfw.cpp
  examples/imgui_impl_opengl3.cpp)
target_link_libraries(imgui ${OPENGL_LIBS})
set(ImGUI_LIBRARIES imgui PARENT_SCOPE)
set(ImGUI_INCLUDE_DIR
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/examples/
  PARENT_SCOPE)


# Visual Sandbox (需要 SFML + ImGui)

# ===== ImGui 源文件（直接内联编译，避免子项目依赖） =====
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui)
set(IMGUI_SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui-sfml)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_SFML_DIR}/imgui-SFML.cpp
)

# 可视化沙盒测试
add_executable(VisualSandbox VisualSandbox.cpp ${IMGUI_SOURCES})

# ImGui 头文件
target_include_directories(VisualSandbox PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_SFML_DIR}
)

target_link_libraries(VisualSandbox PRIVATE ${PROJECT_NAME})

# 链接 SFML（只在沙盒中，不污染核心逻辑）
if(APPLE)
    find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
    target_link_libraries(VisualSandbox PRIVATE SFML::Graphics SFML::Window SFML::System)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VisualSandbox PRIVATE OpenGL::GL)
elseif(WIN32)
    find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
    target_link_libraries(VisualSandbox PRIVATE sfml-graphics sfml-window sfml-system)
    target_link_libraries(VisualSandbox PRIVATE opengl32 imm32)
elseif(UNIX)
    find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
    target_link_libraries(VisualSandbox PRIVATE sfml-graphics sfml-window sfml-system)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VisualSandbox PRIVATE OpenGL::GL)
endif()

# 设置输出目录
set_target_properties(VisualSandbox PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 复制 data 目录到构建输出目录
add_custom_command(TARGET VisualSandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_SOURCE_DIR}/data
        ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying data files to build directory"
)

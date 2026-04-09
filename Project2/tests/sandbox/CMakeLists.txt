# Visual Sandbox (需要 SFML)

# 可视化沙盒测试
add_executable(VisualSandbox VisualSandbox.cpp)
target_link_libraries(VisualSandbox PRIVATE ${PROJECT_NAME})

# 链接 SFML（只在沙盒中，不污染核心逻辑）
if(APPLE)
    find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
    target_link_libraries(VisualSandbox PRIVATE SFML::Graphics SFML::Window SFML::System)
elseif(WIN32)
    find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
    target_link_libraries(VisualSandbox PRIVATE sfml-graphics sfml-window sfml-system)
elseif(UNIX)
    find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
    target_link_libraries(VisualSandbox PRIVATE sfml-graphics sfml-window sfml-system)
endif()

# 设置输出目录
set_target_properties(VisualSandbox PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 设置最低 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 定义项目名称
project(Project1)

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 查找所有源文件
file(GLOB SRC_FILES "src/*.cpp")

# 添加主目标（可执行文件或库）
add_library(${PROJECT_NAME} ${SRC_FILES})  # 如果需要构建库，可以使用 add_library

# 链接外部库（SFML 等）
link_libraries(
    sfml-graphics-s
    sfml-window-s
    sfml-system-s
    sfml-audio-s
    opengl32
    freetype
    winmm
    gdi32
)

# 设置输出目录（可选）
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_subdirectory(test)
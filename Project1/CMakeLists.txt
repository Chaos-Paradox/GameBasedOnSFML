# 设置 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(Project1)

# 设置头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 设置源文件
file(GLOB SRC_FILES "src/*.cpp")

# 创建库
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

# 链接 SFML (使用 find_package 找到的 SFML target)
find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_subdirectory(test)

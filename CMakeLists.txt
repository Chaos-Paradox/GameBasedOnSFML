cmake_minimum_required(VERSION 3.27)

project(GameBasedOnSFML)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
option(CMAKE_BUILD_TYPE "Release")
    

# 设置字体文件路径
set(FONT_PATH "${CMAKE_SOURCE_DIR}/material/fonts/Arial.ttf")

# 添加该变量为预定义宏，使得在代码中可以使用
add_definitions(-DFONT_PATH="${FONT_PATH}")

# 使用 Homebrew 安装的 SFML (SFML 3.0)
find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/ThirdParty
)

add_subdirectory(Project1)
add_subdirectory(Project2)

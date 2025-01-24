cmake_minimum_required(VERSION 3.10)

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

# 定义字体文件的相对路径
set(FONT_PATH "${CMAKE_SOURCE_DIR}/material/fonts/Arial.ttf")

# 添加该变量为预处理宏，使其在代码中可用
add_definitions(-DFONT_PATH="${FONT_PATH}")

include_directories(
    ${PROJECT_SOURCE_DIR}/ThirdParty/SFML/include
)

link_directories(
    ${PROJECT_SOURCE_DIR}/ThirdParty/SFML/lib
)

link_libraries(
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    opengl32
    freetype
    winmm
    gdi32
)

set(SFML_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/SFML")

# 自定义目标来复制 DLL 文件
add_custom_target(CopyDLLs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${SFML_DIR}/bin"
    "${CMAKE_BINARY_DIR}/bin"
)


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
    

# ���������ļ������·��
set(FONT_PATH "${CMAKE_SOURCE_DIR}/material/fonts/Arial.ttf")

# ���Ӹñ���ΪԤ�����꣬ʹ���ڴ����п���
add_definitions(-DFONT_PATH="${FONT_PATH}")

# 使用 Homebrew 安装的 SFML (SFML 3.0 组件名是小写)
find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/ThirdParty
)



# �Զ���Ŀ�������� DLL �ļ�
#add_custom_target(CopyDLLs ALL
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    "${SFML_DIR}/bin"
#    "${CMAKE_BINARY_DIR}/bin"
#)

add_subdirectory(Project1)
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

# 跨平台 SFML 链接
if(APPLE)
    # macOS - 使用 CMake targets
    find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)
    
elseif(WIN32)
    # Windows - 使用静态库
    target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics-s
        sfml-window-s
        sfml-system-s
        sfml-audio-s
        opengl32
        freetype
        winmm
        gdi32
    )
    
elseif(UNIX)
    # Linux - 使用 CMake targets
    find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Audio SFML::Window SFML::System)
endif()

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_subdirectory(test)

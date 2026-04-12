# Project2 - 核心游戏架构（与渲染解耦）
cmake_minimum_required(VERSION 3.10)

project(Project2)

# 选项：是否启用 SFML 渲染后端
option(ENABLE_SFML "Enable SFML rendering backend" OFF)

# 设置头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 设置源文件
file(GLOB SRC_FILES "src/*.cpp")

# 创建核心库（不依赖 SFML）
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

# 可选：链接 SFML
if(ENABLE_SFML)
    message(STATUS "SFML rendering backend enabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLE_SFML)
    
    if(APPLE)
        find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
        target_link_libraries(${PROJECT_NAME} PUBLIC SFML::Graphics SFML::Audio SFML::Window SFML::System)
        
    elseif(WIN32)
        set(SFML_STATIC_LIBRARIES ON)
        add_definitions(-DSFML_STATIC)
        target_link_libraries(${PROJECT_NAME} PUBLIC
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
        find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
        target_link_libraries(${PROJECT_NAME} PUBLIC SFML::Graphics SFML::Audio SFML::Window SFML::System)
    endif()
else()
    message(STATUS "SFML rendering backend disabled - core architecture only")
endif()

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 沙盒测试（可选，需要 SFML）
if(ENABLE_SFML)
    add_subdirectory(tests/sandbox)
endif()

# 测试
add_subdirectory(test)

# 单元测试
add_subdirectory(tests/unit)

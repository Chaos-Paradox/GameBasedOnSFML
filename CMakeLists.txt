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

# GTest 可选支持
option(ENABLE_TESTS "Enable unit tests with Google Test" OFF)

if(ENABLE_TESTS)
    message(STATUS "Unit tests enabled")
    
    # 优先使用系统安装的 GTest，失败时尝试 FetchContent
    find_package(GTest QUIET)
    if(GTest_FOUND)
        message(STATUS "Using system GTest: ${GTest_VERSION}")
    else()
        message(STATUS "System GTest not found, using FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        # Windows MSVC: 使用静态运行时，避免 DLL 问题
        if(MSVC)
            set(gtest_force_shared_crt OFF CACHE BOOL "Use static runtime" FORCE)
        endif()
        FetchContent_MakeAvailable(googletest)
    endif()
    
    enable_testing()
endif()

# 设置字体文件路径（跨平台）
set(FONT_PATH "${CMAKE_SOURCE_DIR}/material/fonts/Arial.ttf")
add_definitions(-DFONT_PATH="${FONT_PATH}")

# 跨平台 SFML 配置
if(APPLE)
    # macOS - 使用 Homebrew 安装的 SFML
    message(STATUS "Configuring for macOS")
    find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
    include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)
    
elseif(WIN32)
    # Windows - 使用 ThirdParty 目录的 SFML
    message(STATUS "Configuring for Windows")
    set(SFML_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/SFML")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${SFML_DIR}")
    set(SFML_STATIC_LIBRARIES ON)
    add_definitions(-DSFML_STATIC)
    
    include_directories(
        ${PROJECT_SOURCE_DIR}/ThirdParty/SFML/include
    )
    link_directories(
        ${PROJECT_SOURCE_DIR}/ThirdParty/SFML/lib
    )
    
    # Windows 特定库
    set(SFML_LIBRARIES 
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
    # Linux - 使用系统 SFML
    message(STATUS "Configuring for Linux")
    find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
endif()

add_subdirectory(Project1)
add_subdirectory(Project2)

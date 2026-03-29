project(test)

# 跨平台 SFML 配置
if(APPLE)
    # macOS
    find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
    set(SFML_LIBS SFML::Graphics SFML::Audio SFML::Window SFML::System)
    
elseif(WIN32)
    # Windows
    set(SFML_LIBS 
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
    # Linux
    find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
    set(SFML_LIBS SFML::Graphics SFML::Audio SFML::Window SFML::System)
endif()

# 测试程序
add_executable("test" "test.cpp")
target_link_libraries("test" Project1 ${SFML_LIBS})

add_executable("test2" "test2.cpp")
target_link_libraries("test2" Project1 ${SFML_LIBS})

add_executable("walktest" "walktest.cpp")
target_link_libraries("walktest" Project1 ${SFML_LIBS})

add_executable("tmptest" "tmptest.cpp")
target_link_libraries("tmptest" Project1 ${SFML_LIBS})

add_executable("maptest" "maptest.cpp")
target_link_libraries("maptest" Project1 ${SFML_LIBS})

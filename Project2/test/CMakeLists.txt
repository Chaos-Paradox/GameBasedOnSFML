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
add_executable("attack_test" "attack_test.cpp")
target_link_libraries("attack_test" Project2 ${SFML_LIBS})

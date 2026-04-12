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

# 简单测试程序（保留向后兼容）
# 注意：启用 CTest 时不能使用 "test" 作为目标名
if(ENABLE_TESTS)
    add_executable("test1_runner" "test.cpp")
    target_link_libraries("test1_runner" Project1 ${SFML_LIBS})
else()
    add_executable("test" "test.cpp")
    target_link_libraries("test" Project1 ${SFML_LIBS})
endif()

add_executable("test2" "test2.cpp")
target_link_libraries("test2" Project1 ${SFML_LIBS})

add_executable("walktest" "walktest.cpp")
target_link_libraries("walktest" Project1 ${SFML_LIBS})

add_executable("tmptest" "tmptest.cpp")
target_link_libraries("tmptest" Project1 ${SFML_LIBS})

add_executable("maptest" "maptest.cpp")
target_link_libraries("maptest" Project1 ${SFML_LIBS})

# GTest 单元测试（可选）
if(ENABLE_TESTS)
    # 示例：将现有测试迁移到 GTest
    # add_executable("unit_tests" "unit_tests.cpp")
    # target_link_libraries("unit_tests" Project1 GTest::gtest_main ${SFML_LIBS})
    # add_test(NAME UnitTests COMMAND unit_tests)
    message(STATUS "GTest available for new unit tests in Project1/test")
endif()

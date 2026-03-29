project(test)

# 找到 SFML
find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)

add_executable("test" "test.cpp")
target_link_libraries("test" Project1 SFML::Graphics SFML::Audio SFML::Window SFML::System)

add_executable("test2" "test2.cpp")
target_link_libraries("test2" Project1 SFML::Graphics SFML::Audio SFML::Window SFML::System)

add_executable("walktest" "walktest.cpp")
target_link_libraries("walktest" Project1 SFML::Graphics SFML::Audio SFML::Window SFML::System)

add_executable("tmptest" "tmptest.cpp")
target_link_libraries("tmptest" Project1 SFML::Graphics SFML::Audio SFML::Window SFML::System)

add_executable("maptest" "maptest.cpp")
target_link_libraries("maptest" Project1 SFML::Graphics SFML::Audio SFML::Window SFML::System)

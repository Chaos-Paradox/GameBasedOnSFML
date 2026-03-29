project(test)

add_executable("test" "test.cpp")
target_link_libraries("test" Project1 SFML::Graphics SFML::Audio SFML::Window SFML::System)

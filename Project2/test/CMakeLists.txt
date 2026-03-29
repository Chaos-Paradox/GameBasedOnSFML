project(test)

# 找到 SFML
find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)

add_executable("attack_test" "attack_test.cpp")
target_link_libraries("attack_test" Project2 SFML::Graphics SFML::Audio SFML::Window SFML::System)

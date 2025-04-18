project(test)

add_executable("test" "test.cpp")
target_link_libraries("test" Project1)

add_executable("test2" "test2.cpp")
target_link_libraries("test2" Project1)
project(test)

add_executable("test" "test.cpp")
target_link_libraries("test" Project1)

add_executable("test2" "test2.cpp")
target_link_libraries("test2" Project1)

add_executable("walktest" "walktest.cpp")
target_link_libraries("walktest" Project1)

add_executable("tmptest" "tmptest.cpp")
target_link_libraries("tmptest" Project1)

add_executable("maptest" "maptest.cpp")
target_link_libraries("maptest" Project1)
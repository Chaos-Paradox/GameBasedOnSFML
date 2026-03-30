# Project2 GTest 测试程序
project(test)

find_package(GTest REQUIRED)

# 战利品管线测试（GTest）
add_executable("loot_pipeline_test" "loot_pipeline_test.cpp")
target_link_libraries("loot_pipeline_test" Project2 GTest::gtest_main)

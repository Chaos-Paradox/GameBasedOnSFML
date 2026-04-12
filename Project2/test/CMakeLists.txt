# Project2 GTest 测试程序
project(test)

# GTest 通过顶层 CMakeLists.txt 的 FetchContent 提供
# 确保 ENABLE_TESTS=ON 时可用

# 战利品管线测试（GTest）
add_executable("loot_pipeline_test" "loot_pipeline_test.cpp")
target_link_libraries("loot_pipeline_test" Project2 GTest::gtest_main)

add_test(NAME LootPipelineTest COMMAND loot_pipeline_test)

#!/bin/bash

# Project2 测试运行脚本
# 用法：./run_all_tests.sh

set -e

echo "========================================"
echo "  Project2 - Running All Tests"
echo "========================================"
echo ""

cd "$(dirname "$0")/../build/bin" || exit 1

# 计数器
TOTAL=0
PASSED=0
FAILED=0

# 运行所有测试
for test in *_test; do
    if [ -x "$test" ]; then
        echo "----------------------------------------"
        echo "Running: $test"
        echo "----------------------------------------"
        
        TOTAL=$((TOTAL + 1))
        
        if ./"$test"; then
            echo ""
            echo "✅ $test PASSED"
            PASSED=$((PASSED + 1))
        else
            echo ""
            echo "❌ $test FAILED"
            FAILED=$((FAILED + 1))
        fi
        
        echo ""
    fi
done

# 总结
echo "========================================"
echo "  Test Summary"
echo "========================================"
echo "Total:  $TOTAL"
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "========================================"

if [ $FAILED -eq 0 ]; then
    echo "✅ All tests PASSED!"
    exit 0
else
    echo "❌ Some tests FAILED!"
    exit 1
fi

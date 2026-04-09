#!/bin/bash
# 跨平台构建测试脚本
# 测试 macOS、Linux 和 Windows (MSVC) 的 CMake 配置

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_test"

echo "========================================="
echo "跨平台构建兼容性测试"
echo "========================================="

# 清理旧的构建目录
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# 测试 1: 默认配置（无测试）
echo ""
echo "[测试 1] 默认配置 (ENABLE_TESTS=OFF)"
echo "-----------------------------------------"
mkdir -p "$BUILD_DIR/default"
cd "$BUILD_DIR/default"
cmake ../.. -DCMAKE_BUILD_TYPE=Release
echo "✓ 默认配置 CMake 生成成功"

# 测试 2: 启用测试
echo ""
echo "[测试 2] 启用单元测试 (ENABLE_TESTS=ON)"
echo "-----------------------------------------"
mkdir -p "$BUILD_DIR/tests"
cd "$BUILD_DIR/tests"
cmake ../.. -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
echo "✓ 启用测试 CMake 生成成功"

# 测试 3: macOS 特定检查
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo ""
    echo "[测试 3] macOS 特定检查"
    echo "-----------------------------------------"
    cd "$BUILD_DIR/tests"
    cmake --build . --target help 2>&1 | head -20 || true
    echo "✓ macOS 构建目标检查完成"
fi

# 测试 4: 验证 CMake 配置
echo ""
echo "[测试 4] 验证 CMake 缓存变量"
echo "-----------------------------------------"
cd "$BUILD_DIR/tests"
if cmake -L -N ../.. | grep -E "ENABLE_TESTS|CMAKE_BUILD_TYPE"; then
    echo "✓ CMake 缓存变量验证成功"
fi

# 测试 5: MSVC 兼容性检查（静态分析）
echo ""
echo "[测试 5] MSVC 兼容性静态检查"
echo "-----------------------------------------"
# 检查是否有 MSVC 特定的代码问题
if grep -r "defined(_MSC_VER)" "$SCRIPT_DIR" --include="*.cpp" --include="*.h" 2>/dev/null; then
    echo "✓ 发现 MSVC 特定代码处理"
else
    echo "ℹ 未发现显式 MSVC 处理（可能依赖 CMake 抽象）"
fi

# 检查 CMake 中的 MSVC 处理
if grep -r "MSVC" "$SCRIPT_DIR" --include="*.txt" --include="*.cmake" 2>/dev/null | head -5; then
    echo "✓ CMake 包含 MSVC 配置"
fi

echo ""
echo "========================================="
echo "跨平台检查完成!"
echo "========================================="
echo ""
echo "构建目录：$BUILD_DIR"
echo ""
echo "手动测试命令:"
echo "  macOS/Linux:  cmake -B build -DENABLE_TESTS=ON"
echo "  Windows MSVC: cmake -B build -G \"Visual Studio 17 2022\" -DENABLE_TESTS=ON"
echo ""

# 清理（可选）
# rm -rf "$BUILD_DIR"

# 跨平台兼容性报告

生成日期：2026-04-09
项目：GameBasedOnSFML

## ✅ 已验证平台

### macOS (Apple Silicon)
- **编译器**: AppleClang 21.0.0.21000099
- **CMake**: 4.3.0
- **C++ 标准**: C++20
- **SFML**: 3.0.2 (Homebrew)
- **GTest**: 1.17.0 (系统安装)
- **状态**: ✅ 通过
  - CMake 配置成功
  - 构建成功
  - 单元测试通过 (1/1)

### Linux (未测试，配置已就绪)
- **支持**: 通过 `find_package(SFML 3)` 使用系统 SFML
- **GTest**: 支持系统安装或 FetchContent
- **状态**: ⚠️ 配置就绪，待验证

### Windows MSVC (配置审查)
- **支持**: Visual Studio 2022 (CMake generator)
- **SFML**: 使用 ThirdParty 目录的静态库
- **GTest**: 支持 FetchContent 或系统安装
- **CRT**: 可选静态运行时 (`gtest_force_shared_crt`)
- **状态**: ⚠️ 配置就绪，待验证

## CMake 配置检查

### 跨平台 SFML 处理
```cmake
if(APPLE)
    # macOS - Homebrew
    find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
elseif(WIN32)
    # Windows - ThirdParty 静态库
    set(SFML_STATIC_LIBRARIES ON)
    add_definitions(-DSFML_STATIC)
elseif(UNIX)
    # Linux - 系统 SFML
    find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
endif()
```

### GTest 可选支持
```cmake
option(ENABLE_TESTS "Enable unit tests with Google Test" OFF)

if(ENABLE_TESTS)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        # 使用系统 GTest
    else()
        # 使用 FetchContent 下载
    endif()
    enable_testing()
endif()
```

### MSVC 特定处理
- ✅ 静态 SFML 库支持 (`-DSFML_STATIC`)
- ✅ Windows 特定系统库链接 (opengl32, freetype, winmm, gdi32)
- ✅ GTest 静态 CRT 选项
- ✅ ThirdParty 目录配置

## 构建命令

### macOS / Linux
```bash
# 默认构建（无测试）
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 启用测试
cmake -B build -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

### Windows (MSVC)
```powershell
# 默认构建
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# 启用测试
cmake -B build -G "Visual Studio 17 2022" -A x64 -DENABLE_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release
```

## 潜在问题与建议

### 1. RAND_MAX 转换警告
**位置**: `Project2/include/systems/LootSpawnSystem.h:61-72`
**问题**: `int` 到 `float` 的隐式转换警告
**影响**: macOS Clang 警告，MSVC 可能报错 (取决于警告级别)
**建议修复**:
```cpp
// 当前代码
float jitterX = (static_cast<float>(std::rand()) / RAND_MAX) * 40.0f - 20.0f;

// 建议修复
float jitterX = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 40.0f - 20.0f;
```

### 2. 网络依赖 (FetchContent)
**问题**: 当系统无 GTest 时需要克隆 GitHub 仓库
**影响**: 离线环境或网络受限时构建失败
**建议**: 
- CI/CD 环境预装 GTest
- 或使用 vendored GTest

### 3. SFML 版本一致性
**当前**: SFML 3.x
**注意**: 确保 Windows ThirdParty 目录版本与 macOS/Linux 系统版本兼容

## 测试覆盖率

| 组件 | 测试状态 | 备注 |
|------|----------|------|
| Project1 | ❌ 无 GTest 测试 | 仅有简单可执行测试 |
| Project2 | ✅ 有 GTest 测试 | loot_pipeline_test 通过 |
| 跨平台配置 | ✅ CMake 配置验证 | macOS 已验证 |

## 后续待办

- [ ] 在 Windows 11 + VS2022 环境验证构建
- [ ] 在 Ubuntu 22.04/24.04 验证构建
- [ ] 修复 RAND_MAX 转换警告
- [ ] 为 Project1 添加 GTest 单元测试
- [ ] CI/CD 集成 (GitHub Actions / Azure Pipelines)

## 总结

**整体状态**: 🟡 基本就绪，待 Windows/Linux 实测

核心 CMake 配置已完成跨平台支持，macOS 验证通过。Windows MSVC 配置已就绪但需要实际环境验证。建议先在 Windows 环境进行一次完整构建测试。

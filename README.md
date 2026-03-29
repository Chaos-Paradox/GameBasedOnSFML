# SFMLGame

基于 SFML 库的游戏项目，支持跨平台开发。

## 跨平台支持

- ✅ macOS (Homebrew SFML)
- ✅ Windows (静态库)
- ✅ Linux (系统 SFML)

## 编译方法

### macOS

```bash
# 安装依赖
brew install sfml cmake

# 编译
mkdir build && cd build
cmake ..
make

# 运行
./build/bin/walktest
```

### Windows

```bash
# 1. 确保 ThirdParty/SFML 目录包含 SFML Windows 静态库
# 2. 使用 Visual Studio 或 MinGW

# Visual Studio
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release

# MinGW
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### Linux

```bash
# Ubuntu/Debian
sudo apt install libsfml-dev cmake

# 编译
mkdir build && cd build
cmake ..
make
```

## 项目结构

```
SFMLGame/
├── CMakeLists.txt          # 主 CMake 配置（跨平台）
├── Project1/               # 项目 1（ECS 框架）
│   ├── CMakeLists.txt
│   ├── src/
│   ├── include/
│   └── test/
├── Project2/               # 项目 2
│   ├── CMakeLists.txt
│   ├── src/
│   └── test/
├── ThirdParty/             # 第三方库（Windows SFML）
└── material/               # 游戏素材
    ├── fonts/
    └── pictures/
```

## 测试程序

- `test` / `test2` - 基础测试
- `walktest` - 角色移动测试
- `maptest` - 地图渲染测试
- `attack_test` - 攻击系统测试

## 注意事项

1. **路径问题**：代码中使用相对路径 `material/`，需要从项目根目录运行可执行文件
2. **Windows 路径**：素材路径已适配为跨平台格式（使用 `/` 而非 `\`）
3. **SFML 版本**：推荐使用 SFML 3.x

## License

见 LICENSE 文件

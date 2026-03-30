# SFMLGame

基于 SFML 库的游戏项目，支持跨平台开发。

## 项目结构

本项目采用多 Project 架构，每个 Project 独立管理：

| Project | 说明 | 文档 |
|---------|------|------|
| **Project1** | ECS 框架（通用基础） | [Project1/docs/](Project1/docs/) |
| **Project2** | 游戏逻辑（状态机、战斗） | [Project2/docs/](Project2/docs/) |
| **ThirdParty** | 第三方库（SFML Windows） | - |
| **material** | 游戏素材 | - |

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

## 📚 文档导航

### Project1 - ECS 框架

- [架构总览](Project1/docs/00_ARCHITECTURE.md) - ECS 核心设计
- [数据字典](Project1/docs/01_DATA_SCHEMA.md) - 组件数据结构
- [功能模块](Project1/docs/features/) - 系统文档索引

### Project2 - 游戏逻辑

- [架构总览](Project2/docs/00_ARCHITECTURE.md) - 游戏架构和状态机
- [数据字典](Project2/docs/01_DATA_SCHEMA.md) - 状态和组件定义
- [功能模块](Project2/docs/features/) - 功能文档索引
- [开发状态](Project2/STATUS.md) - 当前进度

### 通用文档

- [部署指南](DEPLOY.md) - 编译和打包
- [许可证](LICENSE) - MIT License

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

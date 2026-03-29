# SFMLGame 部署指南

## 开发阶段注意事项

### macOS 输入权限

**现象：** 从终端运行程序时，macOS 可能提示"输入监控"权限请求。

**原因：**
- macOS 安全机制对全局键盘监听的敏感度较高
- 从终端启动可能触发进程组关联检测
- **不是 SFML 依赖终端**，程序本身是独立的

**解决方案：**
- 开发阶段：在系统设置中允许终端的输入监控权限
- 或使用独立终端应用（如 iTerm2）运行程序

**当前输入实现：**
```cpp
// 使用 SFML 3.0 的 Scan 枚举（物理扫描码）
sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)
```
- ✅ 跨平台兼容（Windows/macOS/Linux）
- ✅ 支持多键同时按下
- ✅ 不依赖终端，直接从系统获取键盘状态

---

## 生产环境打包

### macOS - 打包为 .app 应用

#### 1. 修改 CMakeLists.txt

```cmake
# 在顶层 CMakeLists.txt 添加
if(APPLE)
    set(MACOSX_BUNDLE TRUE)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourname.sfmlgame")
    set(MACOSX_BUNDLE_BUNDLE_NAME "SFML Game")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist.in")
endif()

# 为每个可执行目标设置
set_target_properties(walktest PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourname.walktest"
)
```

#### 2. 创建 Info.plist.in

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>${MACOSX_BUNDLE_EXECUTABLE_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>${MACOSX_BUNDLE_GUI_IDENTIFIER}</string>
    <key>CFBundleName</key>
    <string>${MACOSX_BUNDLE_BUNDLE_NAME}</string>
    <key>CFBundleVersion</key>
    <string>${MACOSX_BUNDLE_BUNDLE_VERSION}</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleIconFile</key>
    <string>icon.icns</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
```

#### 3. 添加资源文件

```cmake
# 复制素材到 app bundle
set_target_properties(walktest PROPERTIES
    RESOURCE "${CMAKE_SOURCE_DIR}/material"
)
```

#### 4. 编译打包

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# 生成的 .app 在 build/bin/ 目录
open bin/walktest.app
```

#### 5. 代码签名（可选，减少安全警告）

```bash
# 自签名（开发用）
codesign --force --deep --sign - walktest.app

# 或使用开发者证书（发布用）
codesign --force --deep --sign "Developer ID Application: Your Name" walktest.app
```

---

### Windows - 打包为独立 EXE

#### 1. 静态链接 SFML

```cmake
# CMakeLists.txt 已配置
set(SFML_STATIC_LIBRARIES ON)
add_definitions(-DSFML_STATIC)
```

#### 2. 编译 Release 版本

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

#### 3. 复制依赖 DLL（如果动态链接）

```bash
# 从 SFML/bin 复制 DLL 到可执行文件目录
copy /Y ThirdParty/SFML/bin/*.dll build/bin/Release/
```

#### 4. 创建安装包（可选）

使用 Inno Setup 或 NSIS 创建安装程序。

---

### Linux - 打包

#### 1. 编译

```bash
sudo apt install libsfml-dev cmake
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

#### 2. 创建 .deb 包（Debian/Ubuntu）

使用 `cpack` 或手动创建 deb 包。

---

## 跨平台分发清单

- [ ] macOS: `.app` bundle + 代码签名
- [ ] Windows: `.exe` + 依赖 DLL + Visual C++ Redistributable
- [ ] Linux: 可执行文件 + 依赖库说明
- [ ] 素材文件：`material/` 目录
- [ ] 配置文件：如有需要
- [ ] README：安装和运行说明
- [ ] LICENSE：许可证文件

---

## 常见问题

### Q: macOS 提示"无法打开，因为无法验证开发者"
**A:** 右键点击 .app → 打开，或在系统设置中允许。

### Q: Windows 提示"缺少 VCRUNTIME140.dll"
**A:** 安装 Visual C++ Redistributable，或静态链接运行时。

### Q: 程序运行但黑屏/无响应
**A:** 检查素材路径是否正确，确保从正确目录运行。

---

## 当前开发配置

- **SFML 版本：** 3.0.2
- **CMake 版本：** 3.27+
- **输入方式：** `sf::Keyboard::Scan::` (物理扫描码)
- **路径格式：** 相对路径 `material/`
- **运行方式：** 从项目根目录运行 `./build/bin/walktest`

---

**最后更新：** 2026-03-29
**状态：** 开发中

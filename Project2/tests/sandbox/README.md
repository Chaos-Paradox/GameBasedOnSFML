# Visual Sandbox - 可视化沙盒测试

## 🎯 用途

可视化验证 Project2 核心逻辑（无 SFML 依赖），用于：
- 验证运动管线（StateMachine → Locomotion → Movement）
- 调试角色移动和状态切换
- 快速原型测试

## 🏗️ 架构隔离

```
┌─────────────────────────────────────────────────────────┐
│  VisualMovementTest.cpp (沙盒)                          │
│  ✅ 包含 SFML 头文件                                     │
│  ✅ 读取键盘输入                                        │
│  ✅ 渲染图形                                            │
└─────────────────────────────────────────────────────────┘
                          ↓ 链接
┌─────────────────────────────────────────────────────────┐
│  Project2 (静态库 - 纯净)                                │
│  ❌ 无 SFML 依赖                                         │
│  ✅ StateMachineSystem                                  │
│  ✅ LocomotionSystem                                    │
│  ✅ MovementSystem                                      │
└─────────────────────────────────────────────────────────┘
```

## 🚀 编译和运行

### 前提条件

已安装 SFML 3.x：
```bash
# macOS
brew install sfml

# Ubuntu/Debian
sudo apt install libsfml-dev

# Windows (vcpkg)
vcpkg install sfml
```

### 编译

```bash
cd <项目根目录>/Project2

# 启用 SFML 后端并编译沙盒
cmake -S . -B build -DENABLE_SFML=ON
cmake --build build --target VisualMovementTest
```

### 运行

```bash
./build/bin/VisualMovementTest
```

## 🎮 控制

| 按键 | 功能 |
|------|------|
| `W` / `↑` | 向上移动 |
| `S` / `↓` | 向下移动 |
| `A` / `←` | 向左移动 |
| `D` / `→` | 向右移动 |
| `ESC` | 退出 |

## 🎨 视觉反馈

| 颜色 | 状态 |
|------|------|
| 🔵 蓝色 (Idle) | 待机状态 |
| 🟢 绿色 (Move) | 移动状态 |

## 📊 调试信息

控制台实时输出：
- 当前状态（Idle/Move）
- 位置坐标 (x, y)
- 速度向量 (vx, vy)

## 📝 代码结构

```
VisualMovementTest.cpp
├── SFML 窗口初始化
├── ECS 和 Component 初始化
├── 主循环
│   ├── 1. 处理 SFML 事件
│   ├── 2. 读取键盘输入 → InputCommand
│   ├── 3. 更新 Project2 管线
│   │   ├── StateMachineSystem.update()
│   │   ├── LocomotionSystem.update()
│   │   └── MovementSystem.update()
│   └── 4. SFML 渲染
└── 清理和退出
```

## 🔧 扩展沙盒

添加新的可视化测试：

1. 在 `tests/sandbox/` 创建新文件 `VisualXXXTest.cpp`
2. 在 `tests/sandbox/CMakeLists.txt` 添加：
   ```cmake
   add_executable(VisualXXXTest VisualXXXTest.cpp)
   target_link_libraries(VisualXXXTest PRIVATE ${PROJECT_NAME} SFML::...)
   ```
3. 重新编译：
   ```bash
   cmake --build build --target VisualXXXTest
   ```

## ⚠️ 注意事项

1. **SFML 只存在于沙盒** - 核心逻辑保持纯净
2. **不要修改核心逻辑** - 沙盒只负责输入和渲染
3. **跨平台兼容** - SFML 配置已处理 macOS/Windows/Linux

---

**最后更新：** 2026-03-29  
**维护者：** Project2 Team

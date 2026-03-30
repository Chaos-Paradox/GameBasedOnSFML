# Project2 测试目录

## 📁 目录结构

```
tests/
├── README.md              ← 本文件（测试总览）
├── unit/                  ← 单元测试（GTest 风格，无 SFML）
│   ├── CMakeLists.txt
│   ├── movement_pipeline_test.cpp
│   ├── hurt_dead_state_test.cpp
│   ├── attack_test.cpp
│   ├── collision_test.cpp (待修复)
│   ├── state_machine_test.cpp (待修复)
│   └── test_architecture.cpp (待修复)
└── sandbox/               ← 可视化沙盒（需要 SFML）
    ├── CMakeLists.txt
    ├── VisualSandbox.cpp  ← 主沙盒程序
    ├── README.md          ← 沙盒详细文档
    └── QUICKSTART.md      ← 快速开始指南
```

## 🧪 单元测试 (tests/unit/)

**特点：**
- 无 SFML 依赖
- 纯逻辑测试
- 命令行运行
- 适合 CI/CD

**可用测试：**

| 测试 | 状态 | 说明 |
|------|------|------|
| `movement_pipeline_test` | ✅ | 运动管线集成测试（8 个测试） |
| `hurt_dead_state_test` | ✅ | 受伤和死亡状态测试（10 个测试） |
| `attack_test` | ✅ | 攻击系统演示 |

**编译和运行：**
```bash
cd /Users/pioneer/.openclaw/workspace/SFMLGame/Project2

# 编译所有单元测试
cmake --build build --target movement_pipeline_test
cmake --build build --target hurt_dead_state_test
cmake --build build --target attack_test

# 运行
./build/tests/unit/movement_pipeline_test
./build/tests/unit/hurt_dead_state_test
./build/tests/unit/attack_test
```

## 🎮 可视化沙盒 (tests/sandbox/)

**特点：**
- 需要 SFML
- 实时图形渲染
- 玩家控制
- 适合调试和演示

**功能：**
- WASD 移动玩家（绿色方块）
- 假人靶子（红色方块，用于测试）
- 实时状态显示（控制台输出）
- 网格背景

**编译和运行：**
```bash
cd /Users/pioneer/.openclaw/workspace/SFMLGame/Project2

# 启用 SFML 并编译
cmake -S . -B build -DENABLE_SFML=ON
cmake --build build --target VisualSandbox

# 运行
./build/bin/VisualSandbox
```

**控制：**
- **WASD / 方向键** - 移动玩家
- **ESC** - 退出

## 📊 测试统计

| 类别 | 测试数 | 通过率 | 状态 |
|------|--------|--------|------|
| 单元测试 | 19 | 100% | ✅ |
| 可视化沙盒 | 1 | N/A | ✅ |

## 🔧 开发流程

### 添加新单元测试

1. 在 `tests/unit/` 创建 `新测试_test.cpp`
2. 在 `tests/unit/CMakeLists.txt` 添加：
   ```cmake
   add_executable(新测试_test 新测试_test.cpp)
   target_link_libraries(新测试_test PRIVATE ${PROJECT_NAME})
   ```
3. 编译并运行

### 添加新沙盒测试

1. 在 `tests/sandbox/` 创建 `新沙盒.cpp`
2. 在 `tests/sandbox/CMakeLists.txt` 添加：
   ```cmake
   add_executable(新沙盒 新沙盒.cpp)
   target_link_libraries(新沙盒 PRIVATE ${PROJECT_NAME} SFML::...)
   ```
3. 编译并运行

## ⚠️ 注意事项

1. **SFML 隔离** - SFML 只存在于 `tests/sandbox/`，核心逻辑保持纯净
2. **单元测试优先** - 新功能的单元测试必须先于沙盒测试完成
3. **架构验证** - 所有测试必须通过才能合并代码

---

**最后更新：** 2026-03-29  
**维护者：** Project2 Team

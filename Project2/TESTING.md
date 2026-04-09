# Project2 测试指南

## 🚀 快速开始

### 在 VSCode 中运行测试

#### 方法 1：使用任务面板（推荐）

1. 按 `Cmd+Shift+P` (macOS) 或 `Ctrl+Shift+P` (Windows/Linux)
2. 输入 `Tasks: Run Task`
3. 选择以下任务之一：
   - `Test: Run All Tests` - 运行所有测试
   - `Test: Movement Pipeline` - 运行运动管线测试
   - `Test: Hurt & Dead State` - 运行受伤/死亡状态测试

#### 方法 2：使用终端

```bash
cd /Users/pioneer/.openclaw/workspace/SFMLGame/Project2

# 编译
cmake --build build

# 运行所有测试
cd build/test && ./movement_pipeline_test && ./hurt_dead_state_test && ./attack_test
```

#### 方法 3：调试运行

1. 按 `F5` 或点击运行按钮
2. 选择调试配置：
   - `Debug - Movement Pipeline Test`
   - `Debug - Hurt Dead State Test`

---

## 📋 测试列表

| 测试文件 | 测试数 | 断言数 | 说明 |
|----------|--------|--------|------|
| `movement_pipeline_test.cpp` | 8 | 21 | 运动管线集成测试 |
| `hurt_dead_state_test.cpp` | 10 | 10 | 受伤和死亡状态测试 |
| `attack_test.cpp` | 1 | - | 攻击系统演示 |

**总计：** 19 个测试，31+ 个断言

---

## 🧪 测试详情

### Movement Pipeline Test

测试从输入到位移的完整管线：

```
InputCommand → StateMachineSystem → LocomotionSystem → MovementSystem
```

**测试场景：**
1. 初始状态验证（Idle）
2. Idle → Move 状态切换
3. Move 状态速度计算
4. Idle 状态速度归零
5. 位置积分
6. 完整管线集成
7. 停止移动
8. 多实体独立运动

### Hurt & Dead State Test

测试受伤和死亡状态的优先级：

```
状态优先级：Dead > Hurt > Attack > Move > Idle
```

**测试场景：**
1. HurtState 实例
2. DeadState 实例（终态）
3. 受伤状态切换
4. 受伤期间不能移动
5. 受伤期间不能攻击
6. 死亡状态切换
7. 死亡是终态
8. 死亡优先级最高
9. 受伤优先级高于攻击
10. 状态优先级顺序

### Attack Test

攻击系统演示（简化版）：
- 两个玩家实体
- 每帧输入 Attack 命令
- 验证状态切换

---

## ⚙️ 配置说明

### VSCode 配置

**`.vscode/tasks.json`** - 构建和测试任务
- `CMake: Build All` - 编译所有
- `Test: Run All Tests` - 运行所有测试
- `CMake: Clean Rebuild` - 清理并重新编译

**`.vscode/launch.json`** - 调试配置
- `Debug - Movement Pipeline Test`
- `Debug - Hurt Dead State Test`

### CMake 配置

**`test/CMakeLists.txt`** - 测试编译配置

```cmake
add_executable("movement_pipeline_test" "movement_pipeline_test.cpp")
target_link_libraries("movement_pipeline_test" Project2)

add_executable("hurt_dead_state_test" "hurt_dead_state_test.cpp")
target_link_libraries("hurt_dead_state_test" Project2)

add_executable("attack_test" "attack_test.cpp")
target_link_libraries("attack_test" Project2)
```

---

## 📊 测试覆盖率

| 模块 | 测试覆盖 | 状态 |
|------|----------|------|
| StateMachineSystem | ✅ | 已覆盖 |
| LocomotionSystem | ✅ | 已覆盖 |
| MovementSystem | ✅ | 已覆盖 |
| HurtState | ✅ | 已覆盖 |
| DeadState | ✅ | 已覆盖 |

---

## 🔧 故障排除

### 编译错误

**问题：** `fatal error: 'gtest/gtest.h' file not found`

**解决：** 当前测试不使用 gtest，使用自定义测试框架。确保包含正确的头文件。

### 运行错误

**问题：** `no such file or directory: ./movement_pipeline_test`

**解决：** 确保在正确的目录运行：
```bash
cd build/test
./movement_pipeline_test
```

### 测试失败

**问题：** 断言失败

**解决：** 
1. 检查测试代码逻辑
2. 检查 System 的 update 实现
3. 检查 Component 数据是否正确初始化

---

## 📝 添加新测试

1. 在 `test/` 目录创建 `新测试名_test.cpp`
2. 编写测试代码（参考现有测试）
3. 在 `test/CMakeLists.txt` 添加：
   ```cmake
   add_executable("新测试名_test" "新测试名_test.cpp")
   target_link_libraries("新测试名_test" Project2)
   ```
4. 编译并运行：
   ```bash
   cmake --build build --target 新测试名_test
   ./build/test/新测试名_test
   ```

---

## 🎯 测试最佳实践

1. **测试隔离** - 每个测试独立，不依赖其他测试
2. **测试命名** - 使用描述性名称（如 `IdleToMove_Transition`）
3. **断言明确** - 每个断言验证一个条件
4. **集成测试** - 测试完整管线而非单个 System
5. **边界测试** - 测试边界条件（如 HP=0, dt=0）

---

**最后更新：** 2026-03-29  
**维护者：** Project2 Team

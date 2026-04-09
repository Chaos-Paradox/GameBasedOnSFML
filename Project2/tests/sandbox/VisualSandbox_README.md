# VisualSandbox - 可视化调试沙盒

## 🎯 用途

实时可视化验证 Project2 ECS 核心逻辑，用于：
- 调试角色移动和状态切换
- 验证战斗系统（TODO）
- 测试掉落物渲染（TODO）
- 快速原型演示

## 🎮 当前功能

### 已完成 ✅

- [x] 玩家控制（WASD 移动）
- [x] 假人靶子（Dummy Target）
- [x] 状态切换可视化（Idle/Move）
- [x] 阵营区分（玩家=绿色，敌人=红色）
- [x] 网格背景
- [x] 控制台实时输出

### 待开发 📋

- [ ] 头顶 Debug 文字（SFML 3.0 字体 API 变化）
- [ ] 攻击动画和 Hitbox 可视化
- [ ] 受伤效果和无敌帧显示
- [ ] 鼠标点击生成假人
- [ ] 相机跟随玩家

## 🚀 快速开始

```bash
cd /Users/pioneer/.openclaw/workspace/SFMLGame/Project2

# 编译
cmake -S . -B build -DENABLE_SFML=ON
cmake --build build --target VisualSandbox

# 运行
./build/bin/VisualSandbox
```

## 🎨 视觉说明

| 颜色 | 实体 | 说明 |
|------|------|------|
| 🟢 绿色 | 玩家 | 受 WASD 控制 |
| 🔴 红色 | 假人 | 固定位置，用于测试 |
| ⬜ 网格 | 背景 | 辅助定位 |

## 📊 控制台输出

每秒输出一次玩家状态：
```
Player: IDLE | Pos: (200, 300) | HP: 100
Player: MOVE | Pos: (205, 300) | HP: 100
```

## 🔧 代码结构

```cpp
main() {
    // 1. 初始化 SFML 窗口
    // 2. 初始化 ECS 和 Component
    // 3. 创建玩家和假人
    // 4. 主循环
    //    - 处理事件
    //    - 读取输入
    //    - 更新 ECS 管线
    //    - 渲染
    // 5. 清理
}
```

## 📝 扩展指南

### 添加新实体类型

在 `createPlayer()` 和 `createDummy()` 基础上添加：
```cpp
auto createEnemy(...) { ... }
auto createDrop(...) { ... }
```

### 添加新系统测试

在主循环中添加：
```cpp
// 未来添加
CombatSystem::update(world, dt);
RenderSystem::sync(world, window);
```

## ⚠️ 已知问题

1. **字体加载** - SFML 3.0 字体 API 变化，Debug 文字暂时禁用
2. **头顶文字** - 等待 SFML 3.0 文本渲染 API 适配

---

**最后更新：** 2026-03-29  
**维护者：** Project2 Team

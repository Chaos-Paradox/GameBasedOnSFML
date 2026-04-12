# Visual Sandbox 快速开始

## 🚀 运行沙盒测试

```bash
cd <项目根目录>/Project2

# 编译（启用 SFML）
cmake -S . -B build -DENABLE_SFML=ON
cmake --build build --target VisualMovementTest

# 运行
./build/bin/VisualMovementTest
```

## 🎮 控制

- **WASD** 或 **方向键** - 移动角色
- **ESC** - 退出

## 🎨 视觉反馈

- 🔵 **蓝色** = Idle 状态
- 🟢 **绿色** = Move 状态

## ✅ 验证成功

- [x] SFML 只存在于沙盒
- [x] Project2 核心逻辑无 SFML 依赖
- [x] 运动管线正常工作
- [x] 状态切换正确（Idle ↔ Move）

---

**架构验证完成！** ✅

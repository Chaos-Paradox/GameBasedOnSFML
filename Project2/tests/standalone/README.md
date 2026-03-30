# 独立测试程序

由于 OpenClaw AI Agent 有控制权，用户无法直接按键操作。

**解决方案：**
1. 编译 VisualSandbox 可执行程序
2. 在终端直接运行：`./build/bin/VisualSandbox`
3. 使用 WASD 移动，J 键攻击

**控制：**
- WASD / 方向键 - 移动
- J 键 - 攻击
- ESC - 退出

**观察：**
- 沙包每 2 秒自动攻击一次
- 玩家 HP 100，沙包 HP 100
- 每次攻击伤害 10
- 需要 10 次攻击击杀
- 沙包死后 5 秒重生

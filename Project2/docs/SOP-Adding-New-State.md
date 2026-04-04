# SOP - 添加新状态标准操作程序

> **版本:** 1.0  
> **生效日期:** 2026-03-29  
> **适用范围:** 所有状态机相关模块开发

---

## 目的

规范添加新状态的流程，确保代码质量、测试覆盖和文档完整。

**核心原则:**
- **零修改原则** - 添加新状态不修改现有状态代码
- **统一接口** - 所有状态遵循 IState 接口
- **测试覆盖** - 每个新状态必须有单元测试
- **文档完整** - 设计文档 + 实现总结 + 改动记录

---

## 添加新状态流程

### 10 步流程

```
1. 需求分析 → 2. 设计文档 → 3. 实现状态类 → 4. 注册 StateType
     ↓
5. 添加转换规则 → 6. 单元测试 → 7. 运行测试 → 8. 实现总结
     ↓
9. 更新 CHANGELOG → 10. 代码审查
```

### 步骤详解

#### 1. 需求分析

明确新状态的：
- **行为:** 这个状态做什么？
- **进入条件:** 什么情况下进入？
- **退出条件:** 什么情况下离开？
- **优先级:** 相对于其他状态的优先级？

#### 2. 设计文档

在 `docs/journal/YYYY-MM-DD-状态名-Design.md` 创建设计文档。

#### 3. 实现状态类

创建 `include/states/新状态.h`，继承 `IState` 接口。

```cpp
class NewState : public IState {
public:
    void Enter(Entity entity) override;
    IState* Update(Entity entity, float dt) override;
    void Exit(Entity entity) override;
    const char* GetName() const override;
    StateType GetType() const override;
    
    static NewState& Instance();
};
```

#### 4. 注册状态类型

编辑 `include/states/IState.h`，在 `StateType` 枚举中添加：

```cpp
enum class StateType {
    // ... 现有状态
    NewState,  // ← 新增
};
```

#### 5. 添加转换规则

编辑 `include/systems/StateMachineSystem.h`，在 `decideState()` 中添加转换逻辑。

#### 6. 编写单元测试

创建 `test/新状态_test.cpp`，覆盖：
- 状态初始化
- Enter/Update/Exit 调用
- 状态转换
- 边界情况

#### 7. 运行所有测试

```bash
cd /Users/pioneer/.openclaw/workspace/SFMLGame/Project2
./test/run_all_tests.sh
```

#### 8. 编写实现总结

在 `docs/journal/YYYY-MM-DD-状态名-Implementation.md` 创建实现总结。

#### 9. 更新 CHANGELOG

编辑 `docs/CHANGELOG.md`，在顶部添加新条目。

#### 10. 代码审查

使用下方检查清单。

---

## 代码审查清单

### 提交前自检

**代码质量:**
- [ ] 遵循 IState 接口
- [ ] 无编译器警告
- [ ] 代码格式化
- [ ] 注释完整

**功能正确性:**
- [ ] Enter() 正确初始化
- [ ] Update() 逻辑正确
- [ ] Exit() 正确清理
- [ ] 状态转换正确

**测试:**
- [ ] 单元测试通过
- [ ] 所有现有测试通过
- [ ] 测试覆盖率 > 80%

**文档:**
- [ ] 设计文档完成
- [ ] 实现总结完成
- [ ] CHANGELOG.md 更新

---

## 文件结构规范

```
Project2/
├── include/
│   ├── states/
│   │   ├── IState.h              # 修改：添加 StateType
│   │   └── NewState.h            # 新增
│   └── systems/
│       └── StateMachineSystem.h  # 修改：添加转换规则
├── test/
│   └── new_state_test.cpp        # 新增
└── docs/
    ├── features/
    │   └── FXXX-NewState.md      # 新增
    └── journal/
        ├── YYYY-MM-DD-Design.md
        └── YYYY-MM-DD-Implementation.md
```

---

## 代码模板

### 状态类模板

```cpp
#pragma once
#include "IState.h"

class NewState : public IState {
public:
    void Enter(Entity entity) override {
        // 初始化逻辑
    }
    
    IState* Update(Entity entity, float dt) override {
        // 状态逻辑
        // 返回新状态或 nullptr（保持）
        return nullptr;
    }
    
    void Exit(Entity entity) override {
        // 清理逻辑
    }
    
    const char* GetName() const override {
        return "NewState";
    }
    
    StateType GetType() const override {
        return StateType::NewState;
    }
    
    static NewState& Instance() {
        static NewState instance;
        return instance;
    }
};
```

---

## 快速参考卡

### 检查清单

```
□ 1. 需求分析完成
□ 2. 设计文档编写
□ 3. 状态类实现（继承 IState）
□ 4. StateType 枚举注册
□ 5. StateMachineSystem 转换规则
□ 6. 单元测试编写
□ 7. 所有测试通过
□ 8. 实现总结编写
□ 9. CHANGELOG.md 更新
□ 10. 代码审查通过
```

### 文件位置速查

```
状态类：  include/states/状态名 State.h
测试：    test/状态名_test.cpp
功能文档：docs/features/FXXX-状态名.md
设计：    docs/journal/YYYY-MM-DD-Design.md
总结：    docs/journal/YYYY-MM-DD-Implementation.md
改动：    docs/CHANGELOG.md
```

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

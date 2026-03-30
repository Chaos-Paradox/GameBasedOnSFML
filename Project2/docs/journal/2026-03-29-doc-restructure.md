# 2026-03-29 - 文档结构重构

## 背景

Project2 原有的文档结构（01-Design、02-Implementation、03-Standards、04-Updates）虽然完整，但存在以下问题：

1. **文档分散** - 同一功能的设计和实现分散在不同文件夹
2. **查找困难** - 了解一个功能需要跨多个文件夹
3. **缺少功能视图** - 没有按功能模块组织的文档
4. **与 Project1 不一致** - 两个项目的文档结构不同

随着项目规模扩大，需要更清晰的文档体系来支持后续开发。

## 决策

采用统一的文档结构，与 Project1 保持一致：

```
Project2/
├── .cursorrules/
│   └── prompt.md              # AI 助手行为准则
├── docs/
│   ├── 00_ARCHITECTURE.md     # 全局架构地图
│   ├── 01_DATA_SCHEMA.md      # 数据字典
│   ├── features/              # 功能模块文档（重点）
│   │   ├── F001-StateMachine.md
│   │   ├── F103-Attack.md
│   │   ├── F104-Hurt.md
│   │   ├── F105-Death.md
│   │   └── F201-Collision.md
│   ├── journal/               # 迭代日志
│   ├── SOP-Adding-New-State.md # 标准操作程序
│   └── CHANGELOG.md           # 改动记录
└── (代码目录)
```

## 替代方案

### 方案 A：保持原有结构

**优点:**
- 无需迁移现有文档
- Design/Implementation分离清晰

**缺点:**
- 同一功能的文档分散
- 查找困难
- 与 Project1 不一致

**未选择原因:** 不利于长期维护和查找

### 方案 B：完全替换原有文档

删除所有原有文档，重新编写。

**优点:**
- 文档结构统一
- 内容可以重新组织

**缺点:**
- 丢失大量有价值的设计细节
- 重复工作

**未选择原因:** 原有文档包含详细的设计思路和实现细节，不应丢弃

### 方案 C：合并重构（最终选择）

- 保留原有文档的核心内容
- 按功能模块合并设计和实现
- 创建统一的 features 文档
- 保留 SOP 和 CHANGELOG

**优点:**
- 功能文档集中，易于查找
- 保留原有设计细节（在 journal 中）
- 与 Project1 结构一致
- 新人可以快速了解功能全貌

**选择原因:** 平衡了查找便利性和历史文档保留

## 重构过程

### 内容映射

| 原文档 | 新位置 | 说明 |
|--------|--------|------|
| `01-Design/01-Collision-Design.md` | `features/F201-Collision.md` | 合并设计+实现 |
| `01-Design/02-StateMachine-Design.md` | `features/F001-StateMachine.md` | 合并设计+实现 |
| `02-Implementation/01-Collision-Implementation.md` | `features/F201-Collision.md` | 合并到功能文档 |
| `02-Implementation/02-StateMachine-Implementation.md` | `features/F001-StateMachine.md` | 合并到功能文档 |
| `02-Implementation/03-HurtDead-Implementation.md` | `features/F104-Hurt.md` + `F105-Death.md` | 拆分为独立功能 |
| `03-Standards/SOP-Adding-New-State.md` | `SOP-Adding-New-State.md` | 保留，简化 |
| `04-Updates/CHANGELOG.md` | `CHANGELOG.md` | 保留，更新格式 |

### 新增文档

- `features/F103-Attack.md` - 攻击系统功能文档（新增）
- `features/F104-Hurt.md` - 受伤状态功能文档（新增）
- `features/F105-Death.md` - 死亡状态功能文档（新增）
- `journal/README.md` - 迭代日志索引（新增）

### 删除的文件夹

- `01-Design/` - 内容已合并到 features
- `02-Implementation/` - 内容已合并到 features
- `03-Standards/` - 内容已合并到 SOP
- `04-Updates/` - 内容已合并到 CHANGELOG 和 journal

## 结果

### 新文档结构

```
docs/
├── 00_ARCHITECTURE.md        # 架构总览（保留）
├── 01_DATA_SCHEMA.md         # 数据结构（保留）
├── features/                 # 【新增】功能模块文档
│   ├── README.md
│   ├── F001-StateMachine.md  # 状态机（合并自 Design+Implementation）
│   ├── F103-Attack.md
│   ├── F104-Hurt.md
│   ├── F105-Death.md
│   └── F201-Collision.md     # 碰撞检测（合并自 Design+Implementation）
├── journal/                  # 【新增】迭代日志
│   ├── README.md
│   └── 2026-03-29-doc-restructure.md
├── SOP-Adding-New-State.md   # 添加新状态 SOP
└── CHANGELOG.md              # 改动记录
```

### 改进点

1. **功能文档集中** - 每个功能一个文档，包含设计和实现
2. **查找快速** - 通过 features/README.md 快速定位
3. **结构统一** - 与 Project1 保持一致
4. **历史保留** - 原有设计细节保存在 journal 中

## 教训

1. **文档结构应早于代码复杂度增长** - 下次在项目初期就建立统一文档框架
2. **功能视图比阶段视图更实用** - 开发者更关心"这个功能怎么用"，而不是"这个功能的设计和实施"
3. **保留历史文档有价值** - 原有的详细设计文档仍然有参考价值，不应删除

## 相关链接

- [features/README.md](../features/README.md) - 功能模块索引
- [CHANGELOG.md](../CHANGELOG.md) - 改动记录
- [SOP-Adding-New-State.md](../SOP-Adding-New-State.md) - 添加新状态 SOP
- [Project1 文档](../../Project1/docs/) - Project1 文档结构

---

**作者:** 伶  
**日期:** 2026-03-29  
**状态:** ✅ 已完成

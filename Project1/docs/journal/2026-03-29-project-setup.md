# 2026-03-29 - Project1 初始设置

## 背景

Project1 作为 ECS 框架项目，需要独立的文档体系来支持框架开发。由于 Project1 和 Project2 是独立的项目，各自的文档应该在项目内部独立管理。

## 决策

为 Project1 创建以下文档结构：

```
Project1/
├── .cursorrules/
│   └── prompt.md              # AI 助手行为准则
├── docs/
│   ├── 00_ARCHITECTURE.md     # ECS 架构总览
│   ├── 01_DATA_SCHEMA.md      # 组件数据结构
│   ├── features/              # 功能模块文档
│   │   └── README.md
│   └── journal/               # 迭代日志
│       └── README.md
└── (代码目录)
```

## 替代方案

### 方案 A：共享文档

在项目根目录创建共享文档供所有 Project 使用。

**未选择原因：**
- Project1 是通用框架，Project2 是具体游戏逻辑
- 两者的文档受众不同
- 框架文档应独立于具体应用

### 方案 B：保持无文档

等待代码稳定后再写文档。

**未选择原因：**
- ECS 架构需要清晰的设计指导
- 文档驱动开发有助于理清思路

## 结果

- ✅ 创建了 `.cursorrules/prompt.md`
- ✅ 创建了 `docs/00_ARCHITECTURE.md` - ECS 架构设计
- ✅ 创建了 `docs/01_DATA_SCHEMA.md` - 组件数据结构
- ✅ 创建了 `docs/features/README.md` - 功能索引
- ✅ 创建了 `docs/journal/README.md` - 日志索引

## 教训

1. **框架文档应更早建立** - ECS 架构复杂，早期文档有助于保持一致性
2. **区分框架和应用** - Project1 的文档应聚焦通用框架，不涉及具体游戏逻辑

## 相关链接

- [docs/00_ARCHITECTURE.md](../00_ARCHITECTURE.md)
- [docs/01_DATA_SCHEMA.md](../01_DATA_SCHEMA.md)
- [Project2 文档](../../Project2/docs/)

---

**作者:** 伶  
**日期:** 2026-03-29  
**状态:** 已完成

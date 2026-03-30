# prompt.md - Project1 全局大脑

AI 助手在 Project1 中的行为准则和技术栈限制。

---

## 核心身份

你是 Project1（ECS 框架）的开发助手，专注于帮助用户构建基于 ECS 架构的游戏引擎基础。

## 技术栈约束

### 必须遵守

- **架构模式：** ECS（实体 - 组件 - 系统）
- **语言：** C++17 或更高
- **构建系统：** CMake 3.27+
- **图形库：** SFML 3.x（如需要渲染）
- **路径格式：** 使用 `/` 而非 `\`

### 设计原则

- **组件纯数据：** Component 只包含数据，不包含逻辑
- **系统无状态：** System 不应持有持久状态
- **实体轻量：** Entity 只是 ID 和组件容器
- **缓存友好：** 使用连续内存存储组件

## 行为准则

### 代码风格

- 使用现代 C++ 特性（auto, smart pointers, templates）
- 遵循 RAII 原则
- 命名约定：
  - 类/结构体：PascalCase
  - 函数/方法：camelCase
  - 变量：camelCase
  - 私有成员：m_ 前缀
  - 组件：XxxComponent 或 Xxx（如 Transform, Sprite）
  - 系统：XxxSystem（如 RenderSystem, PhysicsSystem）

### 架构原则

- 组件继承自统一的 Component 基类或使用 CRTP
- 系统继承自 System 基类，实现 update(dt) 方法
- 使用 World 或 Scene 管理实体和系统
- 避免系统间直接耦合，通过组件通讯

### 文档要求

- 所有新系统必须更新 `docs/features/` 对应文档
- 重大重构必须写 `docs/journal/` 记录"为什么"
- 代码注释解释"为什么"而非"是什么"

## 禁止事项

- ❌ 组件中包含虚函数（使用纯数据）
- ❌ 系统持有实体引用（使用 ID）
- ❌ 在 update 中创建/销毁实体（使用延迟队列）
- ❌ 硬编码组件类型（使用注册表）
- ❌ 忽略编译警告

## 开发流程

1. 查看 `docs/00_ARCHITECTURE.md` 理解 ECS 架构
2. 查看 `docs/01_DATA_SCHEMA.md` 了解组件数据结构
3. 查看 `docs/features/` 找到相关系统文档
4. 实现前检查 `docs/journal/` 是否有相关历史决策
5. 完成后更新对应文档

## Project1 范围

**包含：**
- ECS 核心（Entity, Component, System, World）
- 基础组件（Transform, Sprite, Collision 等）
- 基础系统（Render, Physics, Input 等）

**不包含：**
- 游戏特定逻辑（如角色状态机）→ Project2
- 网络代码 → 未来 Project
- 工具链 → tools/ 目录

## 当前项目状态

- ECS 核心：已完成
- 基础组件：进行中
- 基础系统：进行中

---

**最后更新：** 2026-03-29  
**适用范围：** Project1 开发

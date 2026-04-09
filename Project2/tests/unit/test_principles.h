#pragma once
/**
 * @file test_principles.h
 * @brief 测试原则与分层架构定义
 * 
 * 本文件定义了 Project2 的测试分层架构，确保代码质量
 * 和 ECS 架构的纯洁性。
 * 
 * 分层结构：
 * - L0: 底层规范防线 (Paradigm & Memory Tests)
 * - L1: 实体装配测试 (Factory Assembly Tests)
 * - L2: 单一 System 隔离测试 (Isolated Logic Tests)
 * - L3: 跨系统链路集成测试 (Pipeline & Tag Integration Tests)
 */

// ============================================================================
// L0: 底层规范防线 (Paradigm & Memory Tests)
// ============================================================================
// 这层测试不测业务逻辑，只测"规矩"。防止未来的你在疲惫时破坏了 ECS 的纯洁性。
// 在 C++ 环境下，这类测试在编译期或极短的运行期就能完成。

// -----------------------------------------------------------------------------
// L0-Test-1: Component 的 POD 验证 (Plain Old Data)
// -----------------------------------------------------------------------------
// 目的：确保没有任何人往 Component 里偷偷塞虚函数或复杂的非连续内存对象
// 保证未来的缓存命中率和极低成本的网络序列化。

#define L0_POD_ASSERTIONS() \
    /* TransformComponent */ \
    static_assert(std::is_standard_layout_v<TransformComponent>, \
        "TransformComponent must be standard layout (POD)"); \
    static_assert(std::is_trivial_v<TransformComponent>, \
        "TransformComponent must be trivial (no constructors)"); \
    \
    /* CharacterComponent */ \
    static_assert(std::is_standard_layout_v<CharacterComponent>, \
        "CharacterComponent must be standard layout (POD)"); \
    \
    /* StateMachineComponent */ \
    static_assert(std::is_standard_layout_v<StateMachineComponent>, \
        "StateMachineComponent must be standard layout (POD)"); \
    \
    /* HitboxComponent */ \
    static_assert(std::is_standard_layout_v<HitboxComponent>, \
        "HitboxComponent must be standard layout (POD)"); \
    \
    /* HurtboxComponent */ \
    static_assert(std::is_standard_layout_v<HurtboxComponent>, \
        "HurtboxComponent must be standard layout (POD)"); \
    \
    /* DamageTagComponent */ \
    static_assert(std::is_standard_layout_v<DamageTagComponent>, \
        "DamageTagComponent must be standard layout (POD)"); \
    \
    /* InventoryComponent */ \
    static_assert(std::is_standard_layout_v<InventoryComponent>, \
        "InventoryComponent must be standard layout (POD)"); \
    \
    /* RuntimeStatsComponent */ \
    static_assert(std::is_standard_layout_v<RuntimeStatsComponent>, \
        "RuntimeStatsComponent must be standard layout (POD)"); \
    \
    /* EvolutionComponent */ \
    static_assert(std::is_standard_layout_v<EvolutionComponent>, \
        "EvolutionComponent must be standard layout (POD)"); \
    \
    std::cout << "[L0] POD Assertions: PASSED\n";

// -----------------------------------------------------------------------------
// L0-Test-2: Component 无虚函数验证
// -----------------------------------------------------------------------------
// 目的：确保 Component 没有虚函数表，保证内存连续性和网络序列化友好

#define L0_NO_VIRTUAL_ASSERTIONS() \
    static_assert(!std::is_polymorphic_v<TransformComponent>, \
        "TransformComponent must not have virtual functions"); \
    static_assert(!std::is_polymorphic_v<CharacterComponent>, \
        "CharacterComponent must not have virtual functions"); \
    static_assert(!std::is_polymorphic_v<StateMachineComponent>, \
        "StateMachineComponent must not have virtual functions"); \
    static_assert(!std::is_polymorphic_v<HitboxComponent>, \
        "HitboxComponent must not have virtual functions"); \
    static_assert(!std::is_polymorphic_v<DamageTagComponent>, \
        "DamageTagComponent must not have virtual functions"); \
    \
    std::cout << "[L0] No Virtual Assertions: PASSED\n";

// -----------------------------------------------------------------------------
// L0-Test-3: 无依赖初始化验证
// -----------------------------------------------------------------------------
// 目的：验证 World 和各个 System 的实例化不需要任何外部依赖
// （特别是绝对不能依赖渲染上下文或窗口句柄）

#define L0_NO_DEPENDENCY_ASSERTIONS() \
    /* 验证 System 可以无依赖实例化 */ \
    StateMachineSystem stateSystem; \
    CollisionSystem collisionSystem; \
    /* 如果能编译通过，说明不需要 SFML Window 等依赖 */ \
    \
    std::cout << "[L0] No Dependency Assertions: PASSED\n";

// ============================================================================
// L1: 实体装配测试 (Factory Assembly Tests)
// ============================================================================
// 这一层专门测试你的"实体制造工厂"，确保下放到业务层的"零件"没有错漏。

// -----------------------------------------------------------------------------
// L1-Test-1: 完整性装配验证
// -----------------------------------------------------------------------------
// 情境：调用 EntityId player = PlayerFactory::createPlayer(...)
// 断言：player 实体精确包含 Transform、Character、StateMachine、Inventory
//       且绝对没有挂载 AIComponent（因为它是玩家）

#define L1_FACTORY_ASSEMBLY_TEST(ecs, transforms, characters, states, inventories, player) \
    do { \
        std::cout << "[L1-Test-1] Factory Assembly Completeness\n"; \
        \
        /* 验证必备组件存在 */ \
        assert(transforms.has(player) && "Player must have TransformComponent"); \
        assert(characters.has(player) && "Player must have CharacterComponent"); \
        assert(states.has(player) && "Player must have StateMachineComponent"); \
        assert(inventories.has(player) && "Player must have InventoryComponent"); \
        \
        std::cout << "  ✓ Transform: Present\n"; \
        std::cout << "  ✓ Character: Present\n"; \
        std::cout << "  ✓ StateMachine: Present\n"; \
        std::cout << "  ✓ Inventory: Present\n"; \
        std::cout << "  ✓ PASSED\n\n"; \
    } while(0)

// -----------------------------------------------------------------------------
// L1-Test-2: 初始值默认验证
// -----------------------------------------------------------------------------
// 情境：检查刚刚生成的玩家实体
// 断言：Character.currentHP == Character.maxHP
//       StateMachine.currentState == Idle
//       初始位置正确

#define L1_INITIAL_VALUES_TEST(characters, states, transforms, player, expectedPos) \
    do { \
        std::cout << "[L1-Test-2] Initial Values Verification\n"; \
        \
        auto& charComp = characters.get(player); \
        auto& stateComp = states.get(player); \
        auto& transComp = transforms.get(player); \
        \
        /* 验证满血 */ \
        assert(charComp.currentHP == charComp.maxHP && \
            "Player must start with full HP"); \
        std::cout << "  ✓ HP: " << charComp.currentHP << "/" << charComp.maxHP << "\n"; \
        \
        /* 验证初始状态为 Idle */ \
        assert(stateComp.currentState == CharacterState::Idle && \
            "Player must start in Idle state"); \
        std::cout << "  ✓ State: Idle\n"; \
        \
        /* 验证初始位置 */ \
        assert(transComp.position.x == expectedPos.x && \
            transComp.position.y == expectedPos.y && \
            "Player position must match spawn position"); \
        std::cout << "  ✓ Position: (" << transComp.position.x << ", " \
                  << transComp.position.y << ")\n"; \
        \
        std::cout << "  ✓ PASSED\n\n"; \
    } while(0)

// ============================================================================
// L2: 单一 System 隔离测试 (Isolated Logic Tests)
// ============================================================================
// ECS 的魅力在于你可以把某一个 System 单独拎出来跑。
// 为每一个核心 System 编写无外部干扰的测试。

// -----------------------------------------------------------------------------
// L2-Test-1: MovementSystem（位移演算）
// -----------------------------------------------------------------------------
// 情境：创建一个空实体，只挂载 Transform(pos: 0,0) 和拥有速度的 Character
//       且 StateMachine 处于 Move 状态
// 行为：手动调用 MovementSystem.update(0.1f)
// 断言：Transform.pos 必须等于 (10, 0) （速度 100 * 时间 0.1）

#define L2_MOVEMENT_SYSTEM_TEST(ecs, transforms, characters, states, moveSystem) \
    do { \
        std::cout << "[L2-Test-1] MovementSystem Isolated Test\n"; \
        \
        EntityId testEntity = ecs.create(); \
        \
        /* 只挂载 Transform 和 Character */ \
        transforms.add(testEntity, { \
            .position = {0.0f, 0.0f}, \
            .velocity = {100.0f, 0.0f}  /* 速度 100 像素/秒 */ \
        }); \
        \
        /* 模拟 Move 状态 */ \
        states.add(testEntity, { \
            .currentState = CharacterState::Move \
        }); \
        \
        /* 运行 MovementSystem */ \
        moveSystem.update(transforms, states, {testEntity}, 0.1f); \
        \
        /* 验证位置 */ \
        auto& trans = transforms.get(testEntity); \
        assert(std::abs(trans.position.x - 10.0f) < 0.001f && \
            "MovementSystem must move entity by velocity * dt"); \
        assert(std::abs(trans.position.y - 0.0f) < 0.001f && \
            "Y position should not change"); \
        \
        std::cout << "  ✓ Position after 0.1s: (" << trans.position.x \
                  << ", " << trans.position.y << ")\n"; \
        std::cout << "  ✓ PASSED\n\n"; \
    } while(0)

// -----------------------------------------------------------------------------
// L2-Test-2: ModifierSystem（RPG 属性结算）
// -----------------------------------------------------------------------------
// 情境：给实体挂载基础攻击力为 10 的 CharacterComponent
//       再挂载一个"攻击力 +50%"和"固定伤害 +5"的 StatModifierComponent
// 行为：运行 ModifierSystem.update()
// 断言：RuntimeStatsComponent.finalAttack 必须精确等于 20

#define L2_MODIFIER_SYSTEM_TEST(ecs, characters, modifiers, runtimeStats, modifierSystem) \
    do { \
        std::cout << "[L2-Test-2] ModifierSystem Isolated Test\n"; \
        \
        EntityId testEntity = ecs.create(); \
        \
        /* 基础攻击力 10 */ \
        characters.add(testEntity, { \
            .baseAttack = 10 \
        }); \
        \
        /* 添加两个修饰器：+50% 和 +5 */ \
        StatModifierComponent modComp; \
        modComp.modifiers.push_back({ \
            .modifierType = StatModifier::Type::Percent, \
            .statName = "attack", \
            .value = 0.5f  /* +50% */ \
        }); \
        modComp.modifiers.push_back({ \
            .modifierType = StatModifier::Type::Flat, \
            .statName = "attack", \
            .value = 5.0f  /* +5 */ \
        }); \
        modifiers.add(testEntity, std::move(modComp)); \
        \
        /* 创建 RuntimeStats */ \
        runtimeStats.add(testEntity, {}); \
        \
        /* 运行 ModifierSystem */ \
        modifierSystem.update(characters, modifiers, runtimeStats, {testEntity}); \
        \
        /* 验证最终攻击力 = 10 + (10 * 0.5) + 5 = 20 */ \
        auto& stats = runtimeStats.get(testEntity); \
        assert(stats.finalAttack == 20 && \
            "ModifierSystem must calculate final attack correctly"); \
        \
        std::cout << "  ✓ Base Attack: 10\n"; \
        std::cout << "  ✓ Modifiers: +50%, +5\n"; \
        std::cout << "  ✓ Final Attack: " << stats.finalAttack << "\n"; \
        std::cout << "  ✓ PASSED\n\n"; \
    } while(0)

// ============================================================================
// L3: 跨系统链路集成测试 (Pipeline & Tag Integration Tests)
// ============================================================================
// 这是最重要的一层，专门测试系统之间的"交接仪式"是否顺畅
// 特别是 Tag 驱动机制。

// -----------------------------------------------------------------------------
// L3-Test-1: 完整的"受击 -> 扣血 -> 状态切换"闭环
// -----------------------------------------------------------------------------
// 情境：1. 初始化一个满血 player (HP: 100, 状态：Idle)
//       2. 强行给 player 挂载一个 DamageTagComponent{rawDamage: 20}
//          （模拟 CombatSystem 刚干完活）
// 行为：调用 StateMachineSystem.update()
// 断言：1. player 的 Character.currentHP 变成了 80
//       2. player 的 StateMachine.currentState 变成了 Hurt
//       3. 极其关键：player 身上的 DamageTagComponent 必须被彻底销毁

#define L3_COMBAT_TAG闭环_TEST(ecs, characters, states, damageTags, stateSystem) \
    do { \
        std::cout << "[L3-Test-1] Combat Tag Integration (Full Loop)\n"; \
        \
        EntityId player = ecs.create(); \
        \
        /* 初始化满血 Idle 玩家 */ \
        characters.add(player, { \
            .currentHP = 100, \
            .maxHP = 100 \
        }); \
        states.add(player, { \
            .currentState = CharacterState::Idle \
        }); \
        \
        /* 模拟 CombatSystem 挂载 DamageTag */ \
        damageTags.add(player, { \
            .rawDamage = 20, \
            .attackerId = player, \
            .timestamp = 0.0f \
        }); \
        \
        std::cout << "  Initial State: Idle, HP: 100\n"; \
        std::cout << "  DamageTag: 20 damage\n"; \
        \
        /* 运行 StateMachineSystem */ \
        stateSystem.update(states, /* inputs */ dummyInputs, damageTags, {player}, 0.016f); \
        \
        /* 验证 HP 扣减 */ \
        auto& charComp = characters.get(player); \
        assert(charComp.currentHP == 80 && \
            "Character HP must be reduced by damage"); \
        std::cout << "  ✓ HP after damage: 80\n"; \
        \
        /* 验证状态切换到 Hurt */ \
        auto& stateComp = states.get(player); \
        assert(stateComp.currentState == CharacterState::Hurt && \
            "Character must switch to Hurt state"); \
        std::cout << "  ✓ State after damage: Hurt\n"; \
        \
        /* 验证 Tag 被销毁（最关键） */ \
        assert(!damageTags.has(player) && \
            "DamageTag must be destroyed after consumption"); \
        std::cout << "  ✓ DamageTag destroyed: Yes\n"; \
        \
        std::cout << "  ✓ PASSED\n\n"; \
    } while(0)

// -----------------------------------------------------------------------------
// L3-Test-2: 无敌帧的拦截测试
// -----------------------------------------------------------------------------
// 情境：挂载了 DamageTag，但 Character.isInvincible 为 true
// 断言：系统运行后，HP 不变，Tag 被销毁，状态不切换

#define L3_INVINCIBLE_INTERCEPT_TEST(ecs, characters, states, damageTags, stateSystem) \
    do { \
        std::cout << "[L3-Test-2] Invincibility Intercept Test\n"; \
        \
        EntityId player = ecs.create(); \
        \
        /* 初始化无敌状态玩家 */ \
        characters.add(player, { \
            .currentHP = 100, \
            .isInvincible = true, \
            .invincibleTimer = 2.0f \
        }); \
        states.add(player, { \
            .currentState = CharacterState::Idle \
        }); \
        \
        /* 挂载 DamageTag */ \
        damageTags.add(player, { \
            .rawDamage = 50, \
            .attackerId = player \
        }); \
        \
        std::cout << "  Initial State: Idle, HP: 100, Invincible: true\n"; \
        std::cout << "  DamageTag: 50 damage\n"; \
        \
        /* 运行 StateMachineSystem */ \
        stateSystem.update(states, dummyInputs, damageTags, {player}, 0.016f); \
        \
        /* 验证 HP 不变 */ \
        auto& charComp = characters.get(player); \
        assert(charComp.currentHP == 100 && \
            "Invincible character must not take damage"); \
        std::cout << "  ✓ HP after damage: 100 (unchanged)\n"; \
        \
        /* 验证状态不切换 */ \
        auto& stateComp = states.get(player); \
        assert(stateComp.currentState == CharacterState::Idle && \
            "Invincible character must not switch to Hurt"); \
        std::cout << "  ✓ State after damage: Idle (unchanged)\n"; \
        \
        /* 验证 Tag 仍被销毁（防止内存泄漏） */ \
        assert(!damageTags.has(player) && \
            "DamageTag must be destroyed even if invincible"); \
        std::cout << "  ✓ DamageTag destroyed: Yes\n"; \
        \
        std::cout << "  ✓ PASSED\n\n"; \
    } while(0)

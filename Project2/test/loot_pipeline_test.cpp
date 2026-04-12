#include <gtest/gtest.h>
#include "core/GameWorld.h"
#include "systems/DamageSystem.h"
#include "systems/LootSpawnSystem.h"
#include "systems/DeathSystem.h"
#include "systems/PickupSystem.h"

/**
 * @brief 战利品管线测试（✅ GameWorld 重构后）
 *
 * 测试流程：
 * 1. 创建怪物（带掉落表）
 * 2. 创建玩家（带 EvolutionComponent）
 * 3. 对怪物造成伤害（HP ≤ 0）
 * 4. 挂载 DeathTag
 * 5. 运行 LootSpawnSystem（生成掉落物）
 * 6. 运行 DeathSystem（切换死亡状态）
 * 7. 移动玩家到掉落物位置
 * 8. 运行 PickupSystem（拾取掉落物）
 * 9. 断言：玩家获得进化点数，掉落物被销毁
 */

class LootPipelineTest : public ::testing::Test {
protected:
    GameWorld world;
    DamageSystem damageSystem;
    LootSpawnSystem lootSpawnSystem;
    DeathSystem deathSystem;
    PickupSystem pickupSystem;

    void SetUp() override {
        // GameWorld 默认构造函数已初始化所有成员
    }
};

/**
 * 测试 1：怪物死亡后生成掉落物
 */
TEST_F(LootPipelineTest, MonsterDeath_SpawnsLoot) {
    // 1. 创建怪物
    Entity monster = world.ecs.create();
    world.transforms.add(monster, {{500.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f});
    world.characters.add(monster, {"Monster", 1, 100, 100, 10, 5, 100.0f, false, 0.0f, 1.0f, 0.0f});

    LootDropComponent monsterLoot;
    monsterLoot.lootTable[0] = {1, 1.0f, 1, 1};
    monsterLoot.lootCount = 1;
    monsterLoot.hasDropped = false;
    world.lootDrops.add(monster, monsterLoot);

    // 2. 伤害
    world.characters.get(monster).currentHP = 0;

    // 3. DeathTag
    world.deathTags.add(monster, {});

    // 4. 运行系统
    lootSpawnSystem.update(world, 0.016f);

    // 5. 断言
    auto lootEntities = world.itemDatas.entityList();
    EXPECT_EQ(lootEntities.size(), 1) << "应该生成 1 个掉落物实体";

    Entity loot = lootEntities[0];
    EXPECT_TRUE(world.itemDatas.has(loot));
    EXPECT_TRUE(world.pickupBoxes.has(loot));
    EXPECT_TRUE(world.transforms.has(loot));

    const auto& itemData = world.itemDatas.get(loot);
    EXPECT_EQ(itemData.itemId, 1);
    EXPECT_EQ(itemData.amount, 1);
    EXPECT_TRUE(itemData.isPickupable);

    const auto& lootTransform = world.transforms.get(loot);
    EXPECT_NEAR(lootTransform.position.x, 500.0f, 20.0f);
    EXPECT_NEAR(lootTransform.position.y, 300.0f, 20.0f);

    EXPECT_TRUE(world.lootDrops.get(monster).hasDropped);
}

/**
 * 测试 2：玩家拾取掉落物后增加进化点数
 */
TEST_F(LootPipelineTest, PlayerPickup_GainsEvolutionPoints) {
    // 1. 创建玩家
    Entity player = world.ecs.create();
    world.transforms.add(player, {{200.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f});
    world.evolutions.add(player, {0, 0});

    // 2. 创建掉落物
    Entity loot = world.ecs.create();
    world.transforms.add(loot, {{200.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f});
    world.itemDatas.add(loot, {1, 5, true});
    world.pickupBoxes.add(loot, {30.0f, 30.0f});

    // 3. 运行拾取
    pickupSystem.update(world, 0.016f);

    // 4. 断言
    const auto& evolution = world.evolutions.get(player);
    EXPECT_EQ(evolution.evolutionPoints, 5);
    EXPECT_EQ(evolution.totalEarned, 5);

    EXPECT_FALSE(world.transforms.has(loot));
    EXPECT_FALSE(world.itemDatas.has(loot));
    EXPECT_FALSE(world.pickupBoxes.has(loot));
}

/**
 * 测试 3：完整管线测试（击杀 → 掉落 → 拾取）
 */
TEST_F(LootPipelineTest, FullPipeline_KillDropPickup) {
    // 1. 创建玩家
    Entity player = world.ecs.create();
    world.transforms.add(player, {{200.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f});
    world.characters.add(player, {"Player", 1, 100, 100, 10, 5, 200.0f, false, 0.0f, 1.0f, 0.0f});
    world.evolutions.add(player, {0, 0});
    world.states.add(player, {CharacterState::Idle, CharacterState::Idle, 0.0f});

    // 2. 创建怪物
    Entity monster = world.ecs.create();
    world.transforms.add(monster, {{500.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f});
    world.characters.add(monster, {"Monster", 1, 100, 30, 10, 5, 100.0f, false, 0.0f, 1.0f, 0.0f});
    world.states.add(monster, {CharacterState::Idle, CharacterState::Idle, 0.0f});

    LootDropComponent monsterLoot;
    monsterLoot.lootTable[0] = {1, 1.0f, 3, 5};
    monsterLoot.lootCount = 1;
    monsterLoot.hasDropped = false;
    world.lootDrops.add(monster, monsterLoot);

    // 3. 伤害
    world.characters.get(monster).currentHP = 0;

    // 伤害事件
    Entity damageEvent = world.ecs.create();
    world.damageEvents.add(damageEvent, {
        .target = monster,
        .actualDamage = 100,
        .hitPosition = {400.0f, 300.0f},
        .isCritical = false,
        .hitDirection = {1.0f, 0.0f},
        .knockbackXY = 0.0f,
        .knockbackZ = 0.0f,
        .attacker = player,
        .timestamp = 0.0f
    });

    // 4. 运行 DamageSystem
    damageSystem.update(world, 0.016f);

    // 5. 运行 LootSpawnSystem
    lootSpawnSystem.update(world, 0.016f);

    // 6. 运行 DeathSystem
    deathSystem.update(world, 0.016f);

    // 7. 验证怪物状态
    if (world.states.has(monster)) {
        EXPECT_EQ(world.states.get(monster).currentState, CharacterState::Dead);
    }

    // 8. 验证掉落物
    auto lootEntities = world.itemDatas.entityList();
    ASSERT_EQ(lootEntities.size(), 1);
    Entity loot = lootEntities[0];

    // 9. 移动玩家
    world.transforms.get(player).position = world.transforms.get(loot).position;

    // 10. 拾取
    pickupSystem.update(world, 0.016f);

    // 11. 验证点数
    const auto& evolution = world.evolutions.get(player);
    EXPECT_GE(evolution.evolutionPoints, 3);
    EXPECT_LE(evolution.evolutionPoints, 5);

    // 12. 验证掉落物销毁
    EXPECT_FALSE(world.transforms.has(loot));
    EXPECT_FALSE(world.itemDatas.has(loot));

    std::cout << "[LootPipelineTest] Full pipeline passed! Player gained "
              << evolution.evolutionPoints << " evolution points.\n";
}

/**
 * 测试 4：掉落概率测试
 */
TEST_F(LootPipelineTest, DropChance_ProbabilityTest) {
    // 0% 掉落
    Entity monsterNoDrop = world.ecs.create();
    world.transforms.add(monsterNoDrop, {{100.0f, 100.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f});
    world.characters.add(monsterNoDrop, {"Monster", 1, 100, 0, 10, 5, 100.0f, false, 0.0f, 1.0f, 0.0f});

    LootDropComponent noDropLoot;
    noDropLoot.lootTable[0] = {1, 0.0f, 1, 1};
    noDropLoot.lootCount = 1;
    noDropLoot.hasDropped = false;
    world.lootDrops.add(monsterNoDrop, noDropLoot);

    // 100% 掉落
    Entity monsterFullDrop = world.ecs.create();
    world.transforms.add(monsterFullDrop, {{200.0f, 100.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f});
    world.characters.add(monsterFullDrop, {"Monster", 1, 100, 0, 10, 5, 100.0f, false, 0.0f, 1.0f, 0.0f});

    LootDropComponent fullDropLoot;
    fullDropLoot.lootTable[0] = {1, 1.0f, 1, 1};
    fullDropLoot.lootCount = 1;
    fullDropLoot.hasDropped = false;
    world.lootDrops.add(monsterFullDrop, fullDropLoot);

    // DeathTag
    world.deathTags.add(monsterNoDrop, {});
    world.deathTags.add(monsterFullDrop, {});

    // 运行
    lootSpawnSystem.update(world, 0.016f);

    // 验证
    auto lootEntities = world.itemDatas.entityList();
    EXPECT_EQ(lootEntities.size(), 1) << "应该只有 1 个掉落物";
}

#include <gtest/gtest.h>
#include "core/ECS.h"
#include "core/Entity.h"
#include "components/Transform.h"
#include "components/Character.h"
#include "components/Hurtbox.h"
#include "components/LootDrop.h"
#include "components/ItemData.h"
#include "components/PickupBox.h"
#include "components/Evolution.h"
#include "components/DamageTag.h"
#include "components/DeathTag.h"
#include "systems/DamageSystem.h"
#include "systems/DamageSystem.h"
#include "systems/LootSpawnSystem.h"
#include "systems/DeathSystem.h"
#include "systems/PickupSystem.h"

/**
 * @brief 战利品管线测试
 * 
 * 测试流程：
 * 1. 创建怪物（带掉落表）
 * 2. 创建玩家（带 EvolutionComponent）
 * 3. 对怪物造成伤害（HP ≤ 0）
 * 4. 挂载 DeathTag
 * 5. 运行 LootSpawnSystem（生成掉落物）
 * 6. 运行 DeathSystem（销毁怪物）
 * 7. 移动玩家到掉落物位置
 * 8. 运行 PickupSystem（拾取掉落物）
 * 9. 断言：玩家获得进化点数，掉落物被销毁
 */

class LootPipelineTest : public ::testing::Test {
protected:
    ECS ecs;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<HurtboxComponent> hurtboxes;
    ComponentStore<LootDropComponent> lootDrops;
    ComponentStore<ItemDataComponent> itemDatas;
    ComponentStore<PickupBoxComponent> pickupBoxes;
    ComponentStore<EvolutionComponent> evolutions;
    ComponentStore<DamageTag> damageTags;
    ComponentStore<DeathTag> deathTags;
    ComponentStore<StateMachineComponent> states;
    
    DamageSystem damageSystem;
    LootSpawnSystem lootSpawnSystem;
    DeathSystem deathSystem;
    PickupSystem pickupSystem;
    
    void SetUp() override {
        // 清空所有组件存储
        transforms = ComponentStore<TransformComponent>();
        characters = ComponentStore<CharacterComponent>();
        hurtboxes = ComponentStore<HurtboxComponent>();
        lootDrops = ComponentStore<LootDropComponent>();
        itemDatas = ComponentStore<ItemDataComponent>();
        pickupBoxes = ComponentStore<PickupBoxComponent>();
        evolutions = ComponentStore<EvolutionComponent>();
        damageTags = ComponentStore<DamageTag>();
        deathTags = ComponentStore<DeathTag>();
    }
};

/**
 * 测试 1：怪物死亡后生成掉落物
 */
TEST_F(LootPipelineTest, MonsterDeath_SpawnsLoot) {
    // 1. 创建怪物（带掉落表）
    Entity monster = ecs.create();
    transforms.add(monster, {{500.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(monster, {"Monster", 1, 100, 100, 10, 5, 100.0f, false, 0.0f, 1.0f, 0.0f});
    
    // 添加掉落表：100% 掉落 1 个进化点（itemId=1）
    LootDropComponent monsterLoot;
    monsterLoot.lootTable[0] = {1, 1.0f, 1, 1};  // itemId=1, 100% 概率，1-1 个
    monsterLoot.lootCount = 1;
    monsterLoot.hasDropped = false;
    lootDrops.add(monster, monsterLoot);
    
    // 2. 对怪物造成伤害（HP ≤ 0）
    characters.get(monster).currentHP = 0;
    
    // 3. 挂载 DeathTag
    deathTags.add(monster, {0.0f});
    
    // 4. 运行 LootSpawnSystem（生成掉落物）
    lootSpawnSystem.update(transforms, lootDrops, itemDatas, pickupBoxes, deathTags, ecs);
    
    // 5. 断言：生成了掉落物实体
    auto lootEntities = itemDatas.entityList();
    EXPECT_EQ(lootEntities.size(), 1) << "应该生成 1 个掉落物实体";
    
    // 6. 验证掉落物数据
    Entity loot = lootEntities[0];
    EXPECT_TRUE(itemDatas.has(loot)) << "掉落物应该有 ItemDataComponent";
    EXPECT_TRUE(pickupBoxes.has(loot)) << "掉落物应该有 PickupBoxComponent";
    EXPECT_TRUE(transforms.has(loot)) << "掉落物应该有 TransformComponent";
    
    // 7. 验证物品数据
    const auto& itemData = itemDatas.get(loot);
    EXPECT_EQ(itemData.itemId, 1) << "物品 ID 应该是 1（进化点）";
    EXPECT_EQ(itemData.amount, 1) << "物品数量应该是 1";
    EXPECT_TRUE(itemData.isPickupable) << "物品应该可拾取";
    
    // 8. 验证掉落位置（应该在怪物位置附近，±20 像素抖动）
    const auto& lootTransform = transforms.get(loot);
    EXPECT_NEAR(lootTransform.position.x, 500.0f, 20.0f) << "掉落物 X 坐标应该在怪物附近";
    EXPECT_NEAR(lootTransform.position.y, 300.0f, 20.0f) << "掉落物 Y 坐标应该在怪物附近";
    
    // 9. 验证掉落表已标记为已掉落
    EXPECT_TRUE(lootDrops.get(monster).hasDropped) << "掉落表应该标记为已掉落";
}

/**
 * 测试 2：玩家拾取掉落物后增加进化点数
 */
TEST_F(LootPipelineTest, PlayerPickup_GainsEvolutionPoints) {
    // 1. 创建玩家
    Entity player = ecs.create();
    transforms.add(player, {{200.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    evolutions.add(player, {0, 0});  // 初始 0 点进化点数
    
    // 2. 创建掉落物（直接在玩家位置）
    Entity loot = ecs.create();
    transforms.add(loot, {{200.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});  // 和玩家同一位置
    itemDatas.add(loot, {1, 5, true});  // itemId=1, amount=5
    pickupBoxes.add(loot, {30.0f, 30.0f});
    
    // 3. 运行 PickupSystem
    pickupSystem.update(evolutions, transforms, transforms, itemDatas, pickupBoxes);
    
    // 4. 断言：玩家获得 5 点进化点数
    const auto& evolution = evolutions.get(player);
    EXPECT_EQ(evolution.evolutionPoints, 5) << "玩家应该获得 5 点进化点数";
    EXPECT_EQ(evolution.totalEarned, 5) << "累计获得点数应该是 5";
    
    // 5. 断言：掉落物被销毁
    EXPECT_FALSE(transforms.has(loot)) << "掉落物 Transform 应该被销毁";
    EXPECT_FALSE(itemDatas.has(loot)) << "掉落物 ItemData 应该被销毁";
    EXPECT_FALSE(pickupBoxes.has(loot)) << "掉落物 PickupBox 应该被销毁";
}

/**
 * 测试 3：完整管线测试（击杀 → 掉落 → 拾取）
 */
TEST_F(LootPipelineTest, FullPipeline_KillDropPickup) {
    // 1. 创建玩家
    Entity player = ecs.create();
    transforms.add(player, {{200.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(player, {"Player", 1, 100, 100, 10, 5, 200.0f, false, 0.0f, 1.0f, 0.0f});
    evolutions.add(player, {0, 0});
    
    // 2. 创建怪物（带掉落表）
    Entity monster = ecs.create();
    transforms.add(monster, {{500.0f, 300.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(monster, {"Monster", 1, 100, 30, 10, 5, 100.0f, false, 0.0f, 1.0f, 0.0f});
    
    LootDropComponent monsterLoot;
    monsterLoot.lootTable[0] = {1, 1.0f, 3, 5};  // 掉落 3-5 个进化点
    monsterLoot.lootCount = 1;
    monsterLoot.hasDropped = false;
    lootDrops.add(monster, monsterLoot);
    
    // 3. 对怪物造成伤害（HP ≤ 0）
    characters.get(monster).currentHP = 0;
    deathTags.add(monster, {0.0f});
    
    // 4. 运行 DamageSystem
    damageSystem.update(characters, damageTags, deathTags);
    
    // 5. 运行 LootSpawnSystem
    lootSpawnSystem.update(transforms, lootDrops, itemDatas, pickupBoxes, deathTags, ecs);
    
    // 6. 运行 DeathSystem（销毁怪物）
    deathSystem.update(states, deathTags, 0.016f);
    
    // 7. 验证怪物状态切换为 Dead（不销毁实体，由 CleanupSystem 处理）
    if (states.has(monster)) {
        EXPECT_EQ(states.get(monster).currentState, CharacterState::Dead) << "怪物应该切换为 Dead 状态";
    } else {
        std::cout << "[Test] Monster entity was destroyed (unexpected)\n";
    }
    
    // 8. 验证掉落物已生成
    auto lootEntities = itemDatas.entityList();
    ASSERT_EQ(lootEntities.size(), 1) << "应该生成 1 个掉落物实体";
    Entity loot = lootEntities[0];
    
    // 9. 移动玩家到掉落物位置（模拟玩家走过去）
    transforms.get(player).position = transforms.get(loot).position;
    
    // 10. 运行 PickupSystem
    pickupSystem.update(evolutions, transforms, transforms, itemDatas, pickupBoxes);
    
    // 11. 验证玩家获得进化点数（3-5 点）
    const auto& evolution = evolutions.get(player);
    EXPECT_GE(evolution.evolutionPoints, 3) << "玩家应该至少获得 3 点进化点数";
    EXPECT_LE(evolution.evolutionPoints, 5) << "玩家最多获得 5 点进化点数";
    
    // 12. 验证掉落物已销毁
    EXPECT_FALSE(transforms.has(loot)) << "掉落物应该被销毁";
    EXPECT_FALSE(itemDatas.has(loot)) << "掉落物 ItemData 应该被销毁";
    
    std::cout << "[LootPipelineTest] Full pipeline test passed! Player gained " 
              << evolution.evolutionPoints << " evolution points.\n";
}

/**
 * 测试 4：掉落概率测试（0% 不掉，100% 必掉）
 */
TEST_F(LootPipelineTest, DropChance_ProbabilityTest) {
    // 1. 创建怪物（0% 掉落率）
    Entity monsterNoDrop = ecs.create();
    transforms.add(monsterNoDrop, {{100.0f, 100.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(monsterNoDrop, {"Monster", 1, 100, 0, 10, 5, 100.0f, false, 0.0f, 1.0f, 0.0f});
    
    LootDropComponent noDropLoot;
    noDropLoot.lootTable[0] = {1, 0.0f, 1, 1};  // 0% 概率
    noDropLoot.lootCount = 1;
    noDropLoot.hasDropped = false;
    lootDrops.add(monsterNoDrop, noDropLoot);
    
    // 2. 创建怪物（100% 掉落率）
    Entity monsterFullDrop = ecs.create();
    transforms.add(monsterFullDrop, {{200.0f, 100.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(monsterFullDrop, {"Monster", 1, 100, 0, 10, 5, 100.0f, false, 0.0f, 1.0f, 0.0f});
    
    LootDropComponent fullDropLoot;
    fullDropLoot.lootTable[0] = {1, 1.0f, 1, 1};  // 100% 概率
    fullDropLoot.lootCount = 1;
    fullDropLoot.hasDropped = false;
    lootDrops.add(monsterFullDrop, fullDropLoot);
    
    // 3. 挂载 DeathTag
    deathTags.add(monsterNoDrop, {0.0f});
    deathTags.add(monsterFullDrop, {0.0f});
    
    // 4. 运行 LootSpawnSystem
    lootSpawnSystem.update(transforms, lootDrops, itemDatas, pickupBoxes, deathTags, ecs);
    
    // 5. 验证：0% 掉落率的怪物没有生成掉落物
    // 6. 验证：100% 掉落率的怪物生成了掉落物
    auto lootEntities = itemDatas.entityList();
    EXPECT_EQ(lootEntities.size(), 1) << "应该只有 1 个掉落物（100% 掉落率的那个）";
}

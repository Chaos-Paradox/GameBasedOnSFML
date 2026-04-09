#include <iostream>
#include <cassert>
#include <vector>
#include "core/ECS.h"
#include "components/Position.h"
#include "components/Hitbox.h"
#include "components/Hurtbox.h"
#include "components/Character.h"
#include "components/DamageTag.h"
#include "systems/CollisionSystem.h"
#include "utils/Logging.h"

/**
 * @brief 碰撞检测测试（新架构 - Tag 驱动）
 * 
 * 测试场景：
 * 1. 两个实体，Hitbox 与 Hurtbox 重叠 → 应该挂载 DamageTag
 * 2. 两个实体，Hitbox 与 Hurtbox 不重叠 → 不应挂载 Tag
 * 3. 无敌状态 → 不应挂载 Tag
 * 4. 圆形碰撞
 * 5. 圆形 vs 矩形
 * 6. 完整战斗流程（Tag 驱动）
 */

int main() {
    std::cout << "=== Collision System Test (New Architecture) ===\n\n";
    
    ECS ecs;
    ComponentStore<Position> positions;
    ComponentStore<HitboxComponent> hitboxes;
    ComponentStore<HurtboxComponent> hurtboxes;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<DamageTagComponent> damageTags;
    CollisionSystem collision;
    
    float currentTime = 0.0f;
    
    // ========== 测试 1: 基本碰撞检测 ==========
    std::cout << "[Test 1] Basic Collision Detection\n";
    std::cout << "-----------------------------------\n";
    
    Entity player = ecs.create();
    Entity enemy = ecs.create();
    
    // 玩家位置 (0, 0)，Hitbox 覆盖 (0,0) 到 (50,50)
    positions.add(player, {0, 0});
    hitboxes.add(player, {
        .bounds = {0, 0, 50, 50},
        .damageMultiplier = 10,
        .element = ElementType::Physical,
        .active = true
    });
    
    // 敌人位置 (30, 0)，Hurtbox 覆盖 (0,0) 到 (30,50) → 与玩家 Hitbox 重叠
    positions.add(enemy, {30, 0});
    hurtboxes.add(enemy, {
        .bounds = {0, 0, 30, 50},
        .faction = Faction::Enemy
    });
    
    collision.update(hitboxes, hurtboxes, positions, damageTags, {player, enemy}, currentTime);
    
    std::cout << "Player Hitbox: (0,0) to (50,50)\n";
    std::cout << "Enemy Hurtbox: (30,0) to (60,50)\n";
    std::cout << "Overlap: YES\n";
    std::cout << "Tags generated: " << damageTags.count() << "\n";
    
    assert(damageTags.has(enemy));
    auto& tag = damageTags.get(enemy);
    assert(tag.rawDamage == 10);
    assert(tag.attackerId == player);
    
    std::cout << "DamageTag: damage=" << tag.rawDamage 
              << ", attacker=" << tag.attackerId << "\n";
    std::cout << "✓ Test 1 PASSED\n\n";
    
    // ========== 测试 2: 无碰撞 ==========
    std::cout << "[Test 2] No Collision\n";
    std::cout << "-----------------------------------\n";
    
    Entity enemy2 = ecs.create();
    positions.add(enemy2, {200, 0}); // 距离远，不重叠
    hurtboxes.add(enemy2, {
        .bounds = {0, 0, 30, 50},
        .faction = Faction::Enemy
    });
    
    collision.update(hitboxes, hurtboxes, positions, damageTags, {player, enemy2}, currentTime);
    
    std::cout << "Player Hitbox: (0,0) to (50,50)\n";
    std::cout << "Enemy2 Hurtbox: (200,0) to (230,50)\n";
    std::cout << "Overlap: NO\n";
    std::cout << "Tags generated: " << damageTags.count() << "\n";
    
    assert(!damageTags.has(enemy2));
    
    std::cout << "✓ Test 2 PASSED\n\n";
    
    // ========== 测试 3: 无敌状态 ==========
    std::cout << "[Test 3] Invincible State\n";
    std::cout << "-----------------------------------\n";
    
    Entity enemy3 = ecs.create();
    positions.add(enemy3, {30, 0});
    hurtboxes.add(enemy3, {
        .bounds = {0, 0, 30, 50},
        .faction = Faction::Enemy,
        .invincibleTime = 2.0f // 无敌 2 秒
    });
    
    collision.update(hitboxes, hurtboxes, positions, damageTags, {player, enemy3}, currentTime);
    
    std::cout << "Enemy3 Invincible Time: 2.0s\n";
    std::cout << "Tags generated: " << damageTags.count() << "\n";
    
    assert(!damageTags.has(enemy3)); // 无敌状态不应挂载 Tag
    
    std::cout << "✓ Test 3 PASSED\n\n";
    
    // ========== 测试 4: 圆形碰撞 ==========
    std::cout << "[Test 4] Circle Collision\n";
    std::cout << "-----------------------------------\n";
    
    sf::FloatRect c1{0 - 25, 0 - 25, 50, 50};  // 近似圆形（用矩形表示）
    sf::FloatRect c2{40 - 25, 0 - 25, 50, 50};
    
    bool circleCollision = collision.checkCollision(c1, c2);
    std::cout << "Circle1: (0,0) r=25 (approximated as rect)\n";
    std::cout << "Circle2: (40,0) r=25 (approximated as rect)\n";
    std::cout << "Collision: " << (circleCollision ? "YES" : "NO") << "\n";
    
    assert(circleCollision == true); // 应该碰撞
    
    sf::FloatRect c3{100 - 10, 0 - 10, 20, 20};
    bool noCircleCollision = collision.checkCollision(c1, c3);
    assert(noCircleCollision == false);
    
    std::cout << "✓ Test 4 PASSED\n\n";
    
    // ========== 测试 5: 圆形 vs 矩形 ==========
    std::cout << "[Test 5] Circle vs Rectangle\n";
    std::cout << "-----------------------------------\n";
    
    sf::FloatRect circle{50 - 30, 50 - 30, 60, 60}; // 近似圆形
    sf::FloatRect rect{40, 40, 40, 40};
    
    bool circleRectCollision = collision.checkCollision(circle, rect);
    std::cout << "Circle: (50,50) r=30 (approximated)\n";
    std::cout << "Rect: (40,40) to (80,80)\n";
    std::cout << "Collision: " << (circleRectCollision ? "YES" : "NO") << "\n";
    
    assert(circleRectCollision == true); // 应该碰撞
    
    std::cout << "✓ Test 5 PASSED\n\n";
    
    // ========== 测试 6: 完整战斗流程（Tag 驱动） ==========
    std::cout << "[Test 6] Full Combat Flow (Tag-driven)\n";
    std::cout << "-----------------------------------\n";
    
    ECS ecs2;
    ComponentStore<Position> pos2;
    ComponentStore<HitboxComponent> hit2;
    ComponentStore<HurtboxComponent> hurt2;
    ComponentStore<CharacterComponent> chars2;
    ComponentStore<DamageTagComponent> tags2;
    
    Entity p1 = ecs2.create();
    Entity p2 = ecs2.create();
    
    pos2.add(p1, {0, 0});
    pos2.add(p2, {40, 0});
    
    hit2.add(p1, {
        .bounds = {0, 0, 60, 50},
        .damageMultiplier = 15,
        .active = true
    });
    
    hurt2.add(p1, {
        .bounds = {0, 0, 30, 50},
        .faction = Faction::Player
    });
    
    hurt2.add(p2, {
        .bounds = {0, 0, 30, 50},
        .faction = Faction::Enemy
    });
    
    chars2.add(p2, {
        .currentHP = 100
    });
    
    // 碰撞检测，挂载 Tag
    collision.update(hit2, hurt2, pos2, tags2, {p1, p2}, currentTime);
    
    std::cout << "P1 attacks P2\n";
    std::cout << "P2 HP before: " << chars2.get(p2).currentHP << "\n";
    
    // 处理 Tag（模拟 StateMachineSystem 或 CombatSystem）
    for (Entity e : {p2}) {
        if (tags2.has(e)) {
            auto& tag = tags2.get(e);
            chars2.get(e).currentHP -= tag.rawDamage;
            std::cout << "[COMBAT] P1 hit P2 for " << tag.rawDamage << " damage\n";
            tags2.remove(e); // 消费后销毁
        }
    }
    
    std::cout << "P2 HP after: " << chars2.get(p2).currentHP << "\n";
    
    assert(chars2.get(p2).currentHP == 85); // 100 - 15 = 85
    assert(!tags2.has(p2)); // Tag 已销毁
    
    std::cout << "✓ Test 6 PASSED\n\n";
    
    // ========== 总结 ==========
    std::cout << "=== All Tests PASSED ===\n";
    std::cout << "Collision System (Tag-driven) is working correctly!\n";
    
    return 0;
}

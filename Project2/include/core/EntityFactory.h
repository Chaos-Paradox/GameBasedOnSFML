#pragma once

/**
 * @brief 数据驱动实体工厂
 *
 * 从 JSON 预制文件读取参数，替代硬编码的 createPlayer/createDummy/placeBomb。
 *
 * 安全读取策略：所有 JSON 字段提取必须使用 .value(key, default)，
 * 禁止直接用 json["key"] 避免运行时崩溃。
 *
 * 用法：
 *   EntityFactory factory;
 *   factory.loadPrefabs("data/prefabs.json");
 *   auto player = factory.spawnPlayer(world, 500, 600);
 *   auto dummy  = factory.spawnDummy(world, 1000, 600);
 *   auto bomb   = factory.spawnThrowableBomb(world, player, inputCmd);
 */

#include "core/ECS.h"
#include "core/GameWorld.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

class EntityFactory {
public:
    /**
     * @brief 从 JSON 文件加载所有预制蓝图
     * @param filepath JSON 文件路径（相对于运行 CWD）
     */
    void loadPrefabs(const std::string& filepath) {
        // 立即输出启动标记（不管成功还是失败）
        std::cerr << "[EntityFactory] === loadPrefabs called with: " << filepath << " ===\n" << std::flush;
        std::cerr << "[EntityFactory] Current working directory: " << std::filesystem::current_path() << "\n" << std::flush;
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[EntityFactory] ❌ ERROR: Cannot open prefabs file: " << filepath << "\n" << std::flush;
            std::cerr << "[EntityFactory] Try using absolute path or copy data/ to build directory\n" << std::flush;
            return;
        }

        try {
            file >> m_prefabs;
            std::cerr << "[EntityFactory] ✅ Loaded prefabs from " << filepath << "\n";
            std::cerr << "[EntityFactory] Available prefab names: ";
            for (auto it = m_prefabs.begin(); it != m_prefabs.end(); ++it) {
                std::cerr << it.key() << " ";
            }
            std::cerr << "\n";
            
            // 详细输出每个预制体的关键参数
            for (auto it = m_prefabs.begin(); it != m_prefabs.end(); ++it) {
                std::cerr << "[EntityFactory] Prefab '" << it.key() << "': ";
                if (it.value().contains("character")) {
                    std::cerr << "HP=" << it.value()["character"].value("maxHP", 0) << " ";
                }
                if (it.value().contains("dash")) {
                    std::cerr << "dashSpeed=" << it.value()["dash"].value("dashSpeed", 0.0f) << " ";
                }
                if (it.value().contains("bomb")) {
                    std::cerr << "fuse=" << it.value()["bomb"].value("fuseTimer", 0.0f) << " ";
                }
                std::cerr << "\n";
            }
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "[EntityFactory] ❌ ERROR: JSON parse failed: " << e.what() << "\n";
        }
    }

    /**
     * @brief 生成玩家实体
     */
    Entity spawnPlayer(GameWorld& world, float x, float y,
                       const std::string& prefabName = "Player") {
        std::cerr << "[EntityFactory] === spawnPlayer called: " << prefabName << " at (" << x << ", " << y << ") ===\n" << std::flush;
        auto entity = world.ecs.create();

        const auto& prefab = m_prefabs[prefabName];
        std::cerr << "[EntityFactory] prefab data loaded, has " << prefab.size() << " sections\n" << std::flush;

        // --- StateMachine ---
        world.states.add(entity, {CharacterState::Idle, CharacterState::Idle, 0.0f});

        // --- Transform ---
        world.transforms.add(entity, {
            .position = {x, y},
            .scale = {1.0f, 1.0f},
            .rotation = 0.0f,
            .velocity = {0.0f, 0.0f},
            .facingX = 1.0f,
            .facingY = 0.0f
        });

        // --- Character ---
        const auto& chr = prefab["character"];
        std::string name = chr.value("name", "Player");
        int level = chr.value("level", 1);
        int maxHP = chr.value("maxHP", 100);
        int baseAttack = chr.value("baseAttack", 10);
        int baseDefense = chr.value("baseDefense", 5);
        float baseMoveSpeed = chr.value("baseMoveSpeed", 200.0f);
        world.characters.add(entity, {
            name, level, maxHP, maxHP, baseAttack, baseDefense,
            baseMoveSpeed
        });

        // --- InputCommand ---
        world.inputs.add(entity, {{0.0f, 0.0f}, ActionIntent::None, 0.0f});

        // --- Hurtbox ---
        const auto& hb = prefab["hurtbox"];
        float hurtRadius = hb.value("radius", 18.0f);
        float hurtHeight = hb.value("height", 45.0f);
        float zOffset = hb.value("zOffset", 0.0f);
        int hurtLayer = hb.value("layer", 1);
        std::string factionStr = hb.value("faction", std::string("Player"));
        Faction faction = (factionStr == "Enemy") ? Faction::Enemy : Faction::Player;
        world.hurtboxes.add(entity, {
            .radius = hurtRadius, .height = hurtHeight, .zOffset = zOffset,
            .offset = {0.0f, 0.0f}, .faction = faction,
            .layer = hurtLayer
        });

        // --- Evolution ---
        world.evolutions.add(entity, {0, 0});

        // --- Dash ---
        const auto& dash = prefab["dash"];
        float dashSpeed = dash.value("dashSpeed", 2000.0f);
        float dashDuration = dash.value("dashDuration", 0.1f);
        float iframeDuration = dash.value("iframeDuration", 0.1f);
        float recoveryDuration = dash.value("recoveryDuration", 0.0f);
        int maxCharges = dash.value("maxCharges", 2);
        float rechargeCooldown = dash.value("rechargeCooldown", 1.0f);
        world.dashes.add(entity, {
            .dashSpeed = dashSpeed, .dashDuration = dashDuration,
            .iframeDuration = iframeDuration, .recoveryDuration = recoveryDuration,
            .maxCharges = maxCharges, .currentCharges = maxCharges,
            .rechargeCooldown = rechargeCooldown,
            .dashTimer = 0.0f, .iframeTimer = 0.0f,
            .recoveryTimer = 0.0f, .dashDir = {1.0f, 0.0f},
            .isInvincible = false,
            .rechargeTimer = 0.0f
        });

        // --- Magnet ---
        const auto& mag = prefab["magnet"];
        float magnetRadius = mag.value("magnetRadius", 150.0f);
        float magnetSpeed = mag.value("magnetSpeed", 400.0f);
        world.magnets.add(entity, {
            .magnetRadius = magnetRadius, .magnetSpeed = magnetSpeed
        });

        // --- ZTransform ---
        const auto& zt = prefab["zTransform"];
        float zGravity = zt.value("gravity", -2000.0f);
        float zHeight = zt.value("height", 40.0f);
        world.zTransforms.add(entity, {
            .z = 0.0f, .vz = 0.0f, .gravity = zGravity, .height = zHeight
        });

        // --- Collider ---
        const auto& col = prefab["collider"];
        float colRadius = col.value("radius", 20.0f);
        world.colliders.add(entity, {
            .radius = colRadius, .isStatic = false
        });

        // --- Momentum ---
        const auto& mom = prefab["momentum"];
        float momMass = mom.value("mass", 100.0f);
        world.momentums.add(entity, {
            .mass = momMass, .velocity = {0.0f, 0.0f}, .useCCD = false
        });

        return entity;
    }

    /**
     * @brief 生成木桩实体
     */
    Entity spawnDummy(GameWorld& world, float x, float y,
                      const std::string& prefabName = "Dummy") {
        auto entity = world.ecs.create();

        const auto& prefab = m_prefabs[prefabName];

        // --- StateMachine ---
        world.states.add(entity, {CharacterState::Idle, CharacterState::Idle, 0.0f});

        // --- Transform ---
        world.transforms.add(entity, {
            .position = {x, y},
            .scale = {1.0f, 1.0f},
            .rotation = 0.0f,
            .velocity = {0.0f, 0.0f},
            .facingX = -1.0f,
            .facingY = 0.0f
        });

        // --- Character ---
        const auto& chr = prefab["character"];
        std::string name = chr.value("name", "Dummy");
        int level = chr.value("level", 1);
        int maxHP = chr.value("maxHP", 100);
        int baseAttack = chr.value("baseAttack", 10);
        int baseDefense = chr.value("baseDefense", 0);
        float baseMoveSpeed = chr.value("baseMoveSpeed", 0.0f);
        world.characters.add(entity, {
            name, level, maxHP, maxHP, baseAttack, baseDefense,
            baseMoveSpeed
        });

        // --- InputCommand（默认无输入，确保 LocomotionSystem 能清零速度） ---
        world.inputs.add(entity, {{0.0f, 0.0f}, ActionIntent::None, 0.0f});

        // --- Hurtbox ---
        const auto& hb = prefab["hurtbox"];
        float hurtRadius = hb.value("radius", 18.0f);
        float hurtHeight = hb.value("height", 45.0f);
        float zOffset = hb.value("zOffset", 0.0f);
        int hurtLayer = hb.value("layer", 2);
        world.hurtboxes.add(entity, {
            .radius = hurtRadius, .height = hurtHeight, .zOffset = zOffset,
            .offset = {0.0f, 0.0f}, .faction = Faction::Enemy,
            .layer = hurtLayer
        });

        // --- LootDrop ---
        const auto& loot = prefab["lootDrop"];
        if (loot.contains("entries") && loot["entries"].is_array()) {
            LootDropComponent lootComp;
            lootComp.lootCount = 0;
            for (const auto& entry : loot["entries"]) {
                if (lootComp.lootCount >= LootDropComponent::MAX_LOOT_ENTRIES) break;
                auto& e = lootComp.lootTable[lootComp.lootCount];
                e.itemId = entry.value("itemId", 0u);
                e.dropChance = entry.value("dropChance", 1.0f);
                e.minCount = entry.value("minCount", 1);
                e.maxCount = entry.value("maxCount", 1);
                e.magnetRadius = entry.value("magnetRadius", 0.0f);
                e.magnetSpeed = entry.value("magnetSpeed", 400.0f);
                lootComp.lootCount++;
            }
            world.lootDrops.add(entity, lootComp);
        }

        // --- Collider ---
        const auto& col = prefab["collider"];
        float colRadius = col.value("radius", 20.0f);
        world.colliders.add(entity, {
            .radius = colRadius, .isStatic = false
        });

        // --- Momentum (Dummy 默认质量) ---
        const auto& mom = prefab.contains("momentum") ? prefab["momentum"] : nlohmann::json{};
        float momMass = mom.value("mass", 100.0f);
        world.momentums.add(entity, {
            .mass = momMass, .velocity = {0.0f, 0.0f}, .useCCD = false
        });

        // --- ZTransform ---
        const auto& zt = prefab["zTransform"];
        float zGravity = zt.value("gravity", -2000.0f);
        float zHeight = zt.value("height", 40.0f);
        world.zTransforms.add(entity, {
            .z = 0.0f, .vz = 0.0f, .gravity = zGravity, .height = zHeight
        });

        return entity;
    }

    /**
     * @brief 生成可投掷炸弹实体
     */
    Entity spawnThrowableBomb(GameWorld& world, Entity owner,
                              const InputCommand& input,
                              const std::string& prefabName = "StandardBomb") {
        Entity bomb = world.ecs.create();
        const auto& prefab = m_prefabs[prefabName];

        // 计算炸弹朝向和出生位置
        const auto& ownerTrans = world.transforms.get(owner);
        float fx = ownerTrans.facingX, fy = ownerTrans.facingY;
        if (fx == 0.0f && fy == 0.0f) {
            if (input.moveDir.x != 0.0f || input.moveDir.y != 0.0f) {
                float len = std::sqrt(input.moveDir.x * input.moveDir.x +
                                      input.moveDir.y * input.moveDir.y);
                fx = input.moveDir.x / len;
                fy = input.moveDir.y / len;
            } else {
                fx = 1.0f; fy = 0.0f;
            }
        }

        const auto& bombCfg = prefab["bomb"];
        float spawnOffset = bombCfg.value("spawnOffset", 35.0f);
        float fuseTimer = bombCfg.value("fuseTimer", 3.0f);

        float spawnX = ownerTrans.position.x + fx * spawnOffset;
        float spawnY = ownerTrans.position.y + fy * spawnOffset;

        // --- Transform ---
        // 炸弹继承投掷者的初速度（Dash 时丢出炸弹会跟着飞出去）
        world.transforms.add(bomb, {
            .position = {spawnX, spawnY},
            .scale = {1.0f, 1.0f},
            .rotation = 0.0f,
            .velocity = ownerTrans.velocity,
            .facingX = fx,
            .facingY = fy
        });

        // --- Bomb ---
        world.bombs.add(bomb, {
            .fuseTimer = fuseTimer
        });

        // --- ZTransform ---
        // 炸弹直接出现在面前地面，不做抛掷动画
        // 避免空中弹跳阶段 Z 高度不稳定导致碰撞检测异常
        const auto& zt = prefab["zTransform"];
        float zGravity = zt.value("gravity", -1500.0f);
        float zHeight = zt.value("height", 30.0f);
        
        world.zTransforms.add(bomb, {
            .z = 0.0f, .vz = 0.0f, .gravity = zGravity, .height = zHeight
        });

        // --- Collider ---
        const auto& col = prefab["collider"];
        float colRadius = col.value("radius", 12.0f);
        world.colliders.add(bomb, {
            .radius = colRadius, .isStatic = false
        });

        // --- Momentum ---
        const auto& mom = prefab["momentum"];
        float momMass = mom.value("mass", 10.0f);
        bool useCCD = mom.value("useCCD", false);
        world.momentums.add(bomb, {
            .mass = momMass, .velocity = ownerTrans.velocity, .useCCD = useCCD
        });

        // --- Throwable ---
        world.throwables.add(bomb, {
            .owner = owner,
            .graceTimer = 0.15f
        });

        return bomb;
    }

    /**
     * @brief 为炸弹生成爆炸 AOE 实体（供 BombSystem 调用）
     *
     * 爆炸参数也来自预制文件，保持数据驱动一致性。
     */
    Entity spawnExplosion(GameWorld& world, float x, float y,
                          Entity sourceBomb,
                          const std::string& prefabName = "StandardBomb") {
        Entity explosion = world.ecs.create();
        const auto& prefab = m_prefabs[prefabName];

        const auto& exp = prefab["explosion"];
        float expRadius = exp.value("radius", 150.0f);
        int expDamageMult = exp.value("damageMultiplier", 100);
        float expKnockbackForce = exp.value("knockbackForce", 500.0f);
        float expKnockbackXY = exp.value("knockbackXY", 2000.0f);
        float expKnockbackZ = exp.value("knockbackZ", 800.0f);
        float expHeight = exp.value("height", 500.0f);
        float expLifetime = prefab.value("explosionLifetime", 0.1f);

        // --- Transform ---
        world.transforms.add(explosion, {
            .position = {x, y},
            .scale = {1.0f, 1.0f},
            .rotation = 0.0f,
            .velocity = {0.0f, 0.0f},
            .facingX = 1.0f,
            .facingY = 0.0f
        });

        // --- Hitbox ---
        world.hitboxes.add(explosion, {
            .radius = expRadius,
            .offset = {0.0f, 0.0f},
            .damageMultiplier = expDamageMult,
            .element = ElementType::Fire,
            .knockbackForce = expKnockbackForce,
            .sourceEntity = sourceBomb,
            .knockbackXY = expKnockbackXY,
            .knockbackZ = expKnockbackZ,
            .active = true
        });

        // --- ZTransform ---
        world.zTransforms.add(explosion, {
            .z = 0.0f, .vz = 0.0f, .gravity = 0.0f, .height = expHeight
        });

        // --- Lifetime ---
        world.lifetimes.add(explosion, {
            .timeLeft = expLifetime
        });

        return explosion;
    }

private:
    nlohmann::json m_prefabs;
};

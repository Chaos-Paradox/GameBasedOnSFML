#pragma once
#include "IState.h"
#include "../components/InputCommand.h"
#include "../components/Position.h"
#include "../components/Hitbox.h"

/**
 * @brief 攻击状态（Attack State）
 * 
 * 播放攻击动画，激活 Hitbox，不可移动。
 * 
 * 转换规则：
 * - 攻击动画完成 → IdleState
 * - 受到伤害 → HurtState（可选，优先度更高）
 * - 攻击期间：不可移动，不可切换其他状态
 * 
 * 设计要点：
 * - 进入时激活 Hitbox
 * - 退出时停用 Hitbox
 * - 攻击期间锁定移动
 * - 可以连击（扩展）
 */
class AttackState : public IState {
public:
    AttackState() = default;
    
    void Enter(Entity entity) override {
        attackTimer = 0.0f;
        hitActivated = false;
        
        // 进入攻击状态
        // 激活 Hitbox
        // 播放攻击动画
        // std::cout << "[State] Enter Attack\n";
    }
    
    IState* Update(Entity entity, float dt) override {
        // 攻击逻辑
        attackTimer += dt;
        
        // 在攻击帧激活 Hitbox（简化：全程激活）
        if (attackTimer >= hitActivateTime && !hitActivated) {
            hitActivated = true;
            // 激活 Hitbox 组件
            // hitboxes.get(entity).active = true;
        }
        
        // 攻击完成
        if (attackTimer >= attackDuration) {
            // 停用 Hitbox
            // hitboxes.get(entity).active = false;
            return &IdleState::Instance(); // 返回 idle
        }
        
        return nullptr; // 保持攻击状态
    }
    
    void Exit(Entity entity) override {
        // 退出攻击状态
        // 停用 Hitbox
        // std::cout << "[State] Exit Attack\n";
    }
    
    const char* GetName() const override {
        return "Attack";
    }
    
    StateType GetType() const override {
        return StateType::Attack;
    }
    
    /**
     * @brief 静态实例（单例模式）
     */
    static AttackState& Instance() {
        static AttackState instance;
        return instance;
    }
    
private:
    float attackTimer = 0.0f;         ///< 攻击计时器
    float attackDuration = 0.3f;      ///< 攻击持续时间（秒）
    float hitActivateTime = 0.1f;     ///< Hitbox 激活时间（攻击前摇）
    bool hitActivated = false;        ///< Hitbox 是否已激活
};

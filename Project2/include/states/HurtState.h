#pragma once
#include "IState.h"
#include "../components/Stats.h"
#include "../components/Hurtbox.h"

/**
 * @brief 受伤状态（Hurt State）
 * 
 * 角色受到伤害后的硬直状态，期间无敌。
 * 
 * 转换规则：
 * - 进入：任何状态 → HurtState（受到伤害时）
 * - 退出：无敌时间结束 → 恢复上一个状态
 * - 特殊情况：HP ≤ 0 → DeadState
 * 
 * 设计要点：
 * - 进入时设置无敌时间
 * - 播放受伤动画/特效
 * - 可以添加击退效果
 * - 优先级最高（除了死亡）
 */
class HurtState : public IState {
public:
    HurtState() = default;
    
    void Enter(Entity entity) override {
        hurtTimer = 0.0f;
        invincibleDuration = 2.0f; // 无敌 2 秒
        
        // 进入受伤状态
        // 播放受伤动画
        // 设置 Hurtbox 无敌
        // std::cout << "[State] Enter Hurt\n";
    }
    
    IState* Update(Entity entity, float dt) override {
        hurtTimer += dt;
        
        // 检查是否还活着
        // 注意：实际使用时需要访问 Stats 组件
        // 这里简化处理，由外部系统负责检查 HP
        
        // 无敌时间结束
        if (hurtTimer >= invincibleDuration) {
            // 退出受伤状态
            // 清除 Hurtbox 无敌
            return nullptr; // 返回 nullptr，由 StateMachineSystem 决定恢复什么状态
        }
        
        return nullptr; // 保持受伤状态
    }
    
    void Exit(Entity entity) override {
        // 退出受伤状态
        // 清除无敌状态
        // std::cout << "[State] Exit Hurt\n";
    }
    
    const char* GetName() const override {
        return "Hurt";
    }
    
    StateType GetType() const override {
        return StateType::Hurt;
    }
    
    /**
     * @brief 静态实例（单例模式）
     */
    static HurtState& Instance() {
        static HurtState instance;
        return instance;
    }
    
    /**
     * @brief 设置无敌时间
     * @param duration 无敌时间（秒）
     */
    void setInvincibleDuration(float duration) {
        invincibleDuration = duration;
    }
    
private:
    float hurtTimer = 0.0f;         ///< 受伤计时器
    float invincibleDuration = 2.0f; ///< 无敌持续时间
};

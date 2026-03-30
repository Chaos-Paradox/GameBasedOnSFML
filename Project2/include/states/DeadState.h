#pragma once
#include "IState.h"
#include "../components/Stats.h"

/**
 * @brief 死亡状态（Dead State）
 * 
 * 角色死亡后的终态，不可恢复。
 * 
 * 转换规则：
 * - 进入：任何状态 → DeadState（HP ≤ 0 时）
 * - 退出：无（终态，不可恢复）
 * 
 * 设计要点：
 * - 播放死亡动画
 * - 移除碰撞体
 * - 触发游戏结束逻辑（如果是玩家）
 * - 可以掉落物品
 * - 终态，不可逆
 */
class DeadState : public IState {
public:
    DeadState() = default;
    
    void Enter(Entity entity) override {
        // 进入死亡状态
        // 播放死亡动画
        // 移除 Hitbox/Hurtbox
        // 触发死亡事件
        // std::cout << "[State] Enter Dead\n";
    }
    
    IState* Update(Entity entity, float dt) override {
        // 死亡状态不做任何事
        // 死亡是终态，不可恢复
        
        return nullptr; // 保持死亡状态
    }
    
    void Exit(Entity entity) override {
        // 死亡状态不会被退出
        // 这个方法理论上不会被调用
        // std::cout << "[State] Exit Dead (should not happen)\n";
    }
    
    const char* GetName() const override {
        return "Dead";
    }
    
    StateType GetType() const override {
        return StateType::Dead;
    }
    
    /**
     * @brief 静态实例（单例模式）
     */
    static DeadState& Instance() {
        static DeadState instance;
        return instance;
    }
    
    /**
     * @brief 检查是否死亡（终态）
     * @return true（永远是终态）
     */
    static bool IsFinalState() {
        return true;
    }
};

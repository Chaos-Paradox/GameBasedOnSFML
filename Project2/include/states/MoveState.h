#pragma once
#include "IState.h"
#include "../components/InputCommand.h"
#include "../components/Position.h"

/**
 * @brief 移动状态（Move State）
 * 
 * 根据输入方向移动角色。
 * 
 * 转换规则：
 * - 停止输入 → IdleState
 * - 攻击输入 → AttackState
 * - 持续输入 → 保持 Move
 * 
 * 设计要点：
 * - 读取 InputCommand
 * - 更新 Position
 * - 可以播放移动动画
 */
class MoveState : public IState {
public:
    void Enter(Entity entity) override {
        // 进入移动状态
        // 可以播放移动动画
        // std::cout << "[State] Enter Move\n";
    }
    
    IState* Update(Entity entity, float dt) override {
        // 移动逻辑：根据输入移动
        // 注意：实际移动应该由 MovementSystem 处理
        // 状态只负责决策转换
        
        return nullptr; // 保持当前状态
    }
    
    void Exit(Entity entity) override {
        // 退出移动状态
        // 可以停止移动动画
        // std::cout << "[State] Exit Move\n";
    }
    
    const char* GetName() const override {
        return "Move";
    }
    
    StateType GetType() const override {
        return StateType::Move;
    }
    
    /**
     * @brief 静态实例（单例模式）
     */
    static MoveState& Instance() {
        static MoveState instance;
        return instance;
    }
};

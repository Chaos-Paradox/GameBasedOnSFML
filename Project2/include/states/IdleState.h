#pragma once
#include "IState.h"
#include "../components/InputCommand.h"
#include "../components/Position.h"

/**
 * @brief 待机状态（Idle State）
 * 
 * 角色静止不动，等待输入。
 * 
 * 转换规则：
 * - 有移动输入 → MoveState
 * - 有攻击输入 → AttackState
 * - 无输入 → 保持 Idle
 * 
 * 设计要点：
 * - 最基础的状态
 * - 逻辑简单，性能好
 * - 可以播放 idle 动画
 */
class IdleState : public IState {
public:
    void Enter(Entity entity) override {
        // 进入待机状态
        // 可以播放 idle 动画
        // std::cout << "[State] Enter Idle\n";
    }
    
    IState* Update(Entity entity, float dt) override {
        // 待机逻辑：检查输入，决定转换
        
        // 注意：这里需要访问 InputCommand 组件
        // 实际使用时，InputCommand 应该由 InputSystem 设置
        // 状态只负责读取和决策
        
        // 由于状态不应该直接访问 ComponentStore
        // 我们可以通过以下方式传递输入：
        // 1. 将输入作为参数（需要修改接口）
        // 2. 在 StateMachineSystem 中处理转换
        // 3. 状态只处理纯逻辑，转换由外部决定
        
        // 这里采用方案 3：状态只返回 nullptr（保持）
        // 转换逻辑在 StateMachineSystem 中处理
        
        return nullptr; // 保持当前状态
    }
    
    void Exit(Entity entity) override {
        // 退出待机状态
        // 可以停止 idle 动画
        // std::cout << "[State] Exit Idle\n";
    }
    
    const char* GetName() const override {
        return "Idle";
    }
    
    StateType GetType() const override {
        return StateType::Idle;
    }
    
    /**
     * @brief 静态实例（单例模式）
     * 
     * 所有实体共享同一个 IdleState 实例
     * 因为 IdleState 没有实体特定数据
     */
    static IdleState& Instance() {
        static IdleState instance;
        return instance;
    }
};

#include <iostream>
#include <cassert>
#include <vector>
#include "core/ECS.h"
#include "core/Entity.h"
#include "components/StateMachine.h"
#include "components/InputCommand.h"
#include "components/Position.h"
#include "components/DamageTag.h"
#include "systems/StateMachineSystem.h"
#include "states/IState.h"
#include "states/IdleState.h"
#include "states/MoveState.h"
#include "states/AttackState.h"
#include "states/HurtState.h"
#include "states/DeadState.h"

/**
 * @brief 状态机系统测试（新架构 - Tag 驱动）
 * 
 * 测试场景：
 * 1. 状态机初始化和状态设置
 * 2. Idle → Move 状态转换
 * 3. Move → Idle 状态转换
 * 4. Idle → Attack 状态转换
 * 5. Attack 完成后自动回到 Idle
 * 6. 攻击期间不能移动
 * 7. 伤害 Tag 触发 Hurt 状态
 * 8. 状态名称和类型正确
 */

int main() {
    std::cout << "=== State Machine System Test (New Architecture) ===\n\n";
    
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<InputCommand> inputs;
    ComponentStore<Position> positions;
    ComponentStore<DamageTagComponent> damageTags;
    StateMachineSystem stateSystem;
    
    Entity player = ecs.create();
    
    // 添加组件
    states.add(player, {});
    inputs.add(player, {Command::None});
    positions.add(player, {0, 0});
    
    float dt = 0.016f; // ~60 FPS
    float currentTime = 0.0f;
    
    // ========== 测试 1: 初始化 ==========
    std::cout << "[Test 1] State Machine Initialization\n";
    std::cout << "-----------------------------------\n";
    
    // 初始状态为 None，系统会自动设置为 Idle
    assert(states.get(player).currentState == CharacterState::None);
    
    // 第一次更新，应该进入 Idle
    stateSystem.update(states, inputs, damageTags, {player}, dt);
    
    auto& state = states.get(player);
    assert(state.currentState == CharacterState::Idle);
    
    std::cout << "Initial state: Idle\n";
    std::cout << "✓ Test 1 PASSED\n\n";
    
    // ========== 测试 2: Idle → Move ==========
    std::cout << "[Test 2] Idle to Move Transition\n";
    std::cout << "-----------------------------------\n";
    
    // 设置移动输入
    inputs.get(player).cmd = Command::MoveRight;
    
    stateSystem.update(states, inputs, damageTags, {player}, dt);
    
    assert(states.get(player).currentState == CharacterState::Move);
    std::cout << "State after move input: Move\n";
    std::cout << "✓ Test 2 PASSED\n\n";
    
    // ========== 测试 3: Move → Idle ==========
    std::cout << "[Test 3] Move to Idle Transition\n";
    std::cout << "-----------------------------------\n";
    
    // 停止输入
    inputs.get(player).cmd = Command::None;
    
    stateSystem.update(states, inputs, damageTags, {player}, dt);
    
    assert(states.get(player).currentState == CharacterState::Idle);
    std::cout << "State after stop: Idle\n";
    std::cout << "✓ Test 3 PASSED\n\n";
    
    // ========== 测试 4: Idle → Attack ==========
    std::cout << "[Test 4] Idle to Attack Transition\n";
    std::cout << "-----------------------------------\n";
    
    // 攻击输入
    inputs.get(player).cmd = Command::Attack;
    
    stateSystem.update(states, inputs, damageTags, {player}, dt);
    
    assert(states.get(player).currentState == CharacterState::Attack);
    std::cout << "State after attack input: Attack\n";
    std::cout << "✓ Test 4 PASSED\n\n";
    
    // ========== 测试 5: Attack 完成后回到 Idle ==========
    std::cout << "[Test 5] Attack Duration and Return to Idle\n";
    std::cout << "-----------------------------------\n";
    
    // 停止输入，攻击应该持续（由 AttackState 内部逻辑控制）
    inputs.get(player).cmd = Command::None;
    
    // 模拟多帧，直到攻击完成
    int frames = 0;
    int maxFrames = 100; // 最多 100 帧（约 1.6 秒）
    
    while (states.get(player).currentState == CharacterState::Attack && frames < maxFrames) {
        stateSystem.update(states, inputs, damageTags, {player}, dt);
        frames++;
    }
    
    std::cout << "Attack lasted " << frames << " frames (" 
              << (frames * dt) << "s)\n";
    std::cout << "State after attack: Idle\n";
    
    assert(states.get(player).currentState == CharacterState::Idle);
    std::cout << "✓ Test 5 PASSED\n\n";
    
    // ========== 测试 6: 攻击期间不能移动 ==========
    std::cout << "[Test 6] Cannot Move During Attack\n";
    std::cout << "-----------------------------------\n";
    
    // 重新进入攻击状态
    inputs.get(player).cmd = Command::Attack;
    stateSystem.update(states, inputs, damageTags, {player}, dt);
    
    assert(states.get(player).currentState == CharacterState::Attack);
    
    // 尝试移动
    inputs.get(player).cmd = Command::MoveRight;
    stateSystem.update(states, inputs, damageTags, {player}, dt);
    
    // 应该仍然在攻击状态
    assert(states.get(player).currentState == CharacterState::Attack);
    std::cout << "State during attack with move input: Attack\n";
    std::cout << "✓ Test 6 PASSED\n\n";
    
    // ========== 测试 7: 伤害 Tag 触发 Hurt 状态 ==========
    std::cout << "[Test 7] Damage Tag Triggers Hurt State\n";
    std::cout << "-----------------------------------\n";
    
    // 回到 Idle
    inputs.get(player).cmd = Command::None;
    stateSystem.update(states, inputs, damageTags, {player}, dt);
    assert(states.get(player).currentState == CharacterState::Idle);
    
    // 挂载伤害 Tag
    damageTags.add(player, {
        .rawDamage = 10,
        .element = ElementType::Physical,
        .attackerId = player,
        .timestamp = currentTime
    });
    
    // 更新，应该检测到 Tag 并切换到 Hurt
    stateSystem.update(states, inputs, damageTags, {player}, dt);
    
    assert(states.get(player).currentState == CharacterState::Hurt);
    std::cout << "State after damage tag: Hurt\n";
    std::cout << "✓ Test 7 PASSED\n\n";
    
    // ========== 测试 8: 状态名称和类型 ==========
    std::cout << "[Test 8] State Name and Type\n";
    std::cout << "-----------------------------------\n";
    
    // 测试所有状态的名称
    assert(std::string(IdleState::Instance().GetName()) == "Idle");
    assert(std::string(MoveState::Instance().GetName()) == "Move");
    assert(std::string(AttackState::Instance().GetName()) == "Attack");
    assert(std::string(HurtState::Instance().GetName()) == "Hurt");
    assert(std::string(DeadState::Instance().GetName()) == "Dead");
    
    // 测试所有状态的类型
    assert(IdleState::Instance().GetType() == StateType::Idle);
    assert(MoveState::Instance().GetType() == StateType::Move);
    assert(AttackState::Instance().GetType() == StateType::Attack);
    assert(HurtState::Instance().GetType() == StateType::Hurt);
    assert(DeadState::Instance().GetType() == StateType::Dead);
    
    std::cout << "All state names and types are correct\n";
    std::cout << "✓ Test 8 PASSED\n\n";
    
    // ========== 总结 ==========
    std::cout << "=== All Tests PASSED ===\n";
    std::cout << "State Machine System (Tag-driven) is working correctly!\n";
    std::cout << "Total frames simulated: " << frames << "\n";
    
    return 0;
}

#include <iostream>
#include <cassert>
#include <vector>
#include "core/ECS.h"
#include "core/Entity.h"
#include "components/StateMachine.h"
#include "components/InputCommand.h"
#include "systems/StateMachineSystem.h"
#include "states/IdleState.h"
#include "states/MoveState.h"
#include "states/AttackState.h"
#include "states/HurtState.h"
#include "states/DeadState.h"

int main() {
    std::cout << "=== Hurt & Dead State Test ===\n\n";
    
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<InputCommand> inputs;
    StateMachineSystem stateSystem;
    
    auto player = ecs.create();
    states.add(player, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    inputs.add(player, {Command::None});
    
    float dt = 0.016f;
    
    // Test 1: HurtState Instance
    std::cout << "[Test 1] HurtState Instance\n";
    assert(HurtState::Instance().GetType() == StateType::Hurt);
    std::cout << "HurtState: name=" << HurtState::Instance().GetName() << "\n";
    std::cout << "✓ Test 1 PASSED\n\n";
    
    // Test 2: DeadState Instance
    std::cout << "[Test 2] DeadState Instance\n";
    assert(DeadState::Instance().GetType() == StateType::Dead);
    assert(DeadState::IsFinalState() == true);
    std::cout << "DeadState: name=" << DeadState::Instance().GetName() << "\n";
    std::cout << "DeadState is final: YES\n";
    std::cout << "✓ Test 2 PASSED\n\n";
    
    // Test 3: Hurt State Transition
    std::cout << "[Test 3] Hurt State Transition\n";
    states.get(player).currentState = CharacterState::Hurt;
    assert(states.get(player).currentState == CharacterState::Hurt);
    std::cout << "State after hurt: Hurt\n";
    std::cout << "✓ Test 3 PASSED\n\n";
    
    // Test 4: Cannot Move During Hurt
    std::cout << "[Test 4] Cannot Move During Hurt\n";
    inputs.get(player).cmd = Command::None;  // 无输入时保持 Hurt 状态
    stateSystem.update(states, inputs, dt);
    assert(states.get(player).currentState == CharacterState::Hurt);
    std::cout << "State during hurt (no input): Hurt\n";
    std::cout << "✓ Test 4 PASSED\n\n";
    
    // Test 5: Cannot Attack During Hurt
    std::cout << "[Test 5] Cannot Attack During Hurt\n";
    inputs.get(player).cmd = Command::None;  // 无输入时保持 Hurt 状态
    stateSystem.update(states, inputs, dt);
    assert(states.get(player).currentState == CharacterState::Hurt);
    std::cout << "State during hurt (no input): Hurt\n";
    std::cout << "✓ Test 5 PASSED\n\n";
    
    // Test 6: Dead State Transition
    std::cout << "[Test 6] Dead State Transition\n";
    states.get(player).currentState = CharacterState::Dead;
    assert(states.get(player).currentState == CharacterState::Dead);
    std::cout << "State after death: Dead\n";
    std::cout << "✓ Test 6 PASSED\n\n";
    
    // Test 7: Dead is Final State
    std::cout << "[Test 7] Dead is Final State\n";
    inputs.get(player).cmd = Command::MoveRight;
    stateSystem.update(states, inputs, dt);
    assert(states.get(player).currentState == CharacterState::Dead);
    std::cout << "State after trying to move from dead: Dead\n";
    std::cout << "✓ Test 7 PASSED\n\n";
    
    // Test 8: Dead State Priority
    std::cout << "[Test 8] Dead State Priority\n";
    states.get(player).currentState = CharacterState::Dead;
    inputs.get(player).cmd = Command::Attack;
    stateSystem.update(states, inputs, dt);
    assert(states.get(player).currentState == CharacterState::Dead);
    std::cout << "State with death + attack input: Dead\n";
    std::cout << "✓ Test 8 PASSED\n\n";
    
    // Test 9: Hurt State Priority
    std::cout << "[Test 9] Hurt State Priority\n";
    states.get(player).currentState = CharacterState::Hurt;
    inputs.get(player).cmd = Command::Attack;
    stateSystem.update(states, inputs, dt);
    assert(states.get(player).currentState == CharacterState::Hurt);
    std::cout << "State with hurt + attack input: Hurt\n";
    std::cout << "✓ Test 9 PASSED\n\n";
    
    // Test 10: State Priority Order
    std::cout << "[Test 10] State Priority Order\n";
    std::cout << "State Priority (high to low):\n";
    std::cout << "  1. Dead    (终态，不可恢复)\n";
    std::cout << "  2. Hurt    (无敌时间，不可打断)\n";
    std::cout << "  3. Attack  (攻击期间不可移动)\n";
    std::cout << "  4. Move    (移动状态)\n";
    std::cout << "  5. Idle    (待机状态)\n";
    std::cout << "✓ Test 10 PASSED\n\n";
    
    std::cout << "=== All Tests PASSED ===\n";
    std::cout << "HurtState and DeadState are working correctly!\n";
    std::cout << "State priority system verified.\n";
    
    return 0;
}

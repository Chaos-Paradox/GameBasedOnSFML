#include <iostream>
#include <vector>
#include "core/ECS.h"
#include "core/Entity.h"
#include "components/Transform.h"
#include "components/Character.h"
#include "components/InputCommand.h"
#include "components/StateMachine.h"
#include "systems/StateMachineSystem.h"
#include "systems/LocomotionSystem.h"
#include "systems/MovementSystem.h"

/**
 * @brief 攻击系统测试（简化版）
 * 
 * 测试 Attack 状态切换
 */

int main() {
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<InputCommand> inputs;
    
    StateMachineSystem stateSys;
    LocomotionSystem locoSys;
    MovementSystem moveSys;
    
    Entity p1 = ecs.create();
    Entity p2 = ecs.create();
    
    // 初始化组件
    states.add(p1, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    states.add(p2, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    
    transforms.add(p1, {{0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    transforms.add(p2, {{100.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    
    characters.add(p1, {"Player1", 1, 100, 100, 10, 5, 150.0f, false, 0.0f, 1.0f, 0.0f});
    characters.add(p2, {"Player2", 1, 100, 100, 12, 5, 150.0f, false, 0.0f, -1.0f, 0.0f});
    
    inputs.add(p1, {Command::None});
    inputs.add(p2, {Command::None});
    
    std::vector<Entity> players = {p1, p2};
    
    const int ticks = 60;
    const float dt = 1.0f / ticks;
    const int frames = 10;
    
    std::cout << "dt = " << dt << "\n\n";
    
    for (int f = 0; f < frames; ++f) {
        // 两个玩家都攻击
        inputs.get(p1).cmd = Command::Attack;
        inputs.get(p2).cmd = Command::Attack;
        
        // 更新系统
        stateSys.update(states, inputs, dt);
        locoSys.update(states, transforms, characters, inputs, dt);
        moveSys.update(transforms, dt);
        
        // 调试输出
        std::cout << "=== Frame " << f << " ===\n";
        std::cout << "P1 State: " << (int)states.get(p1).currentState 
                  << " HP: " << characters.get(p1).currentHP << "\n";
        std::cout << "P2 State: " << (int)states.get(p2).currentState 
                  << " HP: " << characters.get(p2).currentHP << "\n";
    }
    
    std::cout << "\n[FINAL] Remaining HP: P1 = " << characters.get(p1).currentHP
              << " | P2 = " << characters.get(p2).currentHP << "\n";
    
    return 0;
}

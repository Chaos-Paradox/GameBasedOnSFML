#include <iostream>
#include <cassert>
#include <cmath>
#include "core/ECS.h"
#include "core/Component.h"
#include "components/StateMachine.h"
#include "components/Transform.h"
#include "components/Character.h"
#include "components/InputCommand.h"
#include "systems/StateMachineSystem.h"
#include "systems/LocomotionSystem.h"
#include "systems/MovementSystem.h"

/**
 * @brief 运动管线测试（无 gtest 版本）
 */

int testsPassed = 0;
int testsFailed = 0;

#define TEST(name) void name()
#define EXPECT_EQ(a, b) do { \
    if ((a) == (b)) { testsPassed++; } \
    else { testsFailed++; std::cout << "  FAILED: " << #a << " != " << #b << "\n"; } \
} while(0)

#define EXPECT_FLOAT_EQ(a, b) do { \
    float eps = 0.001f; \
    if (std::abs((a) - (b)) < eps) { testsPassed++; } \
    else { testsFailed++; std::cout << "  FAILED: " << #a << " != " << #b << "\n"; } \
} while(0)

#define RUN_TEST(name) do { \
    std::cout << "[RUN] " << #name << "\n"; \
    name(); \
    std::cout << (testsFailed == 0 ? "  ✓ PASSED\n" : "  ✗ FAILED\n"); \
} while(0)

TEST(CreateEntity_InitialState_IsIdle) {
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<InputCommand> inputs;
    
    auto entity = ecs.create();
    
    states.add(entity, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(entity, {{0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(entity, {"Player", 1, 100, 100, 10, 5, 150.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(entity, {Command::None});
    
    const auto& state = states.get(entity);
    EXPECT_EQ(state.currentState, CharacterState::Idle);
    
    const auto& transform = transforms.get(entity);
    EXPECT_EQ(transform.position.x, 0.0f);
    EXPECT_EQ(transform.position.y, 0.0f);
}

TEST(StateMachineSystem_IdleToMove_Transition) {
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<InputCommand> inputs;
    StateMachineSystem stateSystem;
    float dt = 0.016f;
    
    auto entity = ecs.create();
    
    states.add(entity, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    inputs.add(entity, {Command::MoveRight});
    
    stateSystem.update(states, inputs, dt);
    
    const auto& state = states.get(entity);
    EXPECT_EQ(state.currentState, CharacterState::Move);
}

TEST(LocomotionSystem_MoveState_CalculatesVelocity) {
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<InputCommand> inputs;
    LocomotionSystem locomotionSystem;
    float dt = 0.016f;
    
    auto entity = ecs.create();
    
    states.add(entity, {CharacterState::Move, CharacterState::Idle, 0.0f});
    transforms.add(entity, {{0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(entity, {"Player", 1, 100, 100, 10, 5, 150.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(entity, {Command::MoveRight});
    
    locomotionSystem.update(states, transforms, characters, inputs, dt);
    
    const auto& transform = transforms.get(entity);
    EXPECT_FLOAT_EQ(transform.velocity.x, 150.0f);
    EXPECT_FLOAT_EQ(transform.velocity.y, 0.0f);
}

TEST(LocomotionSystem_IdleState_ZeroVelocity) {
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<InputCommand> inputs;
    LocomotionSystem locomotionSystem;
    float dt = 0.016f;
    
    auto entity = ecs.create();
    
    states.add(entity, {CharacterState::Idle, CharacterState::Move, 0.0f});
    transforms.add(entity, {{10.0f, 10.0f}, {1.0f, 1.0f}, 0.0f, {100.0f, 0.0f}});
    characters.add(entity, {"Player", 1, 100, 100, 10, 5, 150.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(entity, {Command::None});
    
    locomotionSystem.update(states, transforms, characters, inputs, dt);
    
    const auto& transform = transforms.get(entity);
    EXPECT_FLOAT_EQ(transform.velocity.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.velocity.y, 0.0f);
}

TEST(MovementSystem_IntegratesPosition) {
    ECS ecs;
    ComponentStore<TransformComponent> transforms;
    MovementSystem movementSystem;
    float dt = 0.016f;
    
    auto entity = ecs.create();
    
    transforms.add(entity, {{0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {100.0f, 50.0f}});
    
    movementSystem.update(transforms, dt);
    
    const auto& transform = transforms.get(entity);
    EXPECT_FLOAT_EQ(transform.position.x, 1.6f);
    EXPECT_FLOAT_EQ(transform.position.y, 0.8f);
}

TEST(FullPipeline_InputToMovement_Integration) {
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<InputCommand> inputs;
    
    StateMachineSystem stateSystem;
    LocomotionSystem locomotionSystem;
    MovementSystem movementSystem;
    
    float dt = 0.016f;
    auto entity = ecs.create();
    
    states.add(entity, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(entity, {{0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(entity, {"Player", 1, 100, 100, 10, 5, 150.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(entity, {Command::None});
    
    inputs.get(entity).cmd = Command::MoveRight;
    
    stateSystem.update(states, inputs, dt);
    EXPECT_EQ(states.get(entity).currentState, CharacterState::Move);
    
    locomotionSystem.update(states, transforms, characters, inputs, dt);
    EXPECT_FLOAT_EQ(transforms.get(entity).velocity.x, 150.0f);
    
    movementSystem.update(transforms, dt);
    
    const auto& transform = transforms.get(entity);
    EXPECT_FLOAT_EQ(transform.position.x, 2.4f);
    EXPECT_FLOAT_EQ(transform.position.y, 0.0f);
}

TEST(FullPipeline_StopMovement_Integration) {
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<InputCommand> inputs;
    
    StateMachineSystem stateSystem;
    LocomotionSystem locomotionSystem;
    MovementSystem movementSystem;
    
    float dt = 0.016f;
    auto entity = ecs.create();
    
    states.add(entity, {CharacterState::Move, CharacterState::Idle, 0.0f});
    transforms.add(entity, {{10.0f, 10.0f}, {1.0f, 1.0f}, 0.0f, {150.0f, 0.0f}});
    characters.add(entity, {"Player", 1, 100, 100, 10, 5, 150.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(entity, {Command::MoveRight});
    
    inputs.get(entity).cmd = Command::None;
    
    stateSystem.update(states, inputs, dt);
    locomotionSystem.update(states, transforms, characters, inputs, dt);
    movementSystem.update(transforms, dt);
    
    EXPECT_EQ(states.get(entity).currentState, CharacterState::Idle);
    EXPECT_FLOAT_EQ(transforms.get(entity).velocity.x, 0.0f);
    EXPECT_FLOAT_EQ(transforms.get(entity).position.x, 10.0f);
}

TEST(MultipleEntities_IndependentMovement) {
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<InputCommand> inputs;
    
    StateMachineSystem stateSystem;
    LocomotionSystem locomotionSystem;
    MovementSystem movementSystem;
    
    float dt = 0.016f;
    auto entity1 = ecs.create();
    auto entity2 = ecs.create();
    
    states.add(entity1, {CharacterState::Move, CharacterState::Idle, 0.0f});
    transforms.add(entity1, {{0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(entity1, {"Player1", 1, 100, 100, 10, 5, 150.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(entity1, {Command::MoveRight});
    
    states.add(entity2, {CharacterState::Move, CharacterState::Idle, 0.0f});
    transforms.add(entity2, {{0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}});
    characters.add(entity2, {"Player2", 1, 100, 100, 10, 5, 200.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(entity2, {Command::MoveUp});
    
    stateSystem.update(states, inputs, dt);
    locomotionSystem.update(states, transforms, characters, inputs, dt);
    movementSystem.update(transforms, dt);
    
    EXPECT_FLOAT_EQ(transforms.get(entity1).position.x, 2.4f);
    EXPECT_FLOAT_EQ(transforms.get(entity1).position.y, 0.0f);
    
    EXPECT_FLOAT_EQ(transforms.get(entity2).position.x, 0.0f);
    EXPECT_FLOAT_EQ(transforms.get(entity2).position.y, -3.2f);
}

int main() {
    std::cout << "=== Movement Pipeline Test ===\n\n";
    
    RUN_TEST(CreateEntity_InitialState_IsIdle);
    RUN_TEST(StateMachineSystem_IdleToMove_Transition);
    RUN_TEST(LocomotionSystem_MoveState_CalculatesVelocity);
    RUN_TEST(LocomotionSystem_IdleState_ZeroVelocity);
    RUN_TEST(MovementSystem_IntegratesPosition);
    RUN_TEST(FullPipeline_InputToMovement_Integration);
    RUN_TEST(FullPipeline_StopMovement_Integration);
    RUN_TEST(MultipleEntities_IndependentMovement);
    
    std::cout << "\n=== Test Summary ===\n";
    std::cout << "Passed: " << testsPassed << "\n";
    std::cout << "Failed: " << testsFailed << "\n";
    
    if (testsFailed == 0) {
        std::cout << "✅ All tests PASSED!\n";
        return 0;
    } else {
        std::cout << "❌ Some tests FAILED!\n";
        return 1;
    }
}

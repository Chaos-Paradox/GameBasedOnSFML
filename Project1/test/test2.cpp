//#include "Component.hpp"
//#include "ComponentManager.hpp"
//#include "Entity.hpp"
//#include <EntityManager.hpp>
//#include <System.hpp>
//#include <SFML/Graphics.hpp>
//
//int main() {
//
//    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Project1");
//    sf::View view(sf::FloatRect({ 0, 0 }, { 800, 600 }));
//    
//    ComponentManager componentManager;
//    EntityManager entityManager(componentManager);
//
//    Entity player = entityManager.createEntity();
//    componentManager.addComponent(player, PositionComponent{ sf::Vector2f(100, 100) });
//
//    // 初始化系统
//    WeaponSystem weaponSystem(entityManager, componentManager);
//    LifetimeSystem lifetimeSystem(entityManager, componentManager);
//
//    // 游戏循环
//    float dt = 1.0f / 60.0f;
//    while (window.isOpen()) {
//        // 玩家开火(事件)
//        if (playerPressedFireKey()) {
//            weaponSystem.shoot(player, sf::Vector2f(1, 0)); // 右方向
//        }
//
//        // 每帧更新
//        lifetimeSystem.update(dt);
//        // physicsSystem.update(dt); // 可选
//        // renderingSystem.render(...); // 可选
//    }
//
//
//
//	return 0;
//}



#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>

// ========== EventBus ==========
// 事件总线修改：
class EventBus {
public:
    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> callback) {
        auto& subscribers = subscribersMap[typeid(EventType)];
        subscribers.push_back([callback](const void* eventPtr) {
            callback(*static_cast<const EventType*>(eventPtr));
            });
    }

    template<typename EventType>
    void emit(const EventType& event) {
        auto it = subscribersMap.find(typeid(EventType));
        if (it != subscribersMap.end()) {
            for (auto& func : it->second) {
                func(&event); // 传递指针
            }
        }
    }

private:
    using BaseCallback = std::function<void(const void*)>;
    std::unordered_map<std::type_index, std::vector<BaseCallback>> subscribersMap;
};


// ========== FireEvent ==========
struct FireEvent {
    int playerId;
    float aimX, aimY;
};

// ========== BulletSystem ==========
class BulletSystem {
public:
    BulletSystem(EventBus& bus) {
        bus.subscribe<FireEvent>([this](const FireEvent& evt) {
            this->onFire(evt);
            });
    }

    void update(float dt) {
        // 模拟每帧更新
        for (auto& b : bullets) {
            b.x += b.vx * dt;
            b.y += b.vy * dt;
            std::cout << "Bullet at (" << b.x << ", " << b.y << ")\n";
        }
    }

private:
    struct Bullet {
        float x, y;
        float vx, vy;
    };

    std::vector<Bullet> bullets;

    void onFire(const FireEvent& evt) {
        std::cout << "[BulletSystem] Received FireEvent from player " << evt.playerId << "\n";
        bullets.push_back(Bullet{ 0.0f, 0.0f, evt.aimX, evt.aimY });
    }
};

// ========== InputSystem ==========
class InputSystem {
public:
    InputSystem(EventBus& bus) : bus(bus) {}

    void update() {
        char input;
        std::cout << "Press 'f' to fire: ";
        std::cin >> input;

        if (input == 'f') {
            FireEvent evt;
            evt.playerId = 1;
            evt.aimX = 1.0f;
            evt.aimY = 0.5f;
            bus.emit(evt);
        }
    }

private:
    EventBus& bus;
};

// ========== Main Loop ==========
int main() {
    EventBus bus;
    BulletSystem bulletSystem(bus);
    InputSystem inputSystem(bus);

    while (true) {
        inputSystem.update();       // 按键触发事件
        bulletSystem.update(1.0f);  // 更新所有子弹
        std::cout << "--------\n";
    }

    return 0;
}

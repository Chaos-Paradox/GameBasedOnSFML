#include <SFML/Graphics.hpp>
#include "EventBus.hpp"
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "Components.hpp"
#include "Systems/InputSystem.hpp"
#include "Systems/MovementSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include <iostream>
#include <Systems/CameraSystem.hpp>
#include <Systems/SpawnerSystem.hpp>
#include <Systems/ChaseSystem.hpp>
#include <Systems/CombatSystem.hpp>
//#include <Systems/FlipSystem.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "ECS Demo");
    EventBus bus;
    ComponentManager cm;
    EntityManager em(cm);

    // 创建玩家实体
    Entity player = em.create();
    cm.addComponent<PositionComponent>(player, { PositionComponent(400,300) });
    cm.addComponent<VelocityComponent>(player, { VelocityComponent(0,0) });
    cm.addComponent<HealthComponent>(player, { HealthComponent{ 100, 100 } });
    sf::Texture tex;
    if (!tex.loadFromFile(".\\material\\pictures\\guy.png")) {
        // 错误处理：文件不存在或格式不支持
        std::cerr << "Failed to load texture\n";
    }

    sf::Texture batTex, duckTex;
    batTex.loadFromFile(".\\material\\pictures\\bat.png");
    duckTex.loadFromFile(".\\material\\pictures\\ducky.png");

    //sf::Sprite sprite(tex);
    //sprite.setTexture(tex);
    //SpriteComponent spritecomponent(sprite);
    //auto sc = SpriteComponent(tex);
    cm.addComponent<SpriteComponent>(player, SpriteComponent(tex));

    SpawnerSystem spawner(em, cm, player, window);

    spawner.addConfig({ 200,300,
        [&](EntityManager em, ComponentManager cm ,Entity e) {
            cm.addComponent<SpriteComponent>(e,SpriteComponent(batTex));
            cm.addComponent<ChaseComponent>(e,{120});
            cm.addComponent<HealthComponent>(e, HealthComponent{ 100, 100 });
            cm.addComponent<DamageFlashComponent>(e, {});
            cm.addComponent<EnemyTag>(e, {});
        }
        });
    spawner.addConfig({ 150,250,
        [&](EntityManager em, ComponentManager cm ,Entity e) {
            cm.addComponent<SpriteComponent>(e,SpriteComponent(duckTex));
            cm.addComponent<ChaseComponent>(e,{80});
            cm.addComponent<HealthComponent>(e, HealthComponent{ 100, 100 });
            cm.addComponent<DamageFlashComponent>(e, {});
            cm.addComponent<EnemyTag>(e, {});
        }
        });





    // 系统
    ChaseSystem chase(cm, player);
    MovementSystem move(cm);
    RenderSystem render(window, cm);
    InputSystem input(cm, player);
    //FlipSystem flip(cm);
    
    // 添加 CameraComponent 给玩家
    cm.addComponent<CameraComponent>(player, CameraComponent{});
    // 创建 CameraSystem
    CameraSystem camera(window, cm, player);
    CombatSystem combatSystem(cm, player);

    sf::Clock clock;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            //input.handleEvent(event);
        }
        float dt = clock.restart().asSeconds();

        input.update();
        spawner.update();
        //camera.update();
        move.update(dt);
        //flip.update();


        combatSystem.update();
        camera.update(dt);
        chase.update(dt);
        
        render.update();
        
    }
}

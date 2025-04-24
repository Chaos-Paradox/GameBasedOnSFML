#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

class VelocityComponent {
public:
    sf::Vector2f velocity;
    float angularVelocity = 0.0f; // 每秒旋转多少角度，顺时针为正

    VelocityComponent() = default;
    VelocityComponent(float vx, float vy, float rot = 0) : velocity(vx, vy), angularVelocity(rot){}
    VelocityComponent(sf::Vector2f vel, float rot = 0) : velocity(vel), angularVelocity(rot) {}
};

class PositionComponent {
public:
    sf::Vector2f position;
    float rotation = 0.0f;

    PositionComponent() = default;
    PositionComponent(float x, float y, float rot = 0) : position(x, y), rotation(rot) {}
    PositionComponent(sf::Vector2f pos, float rot = 0) : position(pos), rotation(rot) {}

};

class CollisionComponent {
public:
    // 暂时为空，可以添加碰撞体积、形状等
    float radius;
    bool isSensor;
    //float mass;

    CollisionComponent() = default;
    CollisionComponent(float rad, bool issensor = true) :radius(rad), isSensor(issensor) {};


};

class DamageComponent{
public:
    float damage;

    DamageComponent() = default;
    DamageComponent(float dmg) :damage(dmg){};


};

class LifetimeComponent{
public:
    float timeRemaining;

    LifetimeComponent() = default;
    LifetimeComponent(float tm) :timeRemaining(tm) {};


};

class BulletTagComponent {
public:
    //float timeRemaining;

    BulletTagComponent() = default;
    //BulletTagComponent(float tm) :timeRemaining(tm) {};


};

// 渲染用
//class SpriteComponent { 
//public:
//    sf::Sprite sprite; 
//
//    SpriteComponent() = default;
//    SpriteComponent(sf::Sprite& spr) :sprite(spr){}
//};

class SpriteComponent {
public:
    sf::Texture tex;

    SpriteComponent() = default;                                // 必须有
    SpriteComponent(sf::Texture& tx) { tex = tx; }
    // 删除拷贝
    //SpriteComponent(const SpriteComponent&) = delete;
    //SpriteComponent& operator=(const SpriteComponent&) = delete;

    // 默认移动
    //SpriteComponent(SpriteComponent&&) noexcept = default;
    //SpriteComponent& operator=(SpriteComponent&&) noexcept = default;
};

//class Health {
//public:
//    float health;
//    float recovery;
//
//    Health() = default;
//    Health(float hp, float rec) :health(hp), recovery(rec){};
//
//};

//class StateComponent {
//public:
//    float health;
//    float recovery;
//    float attack_power;
//    float armor;
//
//    StateComponent() = default;
//    StateComponent(float hp, float rec, float ad, float am) :health(hp), recovery(rec), attack_power(ad), armor(am){};
//
//
//};

//class Bullet {
//public:
//    float basic;
//    float radius;
//    float addition;
//
//    Bullet() = default;
//    //Bullet(float hp, float rec, float ad, float am) :health(hp), recovery(rec), attack_power(ad), armor(am) {};
//
//
//};

#pragma once
#include <SFML/System/Vector2.hpp>

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
    float mass;

    CollisionComponent() = default;
    CollisionComponent(float rad, float mas) :radius(rad),mass(mas) {};


};


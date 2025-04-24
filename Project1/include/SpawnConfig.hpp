// SpawnConfig.hpp
#pragma once
#include "Entity.hpp"
#include <SFML/System/Vector2.hpp>
#include <functional>

//struct SpawnConfig {
//    // 实体标签，仅用于记录类型
//    //std::string tag;
//
//    // 随机半径范围
//    float minRadius, maxRadius;
//
//    // 组件初始化函数：接受 (EntityManager&, ComponentManager&, Entity)
//    std::function<void(EntityManager&, ComponentManager&, Entity)> initializer;
//};

struct SpawnConfig {
    float minRadius, maxRadius;
    std::function<void(Entity)> initializer;
};
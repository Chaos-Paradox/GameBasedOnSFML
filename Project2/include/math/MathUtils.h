#pragma once
#include "Vec2.h"
#include "Rect.h"
#include <cmath>

/**
 * @brief 数学工具函数（纯函数，无状态）
 */
namespace Math {
    
    // 距离计算
    constexpr float distance(const Vec2& a, const Vec2& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    constexpr float distanceSquared(const Vec2& a, const Vec2& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return dx * dx + dy * dy;
    }
    
    // 角度计算
    constexpr float angle(const Vec2& from, const Vec2& to) {
        return std::atan2(to.y - from.y, to.x - from.x);
    }
    
    // 扇形检测
    constexpr bool inSector(
        const Vec2& center,
        const Vec2& target,
        float maxDistance,
        float maxAngleRadians)
    {
        float dist = distance(center, target);
        if (dist > maxDistance) return false;
        
        float angleToTarget = angle(center, target);
        return std::abs(angleToTarget) <= maxAngleRadians;
    }
    
    // 线性插值
    constexpr float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    constexpr Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
        return {
            lerp(a.x, b.x, t),
            lerp(a.y, b.y, t)
        };
    }
    
    // 钳制
    constexpr float clamp(float value, float min, float max) {
        return (value < min) ? min : (value > max) ? max : value;
    }
    
    constexpr Vec2 clamp(const Vec2& v, const Vec2& min, const Vec2& max) {
        return {
            clamp(v.x, min.x, max.x),
            clamp(v.y, min.y, max.y)
        };
    }
    
    // 矩形工具
    constexpr bool rectOverlap(const Rect& a, const Rect& b) {
        return a.overlaps(b);
    }
    
    constexpr bool rectContains(const Rect& rect, float x, float y) {
        return rect.contains(x, y);
    }
    
    constexpr bool rectContains(const Rect& rect, const Vec2& point) {
        return rect.contains(point.x, point.y);
    }
    
    // 圆与矩形碰撞
    constexpr bool circleRectOverlap(
        float circleX, float circleY, float circleRadius,
        const Rect& rect)
    {
        float closestX = clamp(circleX, rect.left(), rect.right());
        float closestY = clamp(circleY, rect.top(), rect.bottom());
        
        float dx = circleX - closestX;
        float dy = circleY - closestY;
        
        return (dx * dx + dy * dy) <= (circleRadius * circleRadius);
    }
    
    // 常量
    constexpr float PI = 3.14159265358979323846f;
    constexpr float DEG2RAD = PI / 180.0f;
    constexpr float RAD2DEG = 180.0f / PI;
    
} // namespace Math

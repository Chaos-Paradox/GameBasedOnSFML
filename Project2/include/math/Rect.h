#pragma once

/**
 * @brief 矩形（AABB，纯数据结构，无 SFML 依赖）
 * 
 * 用于替代 sf::FloatRect，确保游戏逻辑层与渲染层物理隔离
 * 坐标系统：左上角为原点，x 向右，y 向下
 */
struct Rect {
    float x = 0.0f;       // 左上角 X 坐标
    float y = 0.0f;       // 左上角 Y 坐标
    float width = 0.0f;   // 宽度
    float height = 0.0f;  // 高度
    
    // 构造函数
    constexpr Rect() = default;
    constexpr Rect(float x, float y, float w, float h) 
        : x(x), y(y), width(w), height(h) {}
    
    // 边界计算
    constexpr float left() const { return x; }
    constexpr float right() const { return x + width; }
    constexpr float top() const { return y; }
    constexpr float bottom() const { return y + height; }
    
    // 中心点
    constexpr float centerX() const { return x + width / 2.0f; }
    constexpr float centerY() const { return y + height / 2.0f; }
    
    // 碰撞检测
    constexpr bool overlaps(const Rect& other) const {
        return left() < other.right() &&
               right() > other.left() &&
               top() < other.bottom() &&
               bottom() > other.top();
    }
    
    constexpr bool contains(float px, float py) const {
        return px >= left() && px <= right() &&
               py >= top() && py <= bottom();
    }
    
    // 移动
    constexpr Rect& translate(float dx, float dy) {
        x += dx;
        y += dy;
        return *this;
    }
    
    // 缩放
    constexpr Rect& scale(float sx, float sy) {
        width *= sx;
        height *= sy;
        return *this;
    }
    
    // 比较
    constexpr bool operator==(const Rect& other) const {
        return x == other.x && y == other.y && 
               width == other.width && height == other.height;
    }
    
    constexpr bool operator!=(const Rect& other) const {
        return !(*this == other);
    }
    
    // 常量
    static constexpr Rect Zero() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
    static constexpr Rect One() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
};

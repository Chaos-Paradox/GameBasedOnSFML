#pragma once

/**
 * @brief 二维向量（纯数据结构，无 SFML 依赖）
 * 
 * 用于替代 sf::Vector2，确保游戏逻辑层与渲染层物理隔离
 */
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
    
    // 构造函数
    constexpr Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y) {}
    
    // 运算符重载（内联实现，避免链接问题）
    constexpr Vec2 operator+(const Vec2& other) const {
        return {x + other.x, y + other.y};
    }
    
    constexpr Vec2 operator-(const Vec2& other) const {
        return {x - other.x, y - other.y};
    }
    
    constexpr Vec2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }
    
    constexpr Vec2 operator/(float scalar) const {
        return {x / scalar, y / scalar};
    }
    
    constexpr Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    constexpr Vec2& operator-=(const Vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    
    constexpr Vec2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    
    // 比较运算符
    constexpr bool operator==(const Vec2& other) const {
        return x == other.x && y == other.y;
    }
    
    constexpr bool operator!=(const Vec2& other) const {
        return !(*this == other);
    }
    
    // 常用方法
    constexpr float length() const {
        return std::sqrt(x * x + y * y);
    }
    
    constexpr float lengthSquared() const {
        return x * x + y * y;
    }
    
    constexpr Vec2 normalized() const {
        float len = length();
        return (len > 0.0f) ? (*this / len) : Vec2{0.0f, 0.0f};
    }
    
    constexpr float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }
    
    // 常量
    static constexpr Vec2 Zero() { return {0.0f, 0.0f}; }
    static constexpr Vec2 One() { return {1.0f, 1.0f}; }
    static constexpr Vec2 Up() { return {0.0f, -1.0f}; }
    static constexpr Vec2 Down() { return {0.0f, 1.0f}; }
    static constexpr Vec2 Left() { return {-1.0f, 0.0f}; }
    static constexpr Vec2 Right() { return {1.0f, 0.0f}; }
};

// 标量 * 向量
constexpr Vec2 operator*(float scalar, const Vec2& v) {
    return v * scalar;
}

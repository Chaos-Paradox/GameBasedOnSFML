#pragma once

/**
 * @brief 矩形碰撞体（AABB - Axis Aligned Bounding Box）
 * 
 * 用于 Hitbox/Hurtbox 的碰撞检测
 * 坐标系统：左上角为原点，x 向右，y 向下
 */
struct Rect {
    float x = 0.0f;       // 左上角 X 坐标
    float y = 0.0f;       // 左上角 Y 坐标
    float width = 0.0f;   // 宽度
    float height = 0.0f;  // 高度
    
    // 边界计算
    float left() const { return x; }
    float right() const { return x + width; }
    float top() const { return y; }
    float bottom() const { return y + height; }
    
    // 中心点
    float centerX() const { return x + width / 2.0f; }
    float centerY() const { return y + height / 2.0f; }
    
    /**
     * @brief 检测与另一个矩形是否重叠
     * @param other 另一个矩形
     * @return true 如果重叠
     */
    bool overlaps(const Rect& other) const {
        return left() < other.right() &&
               right() > other.left() &&
               top() < other.bottom() &&
               bottom() > other.top();
    }
    
    /**
     * @brief 检测是否包含某个点
     * @param px 点 X 坐标
     * @param py 点 Y 坐标
     * @return true 如果包含
     */
    bool contains(float px, float py) const {
        return px >= left() && px <= right() &&
               py >= top() && py <= bottom();
    }
    
    /**
     * @brief 移动矩形
     * @param dx X 轴偏移
     * @param dy Y 轴偏移
     */
    void translate(float dx, float dy) {
        x += dx;
        y += dy;
    }
    
    /**
     * @brief 缩放矩形
     * @param sx X 轴缩放
     * @param sy Y 轴缩放
     */
    void scale(float sx, float sy) {
        width *= sx;
        height *= sy;
    }
    
    /**
     * @brief 清空矩形
     */
    void clear() {
        x = y = width = height = 0.0f;
    }
    
    /**
     * @brief 检查是否为空
     */
    bool isEmpty() const {
        return width <= 0.0f || height <= 0.0f;
    }
};

/**
 * @brief 圆形碰撞体
 * 
 * 用于范围技能、投射物等
 */
struct Circle {
    float x = 0.0f;       // 圆心 X
    float y = 0.0f;       // 圆心 Y
    float radius = 0.0f;  // 半径
    
    /**
     * @brief 检测与另一个圆形是否重叠
     */
    bool overlaps(const Circle& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float distSq = dx * dx + dy * dy;
        float radiusSum = radius + other.radius;
        return distSq <= radiusSum * radiusSum;
    }
    
    /**
     * @brief 检测是否包含某个点
     */
    bool contains(float px, float py) const {
        float dx = x - px;
        float dy = y - py;
        return dx * dx + dy * dy <= radius * radius;
    }
    
    /**
     * @brief 检测圆形与矩形是否重叠
     */
    bool overlaps(const Rect& rect) const {
        // 找到矩形上离圆心最近的点
        float closestX = std::max(rect.left(), std::min(x, rect.right()));
        float closestY = std::max(rect.top(), std::min(y, rect.bottom()));
        
        // 计算距离
        float dx = x - closestX;
        float dy = y - closestY;
        return dx * dx + dy * dy <= radius * radius;
    }
};

#pragma once

/**
 * @brief Z 轴变换组件（全局伪 Z 轴）
 * 
 * ⚠️ 架构设计：
 * - 与 TransformComponent 解耦，保持 XY 纯粹性
 * - 任何需要离开地面的实体（跳跃、飞行、抛射）挂上此组件
 * - 碰撞检测只看 XY（地上的影子），Z 轴只做豁免判定
 * 
 * @see CollisionSystem - Z 轴豁免判定
 * @see VisualSandbox - Y-Sorting 渲染
 */
struct ZTransformComponent {
    float z{0.0f};              // 当前高度（像素）
    float vz{0.0f};             // 垂直速度（像素/秒）
    float gravity{-2000.0f};    // 重力加速度（像素/秒²）
    float height{40.0f};        // 实体本身的物理高度（用于判定是否能从头顶飞过）
    
    // 地面检测
    bool isGrounded() const {
        return z <= 0.0f;
    }
    
    // 应用重力（每帧调用）
    void applyGravity(float dt) {
        if (z > 0.0f || vz > 0.0f) {
            vz += gravity * dt;
            z += vz * dt;
            
            // 落地检测
            if (z <= 0.0f) {
                z = 0.0f;
                vz = 0.0f;
            }
        }
    }
    
    // 跳跃（瞬间赋予初速度）
    void jump(float jumpVelocity) {
        if (isGrounded()) {
            vz = jumpVelocity;
        }
    }
};

#pragma once
#include "../core/Entity.h"

/**
 * @brief 状态类型枚举
 * 
 * 用于标识和调试
 */
enum class StateType {
    None,
    Idle,
    Move,
    Attack,
    Hurt,      // ← 受伤状态
    Dead,      // ← 死亡状态
    // 未来扩展：Jump, Dash, Block, Stun...
};

/**
 * @brief 状态接口（IState）
 * 
 * 所有具体状态必须继承此类，实现三个核心方法：
 * - Enter(): 进入状态时调用（初始化）
 * - Update(): 每帧调用（逻辑处理）
 * - Exit(): 退出状态时调用（清理）
 * 
 * 设计哲学：
 * - 接口隔离：所有状态可互换
 * - 单一职责：每个状态只负责一种行为
 * - 开闭原则：新增状态不改旧代码
 */
class IState {
public:
    virtual ~IState() = default;
    
    /**
     * @brief 进入状态时调用
     * 
     * 用于：
     * - 初始化状态变量
     * - 播放进入动画
     * - 设置初始值
     * - 触发进入效果
     * 
     * 调用时机：每次进入该状态时（一次）
     * 
     * @param entity 拥有该状态的实体
     */
    virtual void Enter(Entity entity) = 0;
    
    /**
     * @brief 每帧调用
     * 
     * 用于：
     * - 处理状态逻辑
     * - 检查状态转换条件
     * - 更新状态相关数据
     * 
     * 调用时机：每帧（只要处于该状态）
     * 
     * @param entity 拥有该状态的实体
     * @param dt 帧时间（秒）
     * @return 下一个状态（nullptr 表示保持当前状态）
     */
    virtual IState* Update(Entity entity, float dt) = 0;
    
    /**
     * @brief 退出状态时调用
     * 
     * 用于：
     * - 清理资源
     * - 播放退出动画
     * - 重置状态变量
     * - 触发退出效果
     * 
     * 调用时机：每次离开该状态时（一次）
     * 
     * @param entity 拥有该状态的实体
     */
    virtual void Exit(Entity entity) = 0;
    
    /**
     * @brief 获取状态名称（用于调试）
     * @return 状态名称字符串
     */
    virtual const char* GetName() const = 0;
    
    /**
     * @brief 获取状态类型（用于序列化/网络同步）
     * @return 状态类型枚举
     */
    virtual StateType GetType() const = 0;
};

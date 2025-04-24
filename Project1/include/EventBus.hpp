#pragma once
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <functional>

class EventBus {
public:
    // 注册订阅：系统通过它订阅某类事件
    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> callback) {
        auto& subscribers = subscribersMap[typeid(EventType)];
        subscribers.push_back([callback](const void* eventPtr) {
            callback(*static_cast<const EventType*>(eventPtr));
            });
    }

    // 广播事件：系统触发事件通过它发出
    template<typename EventType>
    void emit(const EventType& event) {
        auto it = subscribersMap.find(typeid(EventType));
        if (it != subscribersMap.end()) {
            for (auto& func : it->second) {
                func(&event); // 传入指针，避免引用 void 错误
            }
        }
    }

private:
    // 所有事件类型的回调列表：type_index -> vector<函数>
    using BaseCallback = std::function<void(const void*)>;
    std::unordered_map<std::type_index, std::vector<BaseCallback>> subscribersMap;
};

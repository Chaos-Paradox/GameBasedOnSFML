#pragma once
#include <unordered_map>

template<typename T>
class ComponentStore {
public:
    std::unordered_map<Entity, T> data;

    void add(Entity e, const T& c) {
        data[e] = c;
    }
    bool has(Entity e) const { return data.find(e) != data.end(); }
    T& get(Entity e) { return data.at(e); }
    const T& get(Entity e) const { return data.at(e); }
    void remove(Entity e) { data.erase(e); }

    std::vector<Entity> entityList() const {
        std::vector<Entity> r;
        r.reserve(data.size());
        for (auto& kv : data) r.push_back(kv.first);
        return r;
    }
};
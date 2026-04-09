#pragma once
#include "TileMap.hpp"

class MapService {
public:
    MapService(TileMap& map) : map(map) {}

    bool isWalkable(int x, int y) const {
        return inBounds(x, y) && map.get(x, y).walkable;
    }

    bool inBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < map.getWidth() && y < map.getHeight();
    }

private:
    TileMap& map;
};
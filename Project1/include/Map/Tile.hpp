// Tile.hpp
#pragma once

struct Tile {
    int tileID = 0;          // 纹理编号或类型
    bool walkable = true;    // 是否可通行
    bool transparent = true; // 是否可视穿透
};
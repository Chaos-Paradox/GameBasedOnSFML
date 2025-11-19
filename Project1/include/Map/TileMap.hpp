#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "Map/Tile.hpp"

class TileMap {
public:
    TileMap(int width = 0, int height = 0)
        : width(width), height(height), tiles(width* height) {}

    Tile& get(int x, int y) {
        return tiles[y * width + x];
    }

    const Tile& get(int x, int y) const {
        return tiles[y * width + x];
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    bool loadFromCSV(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        std::vector<Tile> loadedTiles;
        std::string line;
        int w = 0, h = 0;

        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string cell;
            int x = 0;
            while (std::getline(ss, cell, ',')) {
                int id = std::stoi(cell);
                loadedTiles.push_back(Tile{ id, id != 1, true }); // id==1为墙不可通行
                ++x;
            }
            if (x > w) w = x;
            ++h;
        }

        width = w;
        height = h;
        tiles = std::move(loadedTiles);
        return true;
    }

private:
    int width, height;
    std::vector<Tile> tiles;
};
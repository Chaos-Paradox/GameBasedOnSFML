#pragma once
#include <SFML/Graphics.hpp>
#include "Map/TileMap.hpp"

class TileRenderSystem {
public:
    TileRenderSystem(TileMap& map, const sf::Texture& texture, int tileSize)
        : map(map), texture(texture), tileSize(tileSize) {
        buildVertices();
    }

    void draw(sf::RenderWindow& window) {
        sf::RenderStates states;
        states.texture = &texture;
        window.draw(vertices, states);
    }

    void buildVertices() {
        vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        vertices.resize(map.getWidth() * map.getHeight() * 6); // 每个 tile 6 个顶点

        for (int y = 0; y < map.getHeight(); ++y) {
            for (int x = 0; x < map.getWidth(); ++x) {
                int tileID = map.get(x, y).tileID;
                int tu = tileID % (texture.getSize().x / tileSize);
                int tv = tileID / (texture.getSize().x / tileSize);

                float tx = static_cast<float>(tu * tileSize);
                float ty = static_cast<float>(tv * tileSize);

                float px = static_cast<float>(x * tileSize);
                float py = static_cast<float>(y * tileSize);

                // 每个 tile 占 6 个顶点（两个三角形）
                sf::Vertex* tri = &vertices[(x + y * map.getWidth()) * 6];

                // Triangle 1
                tri[0].position = { px, py };
                tri[1].position = { px + tileSize, py };
                tri[2].position = { px + tileSize, py + tileSize };

                tri[0].texCoords = { tx, ty };
                tri[1].texCoords = { tx + tileSize, ty };
                tri[2].texCoords = { tx + tileSize, ty + tileSize };

                // Triangle 2
                tri[3].position = { px, py };
                tri[4].position = { px + tileSize, py + tileSize };
                tri[5].position = { px, py + tileSize };

                tri[3].texCoords = { tx, ty };
                tri[4].texCoords = { tx + tileSize, ty + tileSize };
                tri[5].texCoords = { tx, ty + tileSize };
            }
        }
    }


private:
    TileMap& map;
    const sf::Texture& texture;
    int tileSize;
    sf::VertexArray vertices;
};

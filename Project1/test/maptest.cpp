#include <SFML/Graphics.hpp>
#include "Map/TileMap.hpp"
//#include "Map/TileRenderSystem.hpp"
#include "Map/MapService.hpp"
#include <Systems/TileRenderSystem.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "TileMap Test");
    sf::Texture tileset;
    tileset.loadFromFile("../material/pictures/glass.png");

    TileMap map;
    map.loadFromCSV("map.csv");
    TileRenderSystem renderer(map, tileset, 32);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();
        renderer.draw(window);
        window.display();
    }
    return 0;
}

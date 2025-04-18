#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <iostream>


//这个纯SFML方案存在一个bug，就是松开方向键时会由于加速度的消失而进入摩擦力计算分支。原因还是这里特例出来的物理模型强耦合导致的。


using Entity = std::uint32_t;

struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

struct Acceleration {
    float ax, ay;
};

std::unordered_map<Entity, Position> positions;
std::unordered_map<Entity, Velocity> velocities;
std::unordered_map<Entity, Acceleration> accelerations;

Entity createEntity() {
    static Entity nextEntity = 0;
    return nextEntity++;
}

void addPosition(Entity entity, float x, float y) {
    positions[entity] = { x, y };
}

void addVelocity(Entity entity, float dx, float dy) {
    velocities[entity] = { dx, dy };
}

void addAcceleration(Entity entity, float ax, float ay) {
    accelerations[entity] = { ax, ay };
}


struct Obstacle {
    float x, y;
    float width, height;
};

std::vector<Obstacle> obstacles;

class MovementSystem {
public:
    void update(float deltaTime) {
        for (auto& [entity, position] : positions) {
            if (velocities.find(entity) != velocities.end() && accelerations.find(entity) != accelerations.end()) {
                auto& velocity = velocities[entity];
                auto& acceleration = accelerations[entity];

                // Update velocity with acceleration
                velocity.dx += acceleration.ax * deltaTime;
                velocity.dy += acceleration.ay * deltaTime;

                // Limit maximum speed
                float maxSpeed = 200.0f;
                float speed = std::sqrt(velocity.dx * velocity.dx + velocity.dy * velocity.dy);
                if (speed > maxSpeed) {
                    float scale = maxSpeed / speed;
                    velocity.dx *= scale;
                    velocity.dy *= scale;
                }

                // Calculate new position
                float newX = position.x + velocity.dx * deltaTime;
                float newY = position.y + velocity.dy * deltaTime;

                // Check for collisions with obstacles
                for (const auto& obstacle : obstacles) {
                    if (newX < obstacle.x + obstacle.width && newX + 20 > obstacle.x && // 20 is player width
                        newY < obstacle.y + obstacle.height && newY + 20 > obstacle.y) { // 20 is player height
                        // If collision detected, stop the player from moving
                        velocity.dx = 0;
                        velocity.dy = 0;
                        break;
                    }
                }

                // Update position with velocity
                position.x += velocity.dx * deltaTime;
                position.y += velocity.dy * deltaTime;

                // Apply friction when no acceleration
                if (acceleration.ax == 0) {
                    velocity.dx *= 0.9f;
                }
                if (acceleration.ay == 0) {
                    velocity.dy *= 0.9f;
                }
            }
        }
    }
};

class RenderSystem {
public:
    void render(sf::RenderWindow& window) {
        for (const auto& [entity, position] : positions) {
            sf::CircleShape shape(10);
            shape.setPosition(sf::Vector2f(position.x, position.y));
            shape.setFillColor(sf::Color::Green);
            window.draw(shape);
        }

        // Draw obstacles
        for (const auto& obstacle : obstacles) {
            sf::RectangleShape rect(sf::Vector2f(obstacle.width, obstacle.height));
            rect.setPosition(sf::Vector2f(obstacle.x, obstacle.y));
            rect.setFillColor(sf::Color::Red);
            window.draw(rect);
        }
    }
};



class CameraSystem {
public:
    void update(sf::View& view, const Position& playerPosition) {
        view.setCenter({ playerPosition.x, playerPosition.y });
    }
};


void only_sfml() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Camera Follow with Obstacles");
    sf::View view(sf::FloatRect({ 0, 0 }, { 800, 600 }));

    // Initialize player
    Entity player = createEntity();
    addPosition(player, 100.0f, 100.0f);
    addVelocity(player, 0.0f, 0.0f);
    addAcceleration(player, 0.0f, 0.0f);

    // Initialize obstacles
    obstacles.push_back({ 300.0f, 200.0f, 50.0f, 50.0f });
    obstacles.push_back({ 400.0f, 300.0f, 100.0f, 50.0f });
    obstacles.push_back({ 500.0f, 400.0f, 50.0f, 100.0f });

    MovementSystem movementSystem;
    CameraSystem cameraSystem;
    RenderSystem renderSystem;

    sf::Clock clock;

    while (window.isOpen()) {
        //sf::Event event;
        //while (window.pollEvent(event)) {
        // 
        // Reset acceleration
        accelerations[player].ax = 0.0f;
        accelerations[player].ay = 0.0f;
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            //else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            //    // 按键事件
            //    if (keyPressed->scancode == sf::Keyboard::Scancode::Left) {
            //        std::cout << "left" << std::endl;
            //        accelerations[player].ax = -300.0f;
            //        break;
            //    }
            //    if (keyPressed->scancode == sf::Keyboard::Scancode::Right) {
            //        accelerations[player].ax = 300.0f;
            //        break;
            //    }
            //    if (keyPressed->scancode == sf::Keyboard::Scancode::Up) {
            //        accelerations[player].ay = -300.0f;
            //        break;
            //    }
            //    if (keyPressed->scancode == sf::Keyboard::Scancode::Down) {
            //        accelerations[player].ay = 300.0f;
            //        break;
            //    }
            //}
        }

        // Apply acceleration based on input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            accelerations[player].ax = -300.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            accelerations[player].ax = 300.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            accelerations[player].ay = -300.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            accelerations[player].ay = 300.0f;
        }

        float deltaTime = clock.restart().asSeconds();

        movementSystem.update(deltaTime);
        cameraSystem.update(view, positions[player]);

        window.clear();
        window.setView(view); // Apply updated view before rendering
        renderSystem.render(window);
        window.display();
    }


}



int main() {
    only_sfml();

    return 0;
}


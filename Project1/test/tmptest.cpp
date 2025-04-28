#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

struct Vec3 {
    float x, y, z;
};

Vec3 rotateY(const Vec3& p, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return {
        p.x * cosA - p.z * sinA,
        p.y,
        p.x * sinA + p.z * cosA
    };
}

Vec3 rotateX(const Vec3& p, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    return {
        p.x,
        p.y * cosA - p.z * sinA,
        p.y * sinA + p.z * cosA
    };
}

// 简单的透视投影
sf::Vector2f project(const Vec3& p, float fov, float screenWidth, float screenHeight) {
    float scale = fov / (fov + p.z);
    return {
        p.x * scale * screenWidth / 2 + screenWidth / 2,
        -p.y * scale * screenHeight / 2 + screenHeight / 2
    };
}

int main() {
    sf::RenderWindow window(sf::VideoMode({ 1000, 1000 }), "Simple 3D Cube with SFML");

    std::vector<Vec3> cube = {
        {-1, -1, -1}, {1, -1, -1}, {1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, {1, -1,  1}, {1,  1,  1}, {-1,  1,  1}
    };

    // 立方体的边
    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    float angle = 0.f;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        angle += 0.0002f;

        window.clear(sf::Color::Black);

        sf::VertexArray lines(sf::PrimitiveType::Lines);

        for (auto& edge : edges) {
            Vec3 p1 = cube[edge[0]];
            Vec3 p2 = cube[edge[1]];

            // 旋转
            p1 = rotateY(rotateX(p1, angle), angle * 0.5f);
            p2 = rotateY(rotateX(p2, angle), angle * 0.5f);

            // 投影
            sf::Vector2f point1 = project(p1, 3.0f, 250, 250);
            sf::Vector2f point2 = project(p2, 3.0f, 250, 250);

            // 添加到VertexArray
            lines.append(sf::Vertex(point1, sf::Color::White));
            lines.append(sf::Vertex(point2, sf::Color::White));
        }
        window.setView(sf::View({ 0,0 }, {1000,1000}));
        window.draw(lines);
        window.display();
    }

    return 0;
}

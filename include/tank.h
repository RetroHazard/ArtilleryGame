#pragma once
#include <SFML/Graphics.hpp>

class Tank {
public:
    Tank(const sf::Vector2f& position, float startAngle, bool isCPU);

    void draw(sf::RenderWindow& window) const;
    void setPosition(const sf::Vector2f& newPos);
    void setAngle(float newAngle);
    void adjustAngle(float delta);

    sf::Vector2f getPosition() const;
    float getAngle() const;
    bool isCPUControlled() const;
    sf::FloatRect getBounds() const;

private:
    static constexpr float TANK_SIZE = 40.0f;
    static constexpr float BARREL_LENGTH = 30.0f;
    static constexpr float BARREL_WIDTH = 4.0f;

    sf::Vector2f position;
    float angle;
    bool cpu;

    sf::RectangleShape body;
    sf::RectangleShape barrel;

    void updateBarrelRotation();
};

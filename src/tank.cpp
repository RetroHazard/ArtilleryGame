#include "../include/tank.h"
#include <cmath>

Tank::Tank(const sf::Vector2f& startPos, float startAngle, bool isCPU)
    : position(startPos)
    , angle(startAngle)
    , cpu(isCPU) {

    // Setup tank body
    body.setSize(sf::Vector2f(TANK_SIZE, TANK_SIZE));
    body.setOrigin(TANK_SIZE/2, TANK_SIZE/2);
    body.setPosition(position);
    body.setFillColor(cpu ? sf::Color(200, 0, 0) : sf::Color(0, 200, 0));

    // Setup tank barrel
    barrel.setSize(sf::Vector2f(BARREL_LENGTH, BARREL_WIDTH));
    barrel.setOrigin(0, BARREL_WIDTH/2);
    barrel.setPosition(position);
    barrel.setFillColor(sf::Color(50, 50, 50));

    updateBarrelRotation();
}

void Tank::draw(sf::RenderWindow& window) const {
    window.draw(body);
    window.draw(barrel);
}

void Tank::setPosition(const sf::Vector2f& newPos) {
    position = newPos;
    body.setPosition(position);
    barrel.setPosition(position);
    updateBarrelRotation();
}

void Tank::setAngle(float newAngle) {
    angle = newAngle;
    updateBarrelRotation();
}

void Tank::adjustAngle(float delta) {
    angle = std::clamp(angle + delta, 0.0f, 180.0f);
    updateBarrelRotation();
}

sf::Vector2f Tank::getPosition() const {
    return position;
}

float Tank::getAngle() const {
    return angle;
}

bool Tank::isCPUControlled() const {
    return cpu;
}

sf::FloatRect Tank::getBounds() const {
    return body.getGlobalBounds();
}

void Tank::updateBarrelRotation() {
    barrel.setRotation(-angle);
}

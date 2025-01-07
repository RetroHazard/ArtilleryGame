#pragma once
#include <SFML/Graphics.hpp>
#include <random>
#include <string>
#include <memory>
#include "Tank.h"
#include "Terrain.h"
#include "Menu.h"

class Game {
public:
    Game();
    void run();

private:
    // Window constants
    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;
    static constexpr int TURN_TIME = 600; // 10 seconds at 60 FPS
    static constexpr float GRAVITY = 981.0f;
    static constexpr float POWER_SPEED = 1.0f;  // Speed of power oscillation

    // Core SFML components
    sf::RenderWindow window;
    bool isRunning;
    enum class GameState { Menu, Playing };
    GameState currentState;

    // Font and text elements
    sf::Font gameFont;
    sf::Text timerText;

    // Game objects
    std::unique_ptr<Menu> menu;
    std::unique_ptr<Tank> playerTank;
    std::unique_ptr<Tank> cpuTank;
    std::unique_ptr<Terrain> terrain;

    // Projectile properties
    sf::CircleShape projectile;
    sf::Vector2f projectileVelocity;
    bool isShooting;
    Tank* currentShootingTank{};

    struct ShotData {
        float angle;
        float power;
        sf::Vector2f impactPoint;
        bool wasClose;
    };
    std::vector<ShotData> previousShots;
    float learningRate = 0.2f;
    float integralError = 0.0f;
    float lastError = 0.0f;

    // Power meter properties
    float power;
    float powerDirection;
    float lastPlayerPower;

    // Game state
    int turnTimer;
    bool playerTurn;

    // Random number generation
    std::random_device rd;
    std::mt19937 rng;

    // Game functions
    void handleInput();
    void update(sf::Time deltaTime);
    void render();
    void initializeGame();
    void shoot(const Tank& tank);
    void updateProjectile(sf::Time deltaTime);
    void checkCollisions();
    void switchTurn();
    void handleCPUTurn();

    // Utility functions
    float generateRandomFloat(float min, float max);
    int generateRandomInt(int min, int max);
};

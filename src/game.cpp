#include "../include/game.h"
#include <cmath>

Game::Game()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Artillery Game")
    , isRunning(true)
    , currentState(GameState::Menu)
    , isShooting(false)
    , power(0.0f)
    , powerDirection(1.0f)
    , turnTimer(TURN_TIME)
    , playerTurn(true)
    , lastPlayerPower(0.0f)
    , rng(rd()) {

    window.setFramerateLimit(60);
    initializeGame();
}

void Game::initializeGame() {
    // Initialize menu
    menu = std::make_unique<Menu>(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));

    // Initialize terrain
    terrain = std::make_unique<Terrain>(WINDOW_WIDTH, WINDOW_HEIGHT);
    terrain->generate();

    // Initialize tanks with random positions
    float playerX = generateRandomFloat(50.f, 200.f);
    float cpuX = generateRandomFloat(WINDOW_WIDTH - 200.f, WINDOW_WIDTH - 50.f);

    playerTank = std::make_unique<Tank>(
        sf::Vector2f(playerX, terrain->getHeightAt(playerX)),
        45.f,
        false
    );

    cpuTank = std::make_unique<Tank>(
        sf::Vector2f(cpuX, terrain->getHeightAt(cpuX)),
        135.f,
        true
    );

    // Initialize projectile and reset projectile state
    projectile.setRadius(5.f);
    projectile.setFillColor(sf::Color::Red);
    projectile.setOrigin(5.f, 5.f);
    projectile.setPosition(-100.f, -100.f); // Move projectile off-screen
    projectileVelocity = sf::Vector2f(0.f, 0.f);
    isShooting = false;

    // Load font for timer
    if (!gameFont.loadFromFile("resources/fonts/arial.ttf")) {
        throw std::runtime_error("Failed to load font");
    }

    // Setup timer text
    timerText.setFont(gameFont);
    timerText.setCharacterSize(30);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(WINDOW_WIDTH / 2 - 50, 10);

    // Randomize first turn
    playerTurn = (generateRandomInt(0, 1) == 0);
    turnTimer = TURN_TIME;
    currentShootingTank = nullptr;

    // Reset power settings
    power = 0.0f;
    powerDirection = 1.0f;
    lastPlayerPower = 0.0f;
}

void Game::run() {
    sf::Clock clock;

    while (isRunning && window.isOpen()) {
        sf::Time deltaTime = clock.restart();

        handleInput();
        update(deltaTime);
        render();
    }
}

void Game::handleInput() {
    static bool wasSpacePressed = false; // Track previous space state

    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
            isRunning = false;
            return;
        }

        if (currentState == GameState::Menu) {
            menu->handleInput(sf::Vector2f(
                sf::Mouse::getPosition(window)));

            if (menu->wasItemClicked(0)) { // Start Game
                currentState = GameState::Playing;
            }
            else if (menu->wasItemClicked(1)) { // Quit
                window.close();
                isRunning = false;
            }
        }
        else {
            if (playerTurn && !isShooting) {
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                        case sf::Keyboard::Up:
                            playerTank->adjustAngle(1.0f);
                            break;
                        case sf::Keyboard::Down:
                            playerTank->adjustAngle(-1.0f);
                            break;
                    }
                }

                // Handle shot on space release
                if (event.type == sf::Event::KeyReleased &&
                    event.key.code == sf::Keyboard::Space) {
                    lastPlayerPower = power;  // Store the power used
                    shoot(*playerTank);
                    wasSpacePressed = false;
                }
            }
        }
    }

    // Handle power meter
    if (playerTurn && !isShooting) {
        bool isSpacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

        // Reset power only when space is first pressed
        if (isSpacePressed && !wasSpacePressed) {
            power = 0.0f;
            powerDirection = 1.0f;
        }

        // Update power while space is held
        if (isSpacePressed) {
            power += POWER_SPEED * powerDirection;

            // Reverse direction at limits
            if (power >= 100.0f) {
                power = 100.0f;
                powerDirection = -1.0f;
            } else if (power <= 0.0f) {
                power = 0.0f;
                powerDirection = 1.0f;
            }
        }

        wasSpacePressed = isSpacePressed;
    }
}

void Game::update(sf::Time deltaTime) {
    if (currentState != GameState::Playing) return;

    if (isShooting) {
        updateProjectile(deltaTime);
        checkCollisions();
    }

    if (playerTurn) {
        turnTimer--;
        if (turnTimer <= 0) {
            switchTurn();
        }
    }
    else {
        handleCPUTurn();
    }
}

void Game::updateProjectile(sf::Time deltaTime) {
    projectileVelocity.y += GRAVITY * deltaTime.asSeconds();
    sf::Vector2f pos = projectile.getPosition();
    pos += projectileVelocity * deltaTime.asSeconds();
    projectile.setPosition(pos);
}

void Game::checkCollisions() {
    sf::Vector2f pos = projectile.getPosition();

    // Check terrain collision
    if (terrain->isCollision(pos)) {
        if (!playerTurn) {
            // Record CPU shot data
            sf::Vector2f playerPos = playerTank->getPosition();
            float distance = std::sqrt(
                std::pow(pos.x - playerPos.x, 2) +
                std::pow(pos.y - playerPos.y, 2)
            );

            ShotData shot;
            shot.angle = cpuTank->getAngle();
            shot.power = power;
            shot.impactPoint = pos;
            shot.wasClose = distance < 50.f; // Consider shots within 100 pixels "close"

            previousShots.push_back(shot);
            if (previousShots.size() > 3) {
                previousShots.erase(previousShots.begin());
            }
        }

        terrain->deform(pos, 20.f);
        isShooting = false;
        currentShootingTank = nullptr;
        switchTurn();
        return;
    }

    // Check tank collisions, excluding the shooting tank
    Tank* targetTank = (currentShootingTank == playerTank.get()) ? cpuTank.get() : playerTank.get();
    if (projectile.getGlobalBounds().intersects(targetTank->getBounds())) {
        currentState = GameState::Menu;
        initializeGame();
        return;
    }

    // Check if projectile is off-screen
    if (pos.x < 0 || pos.x > WINDOW_WIDTH || pos.y > WINDOW_HEIGHT) {
        isShooting = false;
        currentShootingTank = nullptr;
        switchTurn();
    }
}

void Game::shoot(const Tank& tank) {
    projectile.setPosition(tank.getPosition());
    float radians = tank.getAngle() * 3.14159f / 180.f;

    float powerMultiplier = 15.0f;
    projectileVelocity = sf::Vector2f(
        std::cos(radians) * power * powerMultiplier,
        -std::sin(radians) * power * powerMultiplier
    );

    isShooting = true;
    currentShootingTank = (playerTurn ? playerTank.get() : cpuTank.get());
}

void Game::handleCPUTurn() {
    if (turnTimer == TURN_TIME - 10) {
        float targetAngle, targetPower;
        sf::Vector2f playerPos = playerTank->getPosition();
        sf::Vector2f cpuPos = cpuTank->getPosition();

        // Calculate distance and height difference
        float distanceX = playerPos.x - cpuPos.x;
        float distanceY = playerPos.y - cpuPos.y;
        float directDistance = std::sqrt(distanceX * distanceX + distanceY * distanceY);

        // Initial shot or reset strategy
        if (previousShots.empty()) {
            // Randomly choose between direct or high arc for initial shot
            bool useHighArc = (generateRandomInt(0, 1) == 1);

            if (useHighArc) {
                targetAngle = generateRandomFloat(140.0f, 180.0f);  // High arc
                targetPower = directDistance / 5.0f;  // More power for high arc
            } else {
                targetAngle = generateRandomFloat(0.0f, 140.0f);  // Direct shot
                targetPower = directDistance / 8.0f;  // Less power for direct shot
            }
        } else {
            const auto& lastShot = previousShots.back();

            if (lastShot.impactPoint.x < playerPos.x) {
                // Hit terrain or fell short - try higher arc
                if (lastShot.angle < 145.0f) {
                    // Current angle too low, switch to high arc strategy
                    targetAngle = generateRandomFloat(150.0f, 165.0f);
                    targetPower = lastShot.power + 15.0f;
                } else {
                    // Already using high arc, increase both
                    targetAngle = lastShot.angle + 5.0f;
                    targetPower = lastShot.power + 10.0f;
                }
            } else {
                // Overshot the target
                if (lastShot.impactPoint.y < playerPos.y) {
                    // Too high, reduce angle but maintain arc strategy
                    targetAngle = lastShot.angle - 5.0f;
                    targetPower = lastShot.power - 5.0f;
                } else {
                    // Too far but good height, reduce power
                    targetAngle = lastShot.angle;
                    targetPower = lastShot.power - 10.0f;
                }
            }

            // Occasionally try completely different approach if missing repeatedly
            if (previousShots.size() >= 3) {
                bool allShortShots = true;
                for (const auto& shot : previousShots) {
                    if (shot.impactPoint.x >= playerPos.x) {
                        allShortShots = false;
                        break;
                    }
                }

                if (allShortShots) {
                    // Switch to high arc strategy
                    targetAngle = generateRandomFloat(150.0f, 165.0f);
                    targetPower = directDistance / 5.0f;
                }
            }
        }

        // Add small random variations to prevent getting stuck
        targetAngle += generateRandomFloat(-2.0f, 2.0f);
        targetPower += generateRandomFloat(-3.0f, 3.0f);

        cpuTank->setAngle(targetAngle);
        power = targetPower;
        shoot(*cpuTank);
    }

    turnTimer--;
    if (turnTimer <= 0) {
        switchTurn();
    }
}

void Game::render() {
    window.clear(sf::Color(135, 206, 235)); // Sky blue

    if (currentState == GameState::Menu) {
        menu->draw(window);
    }
    else {
        terrain->draw(window);
        playerTank->draw(window);
        cpuTank->draw(window);

        if (isShooting) {
            window.draw(projectile);
        }

        // Draw power meter when charging
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) &&
            !isShooting && playerTurn) {
            // Draw the power meter background
            sf::RectangleShape powerMeter(sf::Vector2f(200, 20));
            powerMeter.setPosition(10, 10);
            powerMeter.setFillColor(sf::Color(50, 50, 50));
            window.draw(powerMeter);

            // Draw the current power level
            sf::RectangleShape currentPower(sf::Vector2f(power * 2, 20));
            currentPower.setPosition(10, 10);
            currentPower.setFillColor(sf::Color::Red);
            window.draw(currentPower);

            // Draw the previous power indicator line
            if (lastPlayerPower > 0) {
                sf::RectangleShape previousPower(sf::Vector2f(2, 25));
                previousPower.setPosition(10 + (lastPlayerPower * 2), 7.5f);
                previousPower.setFillColor(sf::Color::Yellow);
                window.draw(previousPower);
            }
        }

        // Update and draw timer
        int seconds = turnTimer / 60;
        timerText.setString(std::to_string(seconds));
        window.draw(timerText);
    }

    window.display();
}

void Game::switchTurn() {
    playerTurn = !playerTurn;
    turnTimer = TURN_TIME;
    currentShootingTank = nullptr;
    power = 0.0f;
    powerDirection = 1.0f;
}

float Game::generateRandomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

int Game::generateRandomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

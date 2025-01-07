#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Terrain {
public:
    Terrain(int width, int height);

    void generate();
    void draw(sf::RenderWindow& window) const;
    void deform(const sf::Vector2f& impact, float radius);
    float getHeightAt(float x) const;
    bool isCollision(const sf::Vector2f& point) const;

private:
    static constexpr float SMOOTHING = 0.1f;
    static constexpr float BASE_HEIGHT = 300.0f;
    static constexpr float HEIGHT_VARIANCE = 50.0f;
    // Add new smoothing parameters
    static constexpr float SMOOTHING_FACTOR = 0.2f;
    static constexpr int SMOOTHING_PASSES = 3;
    static constexpr float BASE_HEIGHT_VARIATION = 100.0f;

    int width;
    int height;
    std::vector<float> heights;
    sf::VertexArray terrain;

    // Add helper methods
    void smoothTerrain();
    float generateSmoothNoise(int x) const;
    void applyHeightGradient();

    void updateVertexArray();
    float smoothNoise(float x) const;
};

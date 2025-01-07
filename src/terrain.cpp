#include "../include/terrain.h"
#include <random>
#include <cmath>

Terrain::Terrain(int w, int h)
    : width(w)
    , height(h)
    , heights(w)
    , terrain(sf::TriangleStrip, w * 2) {
}

void Terrain::generate() {
    // Use random seed for each generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> noiseDist(-1.0f, 1.0f);

    // Generate random control points
    const int NUM_CONTROL_POINTS = 8;
    std::vector<float> controlPoints(NUM_CONTROL_POINTS);
    for(int i = 0; i < NUM_CONTROL_POINTS; ++i) {
        // Generate heights between 30% and 70% of screen height
        controlPoints[i] = height * (0.5f + noiseDist(gen) * 0.2f);
    }

    // Interpolate between control points for smooth terrain
    for(int x = 0; x < width; ++x) {
        float progress = static_cast<float>(x) / width;
        float scaledProgress = progress * (NUM_CONTROL_POINTS - 1);
        int index = static_cast<int>(scaledProgress);
        float t = scaledProgress - index;

        // Ensure we don't access beyond array bounds
        if(index >= NUM_CONTROL_POINTS - 1) {
            heights[x] = controlPoints[NUM_CONTROL_POINTS - 1];
            continue;
        }

        // Cubic interpolation between points
        float h0 = (index > 0) ? controlPoints[index - 1] : controlPoints[0];
        float h1 = controlPoints[index];
        float h2 = controlPoints[index + 1];
        float h3 = (index < NUM_CONTROL_POINTS - 2) ? controlPoints[index + 2] : h2;

        // Catmull-Rom spline interpolation
        float t2 = t * t;
        float t3 = t2 * t;
        heights[x] = ((-0.5f * h0 + 1.5f * h1 - 1.5f * h2 + 0.5f * h3) * t3 +
                     (h0 - 2.5f * h1 + 2.0f * h2 - 0.5f * h3) * t2 +
                     (-0.5f * h0 + 0.5f * h2) * t +
                     h1);
    }

    // Apply additional smoothing
    for(int i = 0; i < SMOOTHING_PASSES; ++i) {
        smoothTerrain();
    }

    updateVertexArray();
}

void Terrain::smoothTerrain() {
    std::vector<float> smoothedHeights = heights;

    // Apply moving average smoothing
    for(int i = 1; i < width - 1; ++i) {
        smoothedHeights[i] = heights[i-1] * SMOOTHING_FACTOR +
                            heights[i] * (1 - 2 * SMOOTHING_FACTOR) +
                            heights[i+1] * SMOOTHING_FACTOR;
    }

    heights = smoothedHeights;
}

float Terrain::generateSmoothNoise(int x) const {
    // Generate smooth noise using simple interpolation
    float noise = std::sin(x * 0.05f) * 0.3f +
                  std::sin(x * 0.02f) * 0.7f;
    return noise;
}

void Terrain::draw(sf::RenderWindow& window) const {
    window.draw(terrain);
}

void Terrain::deform(const sf::Vector2f& impact, float radius) {
    int center = static_cast<int>(impact.x);
    int start = std::max(0, center - static_cast<int>(radius));
    int end = std::min(width - 1, center + static_cast<int>(radius));

    for(int i = start; i <= end; ++i) {
        float distance = std::abs(i - center);
        float factor = 1.0f - (distance / radius);
        if(factor > 0) {
            heights[i] += 20.0f * factor;
        }
    }

    updateVertexArray();
}

float Terrain::getHeightAt(float x) const {
    int index = static_cast<int>(x);
    if(index < 0) return heights[0];
    if(index >= width) return heights[width-1];
    return heights[index];
}

bool Terrain::isCollision(const sf::Vector2f& point) const {
    if(point.x < 0 || point.x >= width) return false;
    return point.y >= getHeightAt(point.x);
}

void Terrain::updateVertexArray() {
    for(int i = 0; i < width; ++i) {
        // Top vertex
        terrain[i*2].position = sf::Vector2f(i, heights[i]);
        terrain[i*2].color = sf::Color(34, 139, 34); // Forest green

        // Bottom vertex
        terrain[i*2+1].position = sf::Vector2f(i, height);
        terrain[i*2+1].color = sf::Color(139, 69, 19); // Saddle brown
    }
}

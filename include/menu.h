#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Menu {
public:
    Menu(const sf::Vector2f& windowSize);

    void draw(sf::RenderWindow& window) const;
    void handleInput(const sf::Vector2f& mousePos);
    void update();
    bool isItemSelected(int index) const;
    bool wasItemClicked(int index) const;

private:
    struct MenuItem {
        sf::RectangleShape shape;
        sf::Text text;
        bool selected;
        bool clicked;
    };

    std::vector<MenuItem> items;
    sf::Font font;

    void loadFont();
    void createMenuItems();
    void updateSelections(const sf::Vector2f& mousePos);
};

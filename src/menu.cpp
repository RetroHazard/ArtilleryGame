#include "../include/menu.h"
#include <stdexcept>

Menu::Menu(const sf::Vector2f& windowSize) {
    loadFont();
    createMenuItems();

    // Center menu items
    float yPos = windowSize.y / 3;
    for(auto& item : items) {
        item.shape.setPosition(
            (windowSize.x - item.shape.getSize().x) / 2,
            yPos
        );

        // Center text in button
        sf::FloatRect textBounds = item.text.getLocalBounds();
        item.text.setPosition(
            item.shape.getPosition().x + (item.shape.getSize().x - textBounds.width) / 2,
            item.shape.getPosition().y + (item.shape.getSize().y - textBounds.height) / 2
        );

        yPos += item.shape.getSize().y + 20.f;
    }
}

void Menu::loadFont() {
    if(!font.loadFromFile("resources/fonts/arial.ttf")) {
        throw std::runtime_error("Failed to load font");
    }
}

void Menu::createMenuItems() {
    std::vector<std::string> menuText = {"Start Game", "Quit"};
    sf::Vector2f buttonSize(200.f, 50.f);

    for(const auto& text : menuText) {
        MenuItem item;

        item.shape.setSize(buttonSize);
        item.shape.setFillColor(sf::Color(200, 200, 200));
        item.shape.setOutlineColor(sf::Color::Black);
        item.shape.setOutlineThickness(2.f);

        item.text.setFont(font);
        item.text.setString(text);
        item.text.setCharacterSize(24);
        item.text.setFillColor(sf::Color::Black);

        item.selected = false;
        item.clicked = false;

        items.push_back(item);
    }
}

void Menu::draw(sf::RenderWindow& window) const {
    for(const auto& item : items) {
        window.draw(item.shape);
        window.draw(item.text);
    }
}

void Menu::handleInput(const sf::Vector2f& mousePos) {
    updateSelections(mousePos);

    if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        for(auto& item : items) {
            if(item.selected) {
                item.clicked = true;
            }
        }
    }
}

void Menu::updateSelections(const sf::Vector2f& mousePos) {
    for(auto& item : items) {
        item.selected = item.shape.getGlobalBounds().contains(mousePos);
        if(!item.selected) {
            item.clicked = false;
        }
    }
}

bool Menu::isItemSelected(int index) const {
    if(index >= 0 && index < items.size()) {
        return items[index].selected;
    }
    return false;
}

bool Menu::wasItemClicked(int index) const {
    if(index >= 0 && index < items.size()) {
        return items[index].clicked;
    }
    return false;
}

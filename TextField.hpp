#pragma once
#include <SFML/Graphics.hpp>

class TextField {
public:
	TextField(const sf::Font& font, sf::Vector2f position, sf::Vector2f size, int max, sf::Color color, sf::Color border);

	void handleInput(const sf::Event& event, sf::RenderWindow& window);
	void render(sf::RenderWindow& window);

	int getValue() const;
	void setValue(int value);

	bool isHovered(const sf::Vector2i& mousePos) const;

private:
	sf::RectangleShape shape;
	sf::Text text;
	std::string buffer;
	bool focused = false;
	int max;
};
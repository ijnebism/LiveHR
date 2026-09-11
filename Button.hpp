#pragma once
#include <SFML/Graphics.hpp>

class Button {
public:
	Button(sf::Vector2f pos, sf::Vector2f size, sf::Color idleColor, sf::Color hoverColor, sf::Color border, sf::Text text);

	void update(const sf::Vector2i& mousePos);
	bool isClicked(const sf::Vector2i& mousePos, sf::Mouse::Button targetButton, const sf::Event& event);
	void render(sf::RenderWindow& window);

	void setString(std::string newString);

private:
	sf::RectangleShape shape;
	sf::Text text;
	sf::Color idle;
	sf::Color hover;
};
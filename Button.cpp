#include "Button.hpp"

Button::Button(sf::Vector2f pos, sf::Vector2f size, sf::Color idleColor, sf::Color hoverColor, sf::Color border, sf::Text text) :
	idle(idleColor), hover(hoverColor), text(text){
	shape.setPosition(pos);
	shape.setSize(size);
	shape.setFillColor(idleColor);
	shape.setOutlineColor(border);
	shape.setOutlineThickness(2);
}

void Button::update(const sf::Vector2i& mousePos) {
	sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
	if (shape.getGlobalBounds().contains(mousePosF)) {
		shape.setFillColor(hover);
	}
	else {
		shape.setFillColor(idle);
	}
}

bool Button::isClicked(const sf::Vector2i& mousePos, sf::Mouse::Button targetButton, const sf::Event& event) {
	sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
	if (shape.getGlobalBounds().contains(mousePosF)) {
		if (event.is<sf::Event::MouseButtonReleased>()) {
			if (event.getIf<sf::Event::MouseButtonReleased>()->button == targetButton) {
				return true;
			}
		}
	}
	return false;
}

void Button::render(sf::RenderWindow& window) {
	window.draw(shape);

	auto textBounds = text.getLocalBounds();
	text.setOrigin({
		textBounds.position.x + textBounds.size.x / 2.f,
		textBounds.position.y + textBounds.size.y / 2.f
		});

	auto shapeBounds = shape.getGlobalBounds();
	text.setPosition({
		shapeBounds.position.x + shapeBounds.size.x / 2.f,
		shapeBounds.position.y + shapeBounds.size.y / 2.f
		});

	window.draw(text);
}

void Button::setString(std::string newString) {
	text.setString(newString);
}
#include <SFML/Graphics.hpp>
#include "BluetoothDevice.hpp"
#include "Scanner.hpp"
#include "Paths.hpp"
#include "Button.hpp"
#include <winrt/windows.devices.bluetooth.h>
#include <winrt/windows.devices.bluetooth.advertisement.h>
#include <winrt/windows.devices.enumeration.h>
#include <winrt/windows.devices.bluetooth.genericattributeprofile.h>
#include <winrt/Windows.Storage.Streams.h>
#include <Windows.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

using namespace winrt::Windows::Devices::Bluetooth::Advertisement;


void makeTransparentClickThrough(sf::RenderWindow& win)
{
	HWND hwnd = win.getNativeHandle();
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hwnd, &margins);
	LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

int main()
{
	Scanner scanner;
	sf::Font font;
	winrt::init_apartment();
	std::filesystem::path assetsDir = getExecutablePath() / "assets";

	sf::RenderWindow window(sf::VideoMode({800, 600}), "Heart Rate Monitor");
	HWND hwnd = window.getNativeHandle();

	COLORREF topColour = RGB(41, 53, 60);
	COLORREF textColour = RGB(230, 230, 230);

	DwmSetWindowAttribute(
		hwnd,
		DWMWA_CAPTION_COLOR,
		&topColour,
		sizeof(topColour)
	);

	DwmSetWindowAttribute(
		hwnd,
		DWMWA_TEXT_COLOR,
		&textColour,
		sizeof(textColour)
	);

	window.setFramerateLimit(60);
	sf::RenderWindow hrWindow;
	sf::Texture heartIcon;
	bool isScanning = false;
	bool hrWindowOpened = false;


	if (!font.openFromFile(assetsDir / "ArchivoBlack-Regular.ttf")) {
		return -1;
	}

	if (!heartIcon.loadFromFile(assetsDir / "heart-rate.png")) {
		return -1;
	}

	sf::Sprite heartSprite(heartIcon);

	sf::Text title(font, "Connect to a Device", 32);
	title.setFillColor(sf::Color(41, 53, 60));
	title.setPosition({ 20.f, 20.f });

	sf::Text scanText = sf::Text(font, "Start Scan", 20);
	scanText.setFillColor(sf::Color(41, 53, 60));

	Button scanBtn({ 640.f, 20.f }, { 140.f, 32.f }, sf::Color(230,230,230), sf::Color(223, 235, 246), sf::Color(41, 53, 60), scanText);


	while (window.isOpen())
	{
		auto devices = scanner.getDevices();
		std::vector<sf::Text> deviceTexts;
		std::vector<sf::Text> connectTexts;
		std::vector<Button> connectButtons;
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		const float rowHeight = 40.f;
		const float rowY = 80.f;

		for (size_t i = 0; i < devices.size(); ++i)
		{
			std::string label = devices[i].name.empty()
				? std::to_string(devices[i].address) : devices[i].name;

			deviceTexts.emplace_back(font, label, 16);
			deviceTexts.back().setFillColor(sf::Color(41, 53, 60));
			deviceTexts.back().setPosition({ 20.f, rowY + i * rowHeight });

			connectTexts.emplace_back(font, "Connect", 16);
			connectTexts.back().setFillColor(sf::Color(41, 53, 60));

			connectButtons.emplace_back(
				sf::Vector2f{ 640.f, rowY + i * rowHeight },
				sf::Vector2f{ 120.f, 32.f },
				sf::Color(230, 230, 230), sf::Color(223, 235, 246), sf::Color(41, 53, 60),
				connectTexts.back()
			);
		}

		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
			if (scanBtn.isClicked(mousePos, sf::Mouse::Button::Left, *event)) {
				scanBtn.setString(isScanning ? "Start Scan" : "Stop Scan");
				if (isScanning) {
					scanner.stopScanning();
				}
				else {
					scanner.startScanning();
				}
				isScanning = !isScanning;
			}

			for (size_t i = 0; i < connectButtons.size(); ++i) {
				if (connectButtons[i].isClicked(mousePos, sf::Mouse::Button::Left, *event)) {
					scanner.connectToDevice(devices[i].address);
				}
			}
		}

		if (scanner.isConnected() && !hrWindowOpened) {
			hrWindow.create(sf::VideoMode({ 300, 100 }), "Heart Rate", sf::Style::None);
			sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
			sf::Vector2u hrSize = hrWindow.getSize();
			makeTransparentClickThrough(hrWindow);
			hrWindow.setFramerateLimit(120);

			hrWindow.setPosition({ static_cast<int>(desktop.size.x - hrSize.x) - 100, 10 });


			hrWindowOpened = true;
		}

		if (hrWindow.isOpen()) {
			while (const std::optional hrEvent = hrWindow.pollEvent()) {
				if (hrEvent->is<sf::Event::Closed>()) hrWindow.close();
			}
		}

		window.clear(sf::Color(170, 199, 216));

		scanner.removeStaleDevice();
		scanBtn.update(mousePos);
		for (auto& btn : connectButtons) btn.update(mousePos);
		for (auto& text : deviceTexts) window.draw(text);

		window.draw(title);
		scanBtn.render(window);
		for (auto& btn : connectButtons) btn.render(window);

		window.display();

		if (hrWindow.isOpen()) {
			hrWindow.clear(sf::Color::Transparent);
			sf::Vector2u winSize = hrWindow.getSize();
			float padding = 20.f;


			float iconSize = static_cast<float>(winSize.y) - (padding * 2.f);
			sf::Vector2u texSize = heartSprite.getTexture().getSize();
			float scale = iconSize / static_cast<float>(texSize.y);
			heartSprite.setScale({ scale, scale });
			heartSprite.setPosition({ padding, padding });

			hrWindow.draw(heartSprite);

			sf::Text hrText(font, std::to_string(scanner.getLatestHeartRate()) + " bpm", 28);
			hrText.setFillColor(sf::Color::White);
			hrText.setOutlineColor(sf::Color::Black);
			hrText.setOutlineThickness(2);

			sf::FloatRect bounds = hrText.getLocalBounds();
			float textX = padding + iconSize + padding;
			float textY = (static_cast<float>(winSize.y) - bounds.size.y) / 2.f - bounds.position.y;
			hrText.setPosition({ textX, textY });


			hrWindow.draw(hrText);
			hrWindow.display();
		}
	}
	
}

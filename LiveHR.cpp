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


void makeClickThrough(HWND hwnd, bool lock)
{
	LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	if (lock) {
		exStyle |= (WS_EX_LAYERED | WS_EX_TRANSPARENT);
	}
	else {
		exStyle = (exStyle | WS_EX_LAYERED) & ~WS_EX_TRANSPARENT;
	}
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
	SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void makeTopMost(HWND hwnd, bool topmost) {
	SetWindowPos(hwnd, topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

int main() {
	Scanner scanner;
	sf::Font font;
	sf::Clock clock;
	float pulsePhase = 0.0f;
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

	bool dragging = false;
	sf::Vector2i dragOffset;


	if (!font.openFromFile(assetsDir / "ArchivoBlack-Regular.ttf")) {
		return -1;
	}

	if (!heartIcon.loadFromFile(assetsDir / "heart-rate.png")) {
		return -1;
	}
	sf::Image heartImage = heartIcon.copyToImage();

	window.setIcon(heartImage);

	sf::Sprite heartSprite(heartIcon);

	sf::Text title(font, "Connect to a Device", 32);
	title.setFillColor(sf::Color(41, 53, 60));
	title.setPosition({ 20.f, 20.f });

	sf::Text scanText = sf::Text(font, "Start Scan", 20);
	scanText.setFillColor(sf::Color(41, 53, 60));

	
	Button scanBtn({ 640.f, 20.f }, { 140.f, 32.f }, sf::Color(230,230,230), sf::Color(223, 235, 246), sf::Color(41, 53, 60), scanText);
	sf::Text pinText(font, "Pin", 20);
	pinText.setFillColor(sf::Color(41, 53, 60));
	Button pinBtn({ 640.f, 160.f }, { 120.f, 32.f }, sf::Color(230, 230, 230), sf::Color(223, 235, 246), sf::Color(41, 53, 60), pinText);

	sf::Text lockText(font, "Lock", 20);
	lockText.setFillColor(sf::Color(41, 53, 60));
	Button lockBtn({ 640.f, 120.f }, { 120.f, 32.f }, sf::Color(230, 230, 230), sf::Color(223, 235, 246), sf::Color(41, 53, 60), lockText);

	bool hrPinned = false;  
	bool hrLocked = false; 

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

			auto connectedDevice = scanner.getConnectedDevice();


			if (connectedDevice  && devices[i].address == connectedDevice.BluetoothAddress()) {
				connectTexts.emplace_back(font, "Disconnect", 16);
			}
			else {
				connectTexts.emplace_back(font, "Connect", 16);
			}

			deviceTexts.emplace_back(font, label, 16);
			deviceTexts.back().setFillColor(sf::Color(41, 53, 60));
			deviceTexts.back().setPosition({ 20.f, rowY + i * rowHeight });

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
				scanner.disconnectDevice();
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
			if (scanner.isConnected()) {
				if (pinBtn.isClicked(mousePos, sf::Mouse::Button::Left, *event)) {
					hrPinned = !hrPinned;
					pinBtn.setString(hrPinned ? "Unpin" : "Pin");
					makeTopMost(hrWindow.getNativeHandle(), hrPinned);
				}
				if (lockBtn.isClicked(mousePos, sf::Mouse::Button::Left, *event)) {
					hrLocked = !hrLocked;
					lockBtn.setString(hrLocked ? "Unlock" : "Lock");
					makeClickThrough(hrWindow.getNativeHandle(), hrLocked);
				}
			}

			for (size_t i = 0; i < connectButtons.size(); ++i) {
				if (connectButtons[i].isClicked(mousePos, sf::Mouse::Button::Left, *event)) {
					auto connectedDevice = scanner.getConnectedDevice();

					if (connectedDevice && devices[i].address == connectedDevice.BluetoothAddress())
					{
						scanner.disconnectDevice();
						hrWindow.close();
						hrWindowOpened = false;
					}
					else {
						scanner.connectToDevice(devices[i].address);
					}
				}
			}
		}

		if (scanner.isConnected() && !hrWindowOpened) {
			
			scanner.stopScanning();
			isScanning = false;
			scanBtn.setString("Start Scan");
			hrWindow.create(sf::VideoMode({ 300, 100 }), "Heart Rate", sf::Style::None);

			HWND hrHwnd = hrWindow.getNativeHandle();
			MARGINS margins = { -1 };
			DwmExtendFrameIntoClientArea(hrHwnd, &margins);

			LONG_PTR exStyle = GetWindowLongPtr(hrHwnd, GWL_EXSTYLE);
			SetWindowLongPtr(hrHwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
			SetWindowPos(hrHwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
			sf::Vector2u hrSize = hrWindow.getSize();
			hrWindow.setFramerateLimit(120);

			hrWindow.setPosition({ static_cast<int>(desktop.size.x - hrSize.x) - 100, 10 });

			hrWindowOpened = true;
		}

		if (hrWindow.isOpen()) {
			while (const std::optional hrEvent = hrWindow.pollEvent()) {
				if (hrEvent->is<sf::Event::Closed>()) {
					scanner.disconnectDevice();
					hrWindow.close();
					hrWindowOpened = false;
				}

				if (!hrLocked) {
					if (auto* pressed = hrEvent->getIf<sf::Event::MouseButtonPressed>()) {
						if (pressed->button == sf::Mouse::Button::Left) {
							dragging = true;
							dragOffset = sf::Mouse::getPosition() - hrWindow.getPosition();
						}
					}
					if (auto* released = hrEvent->getIf<sf::Event::MouseButtonReleased>()) {
						if (released->button == sf::Mouse::Button::Left) {
							dragging = false;
						}
					}
				}
			}
			if (dragging) {
				hrWindow.setPosition(sf::Mouse::getPosition() - dragOffset);
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

		if (scanner.isConnected()) {
			pinBtn.update(mousePos);
			lockBtn.update(mousePos);
			pinBtn.render(window);
			lockBtn.render(window);
		}

		window.display();

		if (hrWindow.isOpen()) {
			hrWindow.clear(sf::Color(0,0,0,0));
			sf::Vector2u winSize = hrWindow.getSize();
			float padding = 20.f;


			float iconSize = static_cast<float>(winSize.y) - (padding * 2.f);
			sf::Vector2u texSize = heartSprite.getTexture().getSize();
			float scale = iconSize / static_cast<float>(texSize.y);

			float bpm = scanner.getLatestHeartRate();

			if (bpm > 0)
			{
				float dt = clock.restart().asSeconds();

				float beatsPerSecond = bpm / 60.0f;
				pulsePhase += dt * beatsPerSecond;
				pulsePhase = std::fmod(pulsePhase, 1.0f);
				float pulse = (std::sin(pulsePhase * 2.0f * 3.14159265359f) + 1.0f) / 2.0f;

				float pulseScale = scale * (1.0f + pulse * 0.05f);

				heartSprite.setScale({ pulseScale, pulseScale });
			}

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

#include <SFML/Graphics.hpp>
#include "BluetoothDevice.hpp"
#include "Scanner.hpp"
#include "Paths.hpp"
#include "Button.hpp"
#include <winrt/windows.devices.bluetooth.h>
#include <winrt/windows.devices.bluetooth.advertisement.h>
#include <winrt/windows.devices.enumeration.h>

using namespace winrt::Windows::Devices::Bluetooth::Advertisement;

int main()
{
	Scanner scanner;
	sf::Font font;
	winrt::init_apartment();
	sf::RenderWindow window(sf::VideoMode({800, 600}), "Heart Rate Monitor");
	bool isScanning = false;

	std::filesystem::path assetsDir = getExecutablePath() / "assets";

	if (!font.openFromFile(assetsDir / "ArchivoBlack-Regular.ttf")) {
		return -1;
	}

	scanner.startScanning();

	sf::Text title(font, "Connect to a Device", 32);
	title.setFillColor(sf::Color(41, 53, 60));
	title.setPosition({ 20.f, 20.f });

	sf::Text scanText = sf::Text(font, "Start Scan", 20);
	scanText.setFillColor(sf::Color(41, 53, 60));

	Button scanBtn({ 640.f, 20.f }, { 140.f, 32.f }, sf::Color(230,230,230), sf::Color(223, 235, 246), sf::Color(41, 53, 60), scanText);


	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
			sf::Vector2i mousePos = sf::Mouse::getPosition(window);

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

			window.clear(sf::Color(170, 199, 216));
			scanner.removeStaleDevice();

			scanBtn.update(mousePos);

			window.draw(title);

			for (const auto& device : scanner.getDevices()) {
				std::string deviceInfo = "Name: " + device.name + ", Address: " + std::to_string(device.address) + ", RSSI: " + std::to_string(device.rssi);
				sf::Text text(font, deviceInfo, 16);
				text.setFillColor(sf::Color(41,53,60));
				window.draw(text);
			}
			scanBtn.render(window);

			window.display();
		}
	}
	
}

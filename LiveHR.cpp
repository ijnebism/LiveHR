#include <SFML/Graphics.hpp>
#include "BluetoothDevice.hpp"
#include "Scanner.hpp"
#include "Paths.hpp"
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

	std::filesystem::path assetsDir = getExecutablePath() / "assets";

	if (!font.openFromFile(assetsDir / "ArchivoBlack-Regular.ttf")) {
		return -1;
	}

	scanner.startScanning();

	sf::Text title(font, "Connect to a Device", 32);
	title.setPosition({ 30.f, 20.f });


	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
			window.clear(sf::Color::Black);
			scanner.removeStaleDevice();

			window.draw(title);

			for (const auto& device : scanner.getDevices()) {
				std::string deviceInfo = "Name: " + device.name + ", Address: " + std::to_string(device.address) + ", RSSI: " + std::to_string(device.rssi);
				sf::Text text(font, deviceInfo, 16);
				text.setFillColor(sf::Color::White);
				window.draw(text);
			}

			window.display();
		}
	}
	
}

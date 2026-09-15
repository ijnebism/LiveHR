#include <winrt/windows.devices.bluetooth.advertisement.h>
#include "BluetoothDevice.hpp"
#include <winrt/windows.devices.bluetooth.h>
#include <winrt/windows.foundation.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/windows.devices.bluetooth.genericattributeprofile.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <chrono>


using namespace winrt::Windows::Devices::Bluetooth::Advertisement;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

class Scanner {
public:
	Scanner();
	void startScanning();
	void stopScanning();
	void removeStaleDevice();
	winrt::fire_and_forget connectToDevice(uint64_t address);
	void disconnectDevice();
	std::vector<BluetoothDevice> getDevices() const;
	std::atomic<uint16_t> latestHeartRate{ 0 };
	std::atomic<bool> connected{ false };
	uint16_t getLatestHeartRate() const { return latestHeartRate; }
	bool isConnected() const { return connected; }
private:
	BluetoothLEAdvertisementWatcher watcher;
	std::vector<BluetoothDevice> devices;
	BluetoothLEAdvertisementFilter filter;
	winrt::Windows::Devices::Bluetooth::BluetoothLEDevice connectedDevice{ nullptr };
	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic hrCharacteristic{ nullptr };

	uint16_t parseHeartRate(winrt::Windows::Storage::Streams::IBuffer const& buffer);
};
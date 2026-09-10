#include <winrt/windows.devices.bluetooth.advertisement.h>
#include "BluetoothDevice.hpp"
#include <winrt/Windows.Foundation.Collections.h>
#include <chrono>


using namespace winrt::Windows::Devices::Bluetooth::Advertisement;

class Scanner {
public:
	Scanner();
	void startScanning();
	void stopScanning();
	void removeStaleDevice();
	std::vector<BluetoothDevice> getDevices() const;
private:
	BluetoothLEAdvertisementWatcher watcher;
	std::vector<BluetoothDevice> devices;
	BluetoothLEAdvertisementFilter filter;
};
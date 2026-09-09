#include <iostream>
#include <winrt/windows.devices.bluetooth.h>
#include <winrt/windows.devices.bluetooth.advertisement.h>
#include <winrt/windows.devices.enumeration.h>
#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt::Windows::Devices::Bluetooth::Advertisement;

int main()
{
	winrt::init_apartment();

    std::cout << "Starting Scan\n";
	
	BluetoothLEAdvertisementWatcher watcher;
	BluetoothLEAdvertisementFilter filter;

	filter.Advertisement().ServiceUuids().Append(winrt::guid(L"0000180D-0000-1000-8000-00805F9B34FB")); // Heart Rate Service UUID
	watcher.AdvertisementFilter(filter);
	watcher.ScanningMode(BluetoothLEScanningMode::Active);

	
	watcher.Received([](BluetoothLEAdvertisementWatcher const& sender, BluetoothLEAdvertisementReceivedEventArgs const& args)
		{
			std::wcout << L"Advertisement received from: " << args.BluetoothAddress() << L"\n";
		});

	watcher.Start();
	std::cout << "Press Enter to stop scanning...\n";
	std::cin.get();
}

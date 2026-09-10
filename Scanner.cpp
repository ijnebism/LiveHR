#include "Scanner.hpp"

Scanner::Scanner() {
	filter.Advertisement().ServiceUuids().Append(winrt::guid(L"0000180D-0000-1000-8000-00805F9B34FB")); // Heart Rate Service UUID
	watcher.AdvertisementFilter(filter);
	watcher.ScanningMode(BluetoothLEScanningMode::Active);
}

void Scanner::startScanning()
{
    watcher.Received(
        [this](
            BluetoothLEAdvertisementWatcher const& sender,
            BluetoothLEAdvertisementReceivedEventArgs const& args)
        {
            uint64_t address = args.BluetoothAddress();
            int rssi = args.RawSignalStrengthInDBm();
            std::string name =
                winrt::to_string(args.Advertisement().LocalName());

            auto now = std::chrono::steady_clock::now();

            for (auto& device : devices)
            {
                if (device.address == address)
                {
                    device.rssi = rssi;
                    device.lastSeen = now;
                    if (device.name.empty() && !name.empty())
                    {
                        device.name = name;
                    }
                    return;
                }
            }
            devices.push_back(
                BluetoothDevice{ name, address, rssi, now }
            );
        }
    );

    watcher.Start();
}

void Scanner::stopScanning() {
	watcher.Stop();
}

void Scanner::removeStaleDevice()
{
    auto now = std::chrono::steady_clock::now();
    devices.erase(
        std::remove_if(
            devices.begin(),
            devices.end(),
            [now](const BluetoothDevice& device)
            {
                auto elapsed =
                    std::chrono::duration_cast<std::chrono::seconds>(
                        now - device.lastSeen
                    );
                return elapsed.count() > 3;
            }
        ),
        devices.end()
    );
}

std::vector<BluetoothDevice> Scanner::getDevices() const {
    auto sortedDevices = devices;

    std::sort(
        sortedDevices.begin(),
        sortedDevices.end(),
        [](const BluetoothDevice& a, const BluetoothDevice& b) {
            return a.rssi > b.rssi;
        }
    );
    return sortedDevices;
}
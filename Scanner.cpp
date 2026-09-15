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

winrt::fire_and_forget Scanner::connectToDevice(uint64_t address) {
    connectedDevice = co_await winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(address);

    auto serviceResult = co_await connectedDevice.GetGattServicesForUuidAsync(
        winrt::guid(L"0000180D-0000-1000-8000-00805F9B34FB"));
    if (serviceResult.Services().Size() == 0) { co_return; }

    auto hrService = serviceResult.Services().GetAt(0);
    auto charResult = co_await hrService.GetCharacteristicsForUuidAsync(
        winrt::guid(L"00002A37-0000-1000-8000-00805F9B34FB"));
    if (charResult.Characteristics().Size() == 0) { co_return; }

    hrCharacteristic = charResult.Characteristics().GetAt(0);

    hrCharacteristic.ValueChanged([this](GattCharacteristic const&, GattValueChangedEventArgs const& args)
        {
            uint16_t bpm = parseHeartRate(args.CharacteristicValue());
            latestHeartRate = bpm;
        });

    auto status = co_await hrCharacteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
        GattClientCharacteristicConfigurationDescriptorValue::Notify);

    if (status == GattCommunicationStatus::Success) {
        connected = true;
    }
}

void Scanner::disconnectDevice() {
    if (connectedDevice) {
        connectedDevice.Close();
        connectedDevice = nullptr;
    }
}

uint16_t Scanner::parseHeartRate(winrt::Windows::Storage::Streams::IBuffer const& buffer)
{
    auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer);
    uint8_t flags = reader.ReadByte();
    return (flags & 0x01) ? reader.ReadUInt16() : reader.ReadByte();
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
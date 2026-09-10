#pragma once
#include <string>

struct BluetoothDevice {
	std::string name;
	uint64_t address;
	int rssi;
	std::chrono::steady_clock::time_point lastSeen;
};
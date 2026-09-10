#pragma once
#include <windows.h>
#include <filesystem>

inline std::filesystem::path getExecutablePath() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path();
}
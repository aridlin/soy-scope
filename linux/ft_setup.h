#pragma once
#include <functional>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Toolkit-neutral setup boundary: the platform controller owns all settings,
// overlay/input operations and persistence; FT owns the visible desktop UI.
namespace soy {
using SetupState = std::map<std::string, std::string>;
struct SetupBackend {
    std::function<SetupState()> state;
    std::function<void(const std::string&)> command;
    std::vector<std::string> screens;
    std::vector<uint32_t> icon; // EWMH width, height, then ARGB pixels.
};
bool openSetup(SetupBackend backend);
bool drawSetup();
void closeSetup();
}

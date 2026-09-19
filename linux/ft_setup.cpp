#define FT_IMPLEMENTATION
#include "ft.hpp"
#include "ft_setup.h"
#include <array>
#include <X11/extensions/Xrandr.h>
#include <set>
#include <cstdio>
#include <cstdlib>

namespace soy {
namespace {
SetupBackend backend;
std::map<std::string, std::array<char, 4096>> edits;
std::string feedback;
bool anchorsOpen = false, markersOpen = false;
void send(const std::string& key, const std::string& value = {}) {
    backend.command(key + (value.empty() ? "" : " " + value));
}
void checkbox(const char* label, const char* key, const SetupState& state) {
    bool value = state.at(key) == "true";
    if (ft::checkbox(label, &value)) send(key, value ? "1" : "0");
}
void slider(const char* label, const char* key, const SetupState& state, float min, float max) {
    float value = std::strtof(state.at(key).c_str(), nullptr);
    if (ft::slider_float(label, &value, min, max)) {
        char text[64]; std::snprintf(text, sizeof(text), "%.7g", double(value)); send(key, text);
    }
}
void edit(const char* label, const char* key) {
    ft::set_next_fill();
    ft::input(label, edits.at(key).data(), edits.at(key).size());
}
void copyEdit(const char* key, const std::string& value) {
    std::snprintf(edits[key].data(), edits[key].size(), "%s", value.c_str());
}
}
bool openSetup(SetupBackend value) {
    backend = std::move(value);
    const auto state = backend.state();
    for (const auto* key : {"image", "hotkey", "medal_marker_key", "medal_target_key"}) copyEdit(key, state.at(key));
    ft::Config config;
    config.title = "Soy Scope — FT";
    config.mode = ft::Mode::Gui;
    config.width = 780; config.height = 780;
    config.resizable = false;
    // Qt drives the platform controller and calls drawSetup on its timer.
    // Neither FT's idle wait nor its frame limiter may block input/timers.
    config.fps_limit = 0;
    // FT currently exposes no Linux native icon/placement API. Discover only
    // the newly created window, then apply standard X11 desktop metadata.
    Display* display = XOpenDisplay(nullptr);
    if (!display) return false;
    const Window root = DefaultRootWindow(display);
    auto children = [&] {
        Window parent, returnedRoot, *windows = nullptr; unsigned count = 0;
        std::set<Window> result;
        if (XQueryTree(display, root, &returnedRoot, &parent, &windows, &count))
            for (unsigned i = 0; i < count; ++i) result.insert(windows[i]);
        if (windows) XFree(windows);
        return result;
    };
    const auto previous = children();
    if (!ft::create_window(config)) { XCloseDisplay(display); return false; }
    for (Window window : children()) {
        if (previous.count(window)) continue;
        char* name = nullptr;
        const bool ours = XFetchName(display, window, &name) && name && std::string(name) == config.title;
        if (name) XFree(name);
        if (!ours) continue;
        XClassHint hint{const_cast<char*>("soy-scope"), const_cast<char*>("soy-scope")};
        XSetClassHint(display, window, &hint);
        std::vector<unsigned long> icon(backend.icon.begin(), backend.icon.end());
        if (!icon.empty()) XChangeProperty(display, window, XInternAtom(display, "_NET_WM_ICON", False),
            XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char*>(icon.data()), int(icon.size()));
        const unsigned long pid = getpid();
        XChangeProperty(display, window, XInternAtom(display, "_NET_WM_PID", False), XA_CARDINAL,
            32, PropModeReplace, reinterpret_cast<const unsigned char*>(&pid), 1);
        int count = 0;
        auto* monitors = XRRGetMonitors(display, root, True, &count);
        if (monitors && count > 0) {
            int chosen = 0;
            for (int i = 0; i < count; ++i) if (monitors[i].primary) chosen = i;
            const auto& m = monitors[chosen];
            XMoveWindow(display, window, m.x + std::max(0, (m.width - config.width) / 2),
                m.y + std::max(0, (m.height - config.height) / 2));
        }
        if (monitors) XRRFreeMonitors(monitors);
    }
    XFlush(display);
    XCloseDisplay(display);
    return true;
}
bool drawSetup() {
    ft::request_redraw();
    if (!ft::pump()) return false;
    auto state = backend.state();
    ft::begin();
    ft::text("Soy Scope");
    ft::text_wrapped("Arm the scope, then hold your trigger button. The pointing image stays transparent and click-through.");
    ft::separator();
    ft::scroll_area("Scope settings", 430, [&] {
        checkbox("Armed", "armed", state);
        checkbox("Use a different mouse button", "custom_mouse_button", state);
        if (state.at("custom_mouse_button") == "true") {
            const char* buttons[] = {"Left", "Middle", "Side X1", "Side X2"};
            int selected = std::atoi(state.at("mouse_button").c_str());
            if (ft::dropdown("Trigger button", buttons, 4, &selected)) send("mouse_button", std::to_string(selected));
        } else ft::text("Trigger: right mouse button");
        edit("Toggle shortcut", "hotkey");
        if (ft::button("Apply shortcut")) send("hotkey", edits["hotkey"].data());
        ft::text("Shortcut examples: F8, Ctrl+Alt+S, Shift+F9");
        std::vector<const char*> screens;
        int selected = 0;
        for (size_t i = 0; i < backend.screens.size(); ++i) {
            screens.push_back(backend.screens[i].empty() ? "Primary screen (automatic)" : backend.screens[i].c_str());
            if (backend.screens[i] == state.at("screen")) selected = int(i);
        }
        if (!screens.empty() && ft::dropdown("Overlay monitor", screens.data(), int(screens.size()), &selected)
            && selected >= 0 && selected < int(screens.size())) send("screen", backend.screens[selected]);
        slider("Delay (ms)", "delay", state, 0, 5000);
        slider("Width (physical pixels)", "width", state, 1, 5000);
        slider("Opacity (%)", "opacity", state, 0, 100);
        ft::separator();
        edit("Image path", "image");
        ft::row({1, 1, 1}, [&] {
            if (ft::button("Load image")) {
                send("image", edits["image"].data());
                feedback = backend.state().at("image") == edits["image"].data() ? "Image loaded" : "Could not load that image";
            }
            if (ft::button("Browse image")) {
                ft::FileFilter filters[] = {{"Images", "*.webp;*.png;*.jpg;*.jpeg;*.bmp"}};
                auto path = ft::open_file_dialog("Choose scope image", filters, 1);
                if (!path.empty()) { send("image", path); copyEdit("image", backend.state().at("image")); }
            }
            if (ft::button("Use original image")) { send("image", "embedded"); copyEdit("image", "embedded"); }
        });
        if (ft::collapsing_header("Fingertip alignment", &anchorsOpen)) {
            slider("Anchor X (%)", "anchorX", state, 0, 100);
            slider("Anchor Y (%)", "anchorY", state, 0, 100);
            if (ft::button("Reset anchor")) send("reset-anchor");
        }
        if (ft::collapsing_header("Recorder markers", &markersOpen)) {
            checkbox("Enable recorder markers", "medal_enabled", state);
            checkbox("Mark every aim", "medal_every_aim", state);
            checkbox("Mark with a separate key", "medal_separate_key", state);
            edit("Manual marker shortcut", "medal_marker_key");
            edit("Recorder shortcut", "medal_target_key");
            if (ft::button("Apply marker shortcuts")) {
                send("medal_marker_key", edits["medal_marker_key"].data());
                send("medal_target_key", edits["medal_target_key"].data());
            }
            ft::text_wrapped("Use the same shortcut in your recorder. Forwarded keys also reach the foreground app; recorder receipt is not confirmed.");
            ft::text_wrapped(state.at("status").c_str());
        }

    });
    ft::separator();
    ft::row({1, 1, 1}, [&] {
        if (ft::button("Show if armed", ft::ColorRole::Accent)) send("show");
        if (ft::button("Hide")) send("hide");
        if (ft::button("Save config", ft::ColorRole::Success)) { send("save"); feedback = "Configuration saved"; }
    });
    ft::text_wrapped(state.at("input_status").c_str());
    if (!feedback.empty()) ft::text_wrapped(feedback.c_str());
    ft::end();
    return true;
}
void closeSetup() { ft::shutdown(); backend = {}; edits.clear(); }
}

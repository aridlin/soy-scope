# Soy Scope for KDE / Linux

Native port of aridlin/soy-scope through the 2026.09.12 update. The original icon
and image are embedded. Launch Soy Scope from the menu, or run `soy-scope --ft-tui`
for terminal setup. `--ft-gui` opens the Qt setup window.

Includes configurable toggle shortcut, right/left/middle/side mouse trigger,
trigger delay, physical-pixel width, opacity, fingertip anchor, custom images,
embedded-image reset, preview/hide, reset anchor, and persistent configuration.
Changing triggers cancels the pending overlay and requires releasing the button.

Optional recorder marker forwarding includes every-aim and separate-key modes,
configurable manual/target shortcuts, conflict checks, repeat suppression,
two-second held-modifier/key timeout, queued-request coalescing, 50ms presses,
and release on cancellation/exit. Preview does not create markers. The native
uinput shortcut is sent to the Linux desktop, including the foreground app;
receipt by Medal or another recorder is not confirmed. Markers default off.

`~/.config/soy-scope.ini` uses the Windows configuration keys and is read at
startup. Save config / a normal window close writes it; `--config PATH` imports
another INI. Qt user settings remain in `~/.config/aridlin/Soycope.conf`.


TUI: type `help` for the commands and settings. `--ft-web` opens local browser
setup with the same controls. The web server listens only on 127.0.0.1, uses a
per-run access token, and rejects state changes from other origins. It stops
when Soy Scope exits. GUI, TUI and web setup are included.

Requires KDE global shortcuts, input-device read access and, for optional marker
forwarding, /dev/uinput write access. It never grabs physical input devices.
On KDE Wayland the overlay uses layer-shell; on X11 it uses a click-through
window. Desktop positioning and shortcut support depend on the compositor.

Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j4`.
Tests: `ctest --test-dir build --output-on-failure` checks the original
luminance transparency mask and marker queue timing/cancellation/held-key logic.
`python linux/test-controls.py build/linux/soy-scope` verifies batched terminal input,
Windows-format settings persistence and local web access protection. It requires
loopback networking, uses temporary configuration and disables physical input
and global shortcut registration.


## Dependencies and installation

This port targets KDE Plasma 6 on Linux x86-64. It uses Qt 6 Widgets/Network,
KGlobalAccel, KWindowSystem and LayerShellQt. The Linux UI currently uses Qt;
the Windows UI uses FT. Source is built from the repository root.

Arch Linux / CachyOS build dependencies:

```sh
sudo pacman -S --needed base-devel cmake python qt6-base qt6-imageformats kglobalaccel kwindowsystem layer-shell-qt
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
cmake --install build
```

The release archive is a dynamically linked Arch/CachyOS x86-64 build, not a
universal Linux binary. Install the runtime packages listed above and extract
its `bin/` and `share/` directories into `~/.local/`. Other distributions should
build from source with compatible Qt 6 and KDE Frameworks 6 development packages.

Preview and setup work without device permissions. Automatic trigger detection
requires read access to the relevant `/dev/input/event*` devices; optional marker
forwarding requires write access to `/dev/uinput`. Permissions are managed by the
OS/session administrator; the app does not grant itself access or grab devices.

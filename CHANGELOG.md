# Linux release 2026.09.18

- Publish the KDE Plasma 6 Linux port with native Wayland layer-shell and X11 overlays.
- Include GUI, terminal and local browser setup, persistent Windows-compatible INI settings and optional recorder marker forwarding.
- Add Linux CMake installation, desktop integration with image-mask, marker-state and setup-control tests.
- Refresh Windows' bundled FT with rendering, input, HTTP and widget fixes.

# Changelog

## 2026.09.12

- Added optional Medal shortcut forwarding: mark every aim, use a separate key, or enable both.
- Persist Medal preferences and configurable manual/Medal shortcuts; markers default off.
- Prevent shortcut collisions, wait for unrelated held modifiers, and release injected keys on disable or exit.
- Added deterministic input/configuration regression tests; actual Medal receipt still requires Windows runtime validation.

- Added a checkbox to replace right-click with left, middle, or either side mouse button in GUI and TUI setup.
- Save the alternate-button preference in the INI; existing configurations keep right-click.
- Cancel pending display and hide the overlay when changing trigger buttons.

## 2026.09.11

First public packaging of the latest local source snapshot, last modified on 27 August 2026.

- Portable Windows executable with embedded image and icon.
- Native GUI and terminal setup in one executable.
- Configurable hotkey, right-click delay, width, opacity, and anchor.
- DirectComposition overlay rendering and saved INI configuration.
- Added usage/build documentation, example configuration, GUI/TUI launchers, and Windows CI.

This publication does not change application behavior from that source snapshot.

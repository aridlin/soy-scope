# Soy Scope

<p align="center"><img src="assets/soyscope-icon.png" alt="Soy Scope icon" width="160"></p>

A portable Windows overlay that puts the pointing soyjak's fingertip on your crosshair. Press **F8** to arm it, hold **right click** to show it after a short delay, and release right click to hide it.

The overlay is transparent, click-through, and positioned relative to the center of your primary display. The image and application icon are embedded in the executable.

[Download for Windows](https://github.com/aridlin/soy-scope/releases/latest) · [Report a bug](https://github.com/aridlin/soy-scope/issues)

## Quick start

1. Download `soy-scope-windows-x64.zip` from Releases and extract it into a writable folder.
2. Run `soy_scope.exe` or `Soy Scope GUI.cmd`.
3. Press **F8** to arm the right-click trigger.
4. Hold the right mouse button. The overlay appears after **250 ms**; releasing the button hides it immediately.
5. Press **F8** again to disarm it.

Use the setup window to change the image, hotkey, delay, size, opacity, or fingertip anchor. Settings apply while the app runs. **Save config** writes them to `soy-scope.ini` beside the executable; closing the app also saves them.

## GUI and TUI

Both interfaces are included in the same executable:

```powershell
# Native graphical setup
.\soy_scope.exe --ft-gui

# Terminal setup; run inside a terminal
.\soy_scope.exe --ft-tui
```

The release includes launchers for both. The TUI changes the setup interface; the overlay still appears on the Windows desktop. FT also exposes `--ft-web`, but the normal entry points documented here are GUI and TUI.

## Controls and defaults

| Setting | Default | Purpose |
| --- | --- | --- |
| Hotkey | `F8` | Toggle the right-click trigger |
| Delay | `250 ms` | Wait before showing the overlay |
| Width | `1200 px` | Scale the overlay image |
| Opacity | `0.72` | Set overlay transparency |
| Anchor X / Y | `57.5% / 37.5%` | Place this point in the image at the primary screen's center |
| Image | Embedded pointing soyjaks | No separate asset file needed |

- **Browse image** selects a WebP, PNG, JPEG, or BMP supported by the installed Windows image decoder.
- **Use embedded** restores the included image.
- **Show if armed** previews the overlay when armed; **Hide** hides it.
- **Reset anchor** restores the default fingertip position.
- Hotkey examples: `F8`, `Ctrl+Alt+S`, `Shift+F9`. Choose another key if another app owns the hotkey.

See [`soy-scope.example.ini`](soy-scope.example.ini) for the configuration format. Copy it to `soy-scope.ini` to configure the app by hand.

## Build from source

Requires Windows x64, CMake 3.20+, Visual Studio 2022 Build Tools with **Desktop development with C++**, and a Windows SDK.

```powershell
git clone https://github.com/aridlin/soy-scope.git
cd soy-scope
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
.\build\Release\soy_scope.exe --ft-gui
```

The app uses C++17, Win32, Direct3D 11, DirectComposition, Direct2D, and Windows Imaging Component. The MSVC runtime is linked statically. FT is vendored, so no package manager or separate UI-library download is needed. GitHub Actions builds the Windows executable on pushes and pull requests.

## How it works

The setup interface sends settings to a separate overlay controller. A global hotkey arms the trigger, mouse-button polling detects right-click state, and a timer handles the delay. The image is prepared ahead of display; DirectComposition controls its visibility and opacity. This is a desktop overlay with no game-specific integration.

## Limitations

- Windows only; the anchor targets the primary display.
- Use a writable folder to save settings.
- Exclusive fullscreen or an application's overlay restrictions may prevent a desktop overlay from appearing. Compatibility with individual games has not been verified.
- CI checks compilation; it does not test physical mouse input, hotkeys, or visual alignment.

## Project layout and notices

`src/main.cpp` contains the app and overlay controller; `vendor/` contains FT; `resources/` embeds the image and icon from `assets/`.

See [third-party notices](THIRD_PARTY_NOTICES.md). No project-wide license has been added; public availability alone does not grant a license to redistribute third-party artwork.

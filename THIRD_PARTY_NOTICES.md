# Third-party notices

## FT

`vendor/ft.hpp` is the bundled FT GUI/TUI/web facade; `vendor/ftui.hpp` is a compatibility include. Upstream: https://github.com/aridlin/ft.

The supplied snapshot does not include a separate license notice for these headers. Their contents are preserved without assigning a new license.

## Artwork

`assets/two-soyjaks-pointing.webp` is preserved from the existing Soy Scope project. The original author, source URL, and redistribution license were not recorded in that snapshot. No new ownership or licensing claim is made here.

The reticle-and-pointers icon is newly drawn in `assets/soyscope-icon.svg`; the PNG and ICO files are generated from that vector source.

## Windows APIs

The app links to Windows system libraries supplied by Microsoft. This repository does not include a Windows SDK or those system DLLs.

## Linux runtime libraries

The Linux port dynamically links system Qt 6, KDE Frameworks 6 and LayerShellQt
libraries, plus Cairo, X11 and XRandR for FT. These are supplied by the Linux distribution, not bundled into the
release archive. Their own licenses and notices apply.

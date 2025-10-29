# PrezTool:
Made this tool so that I could draw and zoom on my screen, during live meetings. Inspired by zoomit (Microsoft Sysinternals).

# Supported Platforms:
- linux
    - [X] X11 (tested on fedora)
    - [ ] wayland
- [ ] Windows 10 and 11 (framework for it is there, I just have not defined a few required functions; I don't have access to a windows machine)

# Features:
- Zooming
    - [X] Zoom in/out: `mouse-scroll`
    - [X] Reset zoom: `z`
- Translating
    - [X] Drag moving the image: `mouse-2`
- Drawing on the screen
    - [X] Drawing: `mouse-1`
    - [X] Clear drawing: `c`
    - [X] Change brush color: `arrow-left` | `arrow-right`
    - [X] Undo drawing: `u`
    - [X] Toggle eraser: `e`
    - [X] Display brush preview: `p`
- Mirror Image
    - [X] Mirror Horizontal: `x`
    - [X] Mirror Vertical: `v`
- Highlight
    - [X] Toggle `h`
    - [X] Change Size: `h` + `mouse-scroll`
- Flashlight
    - [X] Toggle: `f`
    - [X] Change Size: `f` + `mouse-scroll`
- Rotation
    - [ ] Rotate Clockwise: `r`
    - [ ] Rotate Counter-Clockwise: `shift` + `r`
- Screen Selection
    - Initialization:
        - [ ] Display
        - [ ] Window
        - [ ] Cropped
    - [ ] Crop the screen
- Recapture Screen
    - [X] Take a screenshot of desktop and reload the app background: `t`
- Print Screen To:
    - [ ] File
    - [ ] Clipboard

# Dynamic Build:
## Prerequisites:
- gcc
- make
- raylib development library
### raylib
- fedora: `dnf install raylib-devel`
## Commands:
```bash
git clone https://gitlab.com/Uangn/preztool.git
cd preztool
make
```

# Static Build:
## Prerequisites:
- gcc
- make
## Commands:
```bash
git clone --recurse-submodules https://gitlab.com/Uangn/preztool.git
cd preztool
make static

# If you forgot to `--recurse-submodules` when cloning initially
cd preztool
git submodule update --init --recursive
```

# Run:
## If in that directory:
```bash
./preztool
```
## If outside that directory:
You could make a script that moves over to that directory
### Linux (sh)
Make a file with the contents below. Place it in the system path. Make it executable (optional).
```bash preztool_run.sh
#!/bin/sh
cd DIRECTORY_OF_PREZTOOL
./preztool
```
If executable:
```bash
./preztool_run.sh
```
If not executable:
```bash
sh preztool_run.sh
```

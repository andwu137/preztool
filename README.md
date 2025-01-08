# PrezTool:
Made this tool so that I could draw and zoom on my screen, during live meetings. Inspired by zoomit (Microsoft Sysinternals).

# Supported Platforms:
- [X] linux (tested on fedora)
- [ ] windows 10 and 11 (very unstable, I don't have a windows machine)

# Features:
- Zooming
    - [X] Zoom in/out: `mouse-scroll`
    - [X] Reset zoom: `z`
- Translating
    - [X] Drag moving the image: `mouse-2`
- Drawing on the screen
    - [X] Drawing: `mouse-1`
    - [X] Reset drawing: `c`
    - [X] Change brush color: `arrow-left` | `arrow-right`
    - [ ] Undo drawing: `u`
    - [ ] Toggle eraser: `e`
    - [ ] Display brush preview
- Mirror Image
    - [X] Mirror Horizontal: `x`
    - [X] Mirror Vertical: `v`
- Highlight
    - [X] Toggle `h`
- Flashlight
    - [X] Toggle: `f`
    - [X] Change Size: `s` + `mouse-scroll`
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
    - [ ] Take a screenshot of desktop and reload the app background

# Dynamic Build:
## Clone:
```bash
git clone https://gitlab.com/Uangn/preztool.git
```
## Prerequisites:
- gcc
- make
- raylib development library
### raylib
- fedora: `dnf install raylib-devel`
## Compile:
```bash
make
```

# Static Build:
## Clone:
```bash
git clone --recurse-submodules https://gitlab.com/Uangn/preztool.git
```
If you forgot to `--recurse-submodules` when cloning initially
```bash
cd preztool
git submodule update --init --recursive
```
## Prerequisites:
- gcc
- make
## Compile
```bash
make static
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

# DE1-SoC Shell - README

## Overview
A custom shell and desktop environment built for the DE1-SoC FPGA board, running on the RISC-V RV32 processor. The project includes a fully interactive visual desktop with mouse support, a text editor, and a terminal shell, all rendered on a 320x240 VGA display using RGB565 color format.

---

## Features

### Visual Desktop
- Animated gradient wallpaper transitioning between two configurable RGB565 colors across the screen diagonally
- Three desktop app icons rendered in a taskbar at the top of the screen: **Terminal**, **Notepad**, and **Paint** (Paint coming soon)
- Icons highlight with a blue hue on single click and launch on double click
- Clicking outside any icon deselects it

### Mouse Support
- Full PS/2 mouse support with real-time cursor rendering
- 4x4 pixel cursor that correctly saves and restores the pixels beneath it as it moves
- Mouse coordinates clamped to screen bounds to prevent memory access violations
- Rising-edge click detection so holding the mouse button does not repeatedly trigger clicks
- Mouse input is flushed when returning to terminal mode to prevent PS/2 FIFO overflow

### Text Editor (Notepad)
- Accessible from the visual desktop by double clicking the Notepad icon
- Basic text input and editing via keyboard
- Features command-inputs as well such as "delete row" to increase usability
- Returns to the desktop cleanly on exit

### Terminal Shell
- Command-line interface accessible from the visual desktop by double clicking the Terminal icon
- Keyboard driven input via PS/2
- Mouse input is ignored and flushed while in terminal mode to prevent buffer overflow

---

## How to Run in CPUlator

1. Go to [cpulator.01xz.net](https://cpulator.01xz.net) and select **RISC-V RV32 DE1-SoC** as the system
2. Load your your source file (CPUlator allows only one file at a time, so use merged.c)
3. Under the **Peripherals** panel on the right, ensure the following are visible:
   - **PS/2 port 1** (base address `0xFF200100`) - connect your **keyboard** here by clicking the port and typing into the input field
   - **PS/2 port 2** (base address `0xFF200108`) - connect your **mouse** here
   - **VGA pixel buffer** - this is where all graphics are rendered
4. Compile and run the program
5. The visual desktop will appear in the VGA display panel
6. Use the **mouse** to navigate the desktop and click icons
7. Double click the **Terminal** icon to enter the shell
8. Double click the **Notepad** icon to open the text editor

### PS/2 Notes
- **PS/2 Port 1** (`0xFF200100`) is reserved for **keyboard** input
- **PS/2 Port 2** (`0xFF200108`) is reserved for **mouse** input
- Both ports can be used simultaneously
- If the PS/2 FIFO overflow warning appears in the simulator, it means mouse packets are not being flushed fast enough - this is handled automatically when switching between modes

## Project Structure
```

├── shell_v7.c                  # Entry point
├── visual_system.c/h           # Desktop, wallpaper, icons, mouse rendering
├── text_editor_v2.c            # Notepad application
├── text_editor.h               # Notepad application
├── helper_funcs.c/h            # VGA and PS/2 utility functions
├── picture_array.c/h           # Pixel art arrays for icons and images
|
├── merged.c                    # Consolidated code file for CPUlator

```

---

## Known Limitations
- No window management - apps are full screen
- No persistent storage, all state is lost on reset

---

## Hardware Target
- **Board:** DE1-SoC
- **Processor:** RISC-V RV32
- **Display:** 320x240 VGA, RGB565 color format
- **Input:** PS/2 keyboard and mouse

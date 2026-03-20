# Programming in Graphical Environment 

## Laboratory Task - WinAPI 3 - DevCalculator (Basic)

### 1. Introduction
Your task is to create a classic desktop calculator window using pure WinAPI. In this laboratory part, you will implement the **"Basic"** mode, focusing on UI layout, custom GDI rendering, and standard arithmetic logic.

---

### 2. Requirements

#### Main Application Window:
- Must have a title bar, system menu, and support for minimization, maximization, and smooth resizing.
- Use a **custom icon** instead of the default Win32 icon.
- Enforce a **minimum window size** (e.g., 320x400 pixels).
- Smooth and proportional scaling of all UI elements during resizing.
- **Main Menu**:
    - **Mode**: "Basic" (checked) and "Programmer" (should currently display a MessageBox: "Not implemented yet").
    - **Edit**: "Clear" (shortcut: *Esc*).
- Full keyboard support (digits, operators, Enter, Esc, Backspace).

#### UI Layout:
- Dynamic creation of standard `BUTTON` controls for digits and operators.
- The display occupies the top 1/4 of the window; buttons occupy the remaining space arranged in a grid.
- Constant **5-pixel padding** between all elements and window edges.
- The "=" button in the bottom row must span the remaining available width.

#### Custom Display Control (GDI):
- Instead of standard controls, create a separate window class via `RegisterClassEx`.
- White background.
- Right-aligned text rendering:
    - Top: Smaller, dark gray operation history.
    - Bottom: Larger, black, bold current input or result.
- Dynamic font sizing updated only in response to `WM_SIZE`.

---

### 3. Scoring (8 pts)
- **1 pt.** Correct enforcement of the minimum main window size.
- **1 pt.** Implemented menu with shortcuts (Accelerator table).
- **2 pts.** Custom display control drawing history/result using GDI with dynamic fonts.
- **2 pts.** Scalable grid layout for buttons with constant padding.
- **2 pts.** Correct handling of numeric/character keyboard input and focus management.

---

### 4. Technical Hints

#### UI Management:
- To enforce minimum size, handle the `WM_GETMINMAXINFO` message.
- Use `BeginDeferWindowPos`, `DeferWindowPos`, and `EndDeferWindowPos` for optimal bulk movement of buttons during `WM_SIZE`.
- Use `SetFocus` to return keyboard control to the main window after a button click.

#### Graphics (GDI):
- Create `HFONT` objects using `CreateFontW` only during `WM_SIZE` and clean them up with `DeleteObject`. Do not create GDI objects inside the `WM_PAINT` loop.
- Use `SetTextColor`, `SetBkMode(..., TRANSPARENT)`, and `DrawTextW` with flags like `DT_RIGHT | DT_VCENTER` for text rendering.
- Register your custom class with `CS_HREDRAW | CS_VREDRAW` styles to ensure it repaints correctly on resize.

## Home Task - WinAPI 3 - DevCalculator

### 1. Introduction
This task is a continuation of the laboratory assignment - you must first finish the lab part to continue. 
You will extend the "Basic" calculator with a fully functional **Programmer Mode**, featuring bit manipulation, advanced data types, and persistent window settings.
In all cases where the specification is not clear or a detail is missing, you should mimic the behavior of the provided example application, except for any potential bugs found within it.

---

### 2. Requirements

#### Programmer Mode & Advanced Data Types:
- Implement a fully functional mode selectable via the menu or shortcuts (**Ctrl+1** for Basic, **Ctrl+2** for Programmer).
- Support for multiple data types via a `COMBOBOX`:
    - **Integers**: 8/16/32/64 bit (signed/unsigned).
    - **Floating point (IEEE 754)**: Half (16-bit), Float (32-bit), Double (64-bit).
- Support for display bases: **Hex, Dec, Oct, Bin** with appropriate prefixes (`0x`, `b`, `o`) and shortcuts (**Ctrl+H/D/O/B**).
- Full support for Hexadecimal input (A-F keys and UI buttons).

#### Interactive Bit Display (Custom GDI):
- A second custom GDI control that visualizes the binary representation (up to 64 bits).
- **Interactivity**: Allow users to toggle individual bits by clicking.
- **Visual Feedback**:
    - Add a hover effect for bits.
    - Highlight components for floating-point types: **Sign bit** (Red), **Exponent** (Green), **Mantissa** (Blue).
    - Use smart font scaling: if boxes become too narrow, scale the font size based on box width.
- **Tooltips**: Display informative weighted hints (e.g., `Sign bit`, `Exponent: 2^N (Bias: X)`, `Mantissa: 2^-N`).

#### Logic & Features:
- **Exact Input**: Decimal input should preserve the raw string during typing to avoid immediate rounding.
- **Precision Warning**: Display the actual stored value in red below the main input if the decimal value cannot be exactly represented by the chosen floating-point type.
- **Productivity**: Support **Copy (Ctrl+C)** and **Paste (Ctrl+V)** for numeric values.

#### Window Management & Persistence:
- **Always on Top**: Toggleable top-most state.
- **Translucency**: When "Always on Top" is active, make the window partially transparent (approx. 70% opacity) whenever it loses focus.
- **Configuration Persistence**: Save and load window position, size, mode, data type, base, and view options in a `config.ini` file.

---

### 3. Scoring (12 pts)

#### Programmer Logic & Data Types (3 pts)
- **Robust Types**: Correct bit-level interpretation of all 11 types (Int/UInt 8-64, Half, Float, Double) with correct casting during type changes.
- **Input Accuracy**: Precise string-based decimal input that matches hardware-level floating point representations.
- **Mode Switching**: Seamless transition between Basic and Programmer modes with UI synchronization and functional shortcuts (**Ctrl+1/2**).

#### Interactive Bit Display (4 pts)
- **Core Interactivity**: Accurate bit toggling via mouse clicks, distinct hover effects, and flicker-free rendering using double buffering.
- **Advanced Visuals**: Intelligent font scaling (width-aware) and clear color coding for floating-point components (Sign/Exp/Mant).
- **Informative Tooltips**: Dynamic hints showing the specific mathematical contribution, bias, and weights of each bit.

#### Formatting & User Experience (2 pts)
- **Base Support**: Correct conversion and display for Hex, Oct, and Bin bases including standard prefixes and input filtering.
- **Readability**: A red "Precision Warning" for floating-point approximations.
- **Clipboard**: Reliable implementation of **Ctrl+C** and **Ctrl+V** for numeric values.

#### Window Management & Persistence (3 pts)
- **Window States**: Functional "Always on Top" mode with appropriate menu state tracking.
- **Focus Effects**: Automatic transparency adjustment (translucency) when the window is inactive in Top-Most mode.
- **Full Persistence**: Robust saving/loading of window position, size, and every UI setting in a `config.ini` file.

---

### 4. Hints

#### Graphics & Tooltips:
- **Double Buffering**: To prevent flickering in custom controls, return `1` for `WM_ERASEBKGND` and perform all drawing on a memory DC created with `CreateCompatibleDC` and `CreateCompatibleBitmap`, then `BitBlt` to the screen.
- **Manual Tooltips**: Use `TOOLTIPS_CLASS` with `TTF_TRACK` and `TTF_ABSOLUTE`. Manually manage position and visibility via `TTM_TRACKPOSITION` and `TTM_TRACKACTIVATE`.

#### Window Features:
- **Transparency**: Requires the `WS_EX_LAYERED` extended style. Use `SetLayeredWindowAttributes` with the `LWA_ALPHA` flag to control opacity.
- **Persistence**: Use `WritePrivateProfileStringW` and `GetPrivateProfileIntW` to manage `.ini` configuration files.

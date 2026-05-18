# iRon AI Analysis & Refactor Plan

## Project Overview
iRon is a lightweight overlay suite for iRacing, providing customizable on-screen displays such as relative position, dashboard (DDU), input graphs, standings, and a cover overlay. The project is built with C++ and DirectX, aiming for minimal dependencies and high performance.

## Source Structure
```
iRon-with-ai/
├── main.cpp                 # Entry point, message loop, hotkey handling, overlay creation
├── Overlay.h                # Base Overlay class with DirectX/Direct2D resources
├── Overlay.cpp              # Base Overlay implementation
├── OverlayRelative.h/.cpp   # Relative position overlay (with minimap option)
├── OverlayInputs.h/.cpp     # Throttle/brake/steering input graph overlay
├── OverlayStandings.h/.cpp  # Field standings overlay (with safety rating, iRating, pit age)
├── OverlayDDU.h/.cpp        # Dashboard overlay (fuel calculator, lap times, etc.)
├── OverlayCover.h/.cpp      # Simple cover overlay to block distracting in-game elements
├── OverlayDebug.h/.cpp      # Debug overlay (shown only in debug builds)
├── Config.h/.cpp            # JSON configuration loader/writer with file watcher
├── iracing.h/.cpp           # Wrapper around iRacing SDK (irsdk) for data access
├── irsdk/                   # iRacing SDK headers and utilities (provided)
├── picojson.h               # JSON parsing library (single header)
├── util.h                   # Miscellaneous helper functions
├── resources (icons, etc.)  # PNG images for overlays (inputs.png, ddu.png, etc.)
├── iron.sln, iron.vcxproj   # Visual Studio solution and project files
├── README.md                # Original project documentation
└── LICENSE                  # MIT License
```

## Key Components & Responsibilities

### 1. main.cpp
- **Entry Point**: Sets process priority, loads configuration, registers global hotkeys.
- **Overlay Management**: Creates instances of all overlays (Cover, Relative, Inputs, Standings, DDU, and Debug in debug mode).
- **Main Loop**:
  - Calls `ir_tick()` to refresh iRacing connection status and session data.
  - Handles connection/status changes (disconnected, connected, driving) and updates overlay visibility accordingly.
  - Processes Windows messages for hotkey toggles (UI edit, toggle each overlay).
  - Updates and renders overlays each frame (with optional performance mode to update at 30Hz).
- **Hotkeys**: Configurable via `config.json` (default: Alt-J for UI edit, Ctrl+1/2/3/4 for overlays, Ctrl+Space for standings).

### 2. Overlay.h / Overlay.cpp
- **Base Class**: Defines the interface for all overlays.
- **DirectX Resources**: Manages Direct3D 11 device, swap chain, Direct2D factory, render target, DirectWrite factory, DirectComposition objects for layered window rendering.
- **Lifecycle Methods**: `onEnable()`, `onDisable()`, `onUpdate()`, `onConfigChanged()`, `onSessionChanged()` (to be overridden by derived classes).
- **Window Handling**: Methods to set position/size, save window placement, enable/disable UI edit mode.
- **Rendering**: Provides common rendering utilities (brush, text rendering) via protected members.

### 3. Config.h / Config.cpp
- **Configuration**: Loads and saves `config.json` in the working directory.
- **File Watching**: Uses a background thread to monitor file changes and reload automatically.
- **API**: Provides typed getters/setters for components (e.g., `getBool(component, key, default)`).
- **Thread Safety**: Uses `std::atomic<bool>` for change notification.

### 4. iracing.h / iracing.cpp
- **iRacing SDK Wrapper**: Encapsulates the iRacing SDK (irsdk) connection and data retrieval.
- **Initialization**: Loads the iRacing SDK DLL and initializes the connection.
- **Data Access**: Provides global-like access to telemetry (e.g., `ir_IsOnTrackCar`, `ir_Speed`, session info) via wrapper functions/variables.
- **Error Handling**: Checks connection status and provides reconnection logic.

### 5. Overlay Overlays (Relative, Inputs, Standings, DDU, Cover, Debug)
Each overlay inherits from `Overlay` and implements:
- **onEnable()**: Initial overlay setup (e.g., load fonts, initialize internal state).
- **onDisable()**: Cleanup.
- **onUpdate()**: Update internal state based on latest iRacing data (called each frame).
- **onConfigChanged()**: Reload settings from `config.json` (e.g., colors, fonts, visibility toggles).
- **onSessionChanged()**: Adjust to new session type (practice, race, qualifying, etc.).
- **getDefaultSize()**: Return default width/height for the overlay.
- **hasCustomBackground()**: Whether the overlay draws its own background (affects composition).

Specifics:
- **OverlayRelative**: Shows car position relative to others, optional minimap, buddy highlighting, license, iRating, laps since pit.
- **OverlayInputs**: Moving graph of throttle, brake, and steering inputs.
- **OverlayStandings**: Table of all cars in session with position, name, license, iRating, safety rating, laps since pit.
- **OverlayDDU**: Dashboard with fuel calculator (estimated laps remaining, fuel used, etc.), lap times, sector times, gear, RPM, speed.
- **OverlayCover**: Transparent rectangle to block in-game UI elements.
- **OverlayDebug**: Displays raw iRacing variable values for debugging (only in debug builds).

### 6. irsdk/ Directory
- Contains the iRacing SDK header files (`irsdk_defines.h`, `irsdk_client.h`, etc.) and utility code (`yaml_parser.h/.cpp`) for reading the iRacing SDK's session YAML descriptions.
- These are provided as part of the repository to avoid external SDK download.

### 7. picojson.h
- Single-header JSON library used for configuration parsing.

### 8. util.h
- Helper functions such as string conversion, color manipulation, math helpers, and debugging macros.

## Libraries & Dependencies
- **DirectX**: DirectX Graphics Infrastructure (DXGI), Direct3D 11, Direct2D, DirectWrite, DirectComposition (all via Windows SDK).
- **iRacing SDK**: Provided locally in `irsdk/`; links against the iRacing SDK DLL at runtime (no lib needed).
- **picojson**: Header-only JSON parser.
- **Standard Library**: C++11/14/17 features (atomics, threads, vectors, strings, WRL smart pointers).
- **Windows API**: For window creation, message loop, hotkeys, process priority.

## Build Instructions (from original README)
- Requires Visual Studio 2022 (free edition sufficient).
- Solution/Project files included; should build out-of-the-box if DirectX development components are installed.
- No external dependencies beyond Windows SDK and the included iRacing SDK headers.

## Potential AI Enhancement Areas
As requested, the goal is to fork iRon and add AI analysis capabilities. Possible extensions:
1. **Telemetry Analysis Overlay**: New overlay that uses AI models to predict optimal braking points, suggest gear shifts, or warn of impending loss of control.
2. **Voice Command Integration**: Use speech-to-text to allow voice commands for toggling overlays or changing settings.
3. **Behavioral Cloning**: Record user inputs and overlay preferences to suggest personalized layouts.
4. **Incident Prediction**: AI model that flags potential incidents based on proximity and relative velocity.
5. **Natural Language Config**: Allow configuring overlays via natural language commands (e.g., "make the relative overlay bigger and move it to the top-left").

These would require:
- Adding new overlay classes (e.g., `OverlayAIAnalysis`).
- Integrating an inference framework (TensorRT, ONNX Runtime, or direct PyTorch/TensorFlow via DLL).
- Possibly collecting andprocessing telemetry data (via the existing iRacing SDK wrapper).
- Ensuring low latency to maintain overlay responsiveness.

## Files of Interest for AI Integration
- **main.cpp**: Where new overlay instances would be created and updated.
- **Overlay.h**: Base class to inherit from for new AI overlay.
- **iracing.h/.cpp**: Source of raw telemetry data to feed into AI models.
- **Config.h/.cpp**: To add new configuration options for AI features (model paths, thresholds, etc.).
- **util.h**: May need helpers for tensor formatting or data normalization.

## Conclusion
iRon is a well-structured, lightweight overlay suite with clear separation of concerns. Its use of DirectX for rendering and the iRacing SDK for data access makes it a solid foundation for adding AI-driven enhancements while maintaining low overhead and high fidelity.

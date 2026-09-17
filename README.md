# pubg-external

External PUBG cheat with Discord overlay hijacking and DirectX 11 rendering.

## Features

- **ESP**
  - Box ESP
  - Health bars (color-coded)
  - Player names
  - Distance display
  - Team filtering
  - Downed player filtering

- **Aimbot**
  - Silent aim
  - FOV-based targeting
  - Distance filtering
  - Smoothness control
  - Target selection modes

## Requirements

- Windows 10/11 x64
- Visual Studio 2022 (v143 toolset)
- Discord Overlay enabled (for overlay rendering)
- Hypervisor driver (HVRE/Athena)

## Building

1. Open `pubg-external.vcxproj` in Visual Studio 2022
2. Select Release | x64 configuration
3. Build the project
4. Output: `bin\pubg-external.exe`


## Project Structure

```
pubg-external/
├── root/
│   ├── entry.cxx              # Main entry point
│   ├── engine/                # Core engine and memory functions
│   │   ├── engine.cxx/.cxx    # Initialization and threads
│   │   ├── offsets/           # PUBG game offsets
│   │   ├── math/              # Vector math and projections
│   │   ├── functions/         # Memory reading helpers
│   │   └── xenuine.cxx/.cxx    # Xenuine decryptor
│   ├── driver/                # Hypervisor communication
│   │   └── athena/hypervisor/  # HVRE implementation
│   ├── features/              # Cheat features
│   │   ├── aimbot/            # Aimbot logic
│   │   └── visuals/           # ESP rendering
│   ├── overlay/               # Discord overlay
│   │   ├── hijack.cxx/.cxx    # Window hijacking
│   │   └── renderer.cxx/.cxx  # DirectX 11 renderer
│   ├── dependencies/          # External libraries
│   │   ├── imgui/             # ImGui UI library
│   │   └── freetype/          # Font rendering
│   └── additional/            # Utilities
│       ├── logger/            # Console logging
│       └── config/            # Configuration
└── pubg-external.vcxproj      # Visual Studio project
```

## Offsets

Current PUBG offsets are defined in `root\engine\offsets\offsets.hxx`. Update these if the game patches.

## Disclaimer

This project is for educational purposes only. Use at your own risk.

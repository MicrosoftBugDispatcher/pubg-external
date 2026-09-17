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

- **Overlay**
  - Discord Overlay hijacking
  - DirectX 11 + ImGui rendering
  - Safe window positioning (no aggressive style modifications)

- **Memory Communication**
  - HVRE hypervisor integration
  - Xenuine decryptor for encrypted pointers
  - Safe process attachment

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

## Usage

1. Load your hypervisor driver
2. Run `pubg-external.exe`
3. Wait for PUBG game window detection
4. Discord Overlay will be hijacked automatically
5. ESP will render on the overlay

## Configuration

Edit `root\additional\config\config.hxx` to customize:

```cpp
namespace aimbot
{
    inline bool enabled = true;
    inline int bind_key = VK_RBUTTON;
    inline float field_of_view = 500.0f;
    inline float smooth_x = 0.15f;
    inline float smooth_y = 0.15f;
    inline float max_distance = 5000.0f;
    inline bool target_teammates = false;
    inline bool target_downed = false;
}

namespace visuals
{
    inline bool enabled = true;
    inline bool box = true;
    inline bool health_bar = true;
    inline bool name = true;
    inline bool distance = true;
}
```

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

## Notes

- The overlay uses Discord Overlay for rendering. Ensure Discord is running and overlay is enabled.
- Gentle window positioning is used to avoid triggering Windows gaming-overlay dialogs.
- The visuals thread was removed - ESP now renders in the DirectX overlay loop for proper synchronization.
- Project uses `.cxx/.hxx` file extensions for C++ sources.

## Disclaimer

This project is for educational purposes only. Use at your own risk.

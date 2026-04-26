# System-0 Engine

High-performance Forward+ 3D game engine optimized for **8GB RAM / 2GB VRAM** systems. Purpose-built for corridor-based, story-driven FPS games (HL2/F.E.A.R. style).

## Technical Specifications

### Core Architecture
- **Language**: C++20 (No embedded scripting - pure native code)
- **Renderer**: Forward+ (Clustered) with 100% dynamic lighting
- **NO baked lightmaps** - all lighting is real-time
- **APIs**: OpenGL 4.6 (Windows/Linux), Metal (macOS)

### Key Features
- **Uniform 3D Grid** (16×8×16) for light culling - O(1) cell lookup
- **Shadow Atlas**: Single 4K texture with distance-based LODs
  - <10m: 1024² resolution
  - <20m: 512² resolution  
  - <50m: 256² resolution
  - >50m: No shadows (VRAM savings)
- **Double-buffered Physics/Render** threading for stability
- **Arena Allocators** for zero per-frame allocations
- **Object Pools** for entities/projectiles

### Memory Budgets
| Resource | Budget |
|----------|--------|
| Shadow Atlas | ~64MB VRAM |
| Max Shadow Casters | 64 lights |
| Light Grid Cells | 2,048 cells |
| Max Dynamic Lights | 1,024 |

## Project Structure

```
system0-engine/
├── CMakeLists.txt          # Build configuration
├── src/
│   ├── core/               # Memory, logging, types
│   ├── rhi/                # Render Hardware Interface
│   │   ├── gl/             # OpenGL backend
│   │   └── metal/          # Metal backend
│   ├── renderer/           # Forward+ pipeline
│   ├── engine/             # Application, World
│   ├── loader/             # S0_DATA format
│   ├── physics/            # Bullet wrapper
│   └── audio/              # Miniaudio wrapper
├── cooker/                 # Asset cooking CLI
└── game/                   # Game executable
```

## Binary Format: S0_DATA

Custom binary format for instant level loading:

```cpp
struct S0Header {          // 64 bytes
    u32 magic;             // "S0DB"
    u32 version;
    u64 total_file_size;
    u64 vertex_data_offset;
    u64 index_data_offset;
    u64 light_data_offset;
    u64 collision_offset;
    u64 entity_count;
    u64 entity_data_offset;
};

struct PackedVertex {      // 32 bytes
    f32 position[3];       // World space
    u8 normal[4];          // Octahedral encoded
    f32 texcoord[2];       // UV coordinates
    f32 tangent[4];        // Tangent + bitangent sign
};

struct PackedLight {       // 48 bytes
    f32 position[3];
    f32 radius;
    f32 color[3];
    f32 intensity;
    LightType type;
    u32 casts_shadow;
    f32 spot_angles[2];
};
```

## Building

### Prerequisites
- CMake 3.20+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- SDL2
- Bullet Physics
- OpenGL 4.6 or Metal

### Build Commands
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Targets
- `s0_cooker` - Asset cooking CLI tool
- `s0_game` - Game executable

## Usage

### Cooking a Level
```bash
./s0_cooker input.map output.s0data
```

### Running the Game
```bash
./s0_game
```

## License

Proprietary - All rights reserved.

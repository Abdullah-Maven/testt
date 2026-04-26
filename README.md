# System-0 Engine

High-performance 3D game engine optimized for 8GB RAM / 2GB VRAM systems. Purpose-built for corridor-based, story-driven FPS games (HL2/F.E.A.R. style).

## Architecture Decisions

### 1. Spatial Partitioning: **Uniform 3D Grid**
- **Decision**: Uniform grid for Forward+ light culling + separate BVH for physics
- **Rationale**: Corridor environments have consistent vertical spacing; uniform grid provides O(1) cell lookup and fast frustum-light intersection tests
- **Grid Dimensions**: 16x8x16 cells optimized for typical indoor ceiling heights

### 2. Shadow Technique: **Single Atlas with Distance LODs**
- **Decision**: Single 4K shadow atlas with aggressive distance-based quality levels
- **Quality Tiers**:
  - <10m: 1024x1024 per light
  - <20m: 512x512 per light  
  - <50m: 256x256 per light
  - >50m: No shadows (bandwidth savings)
- **Max Shadowed Lights**: 64 concurrent
- **VRAM Budget**: ~64MB for shadow atlas (fits comfortably in 2GB VRAM)

### 3. Threading Model: **Double-Buffered Physics/Render**
- **Decision**: Double-buffered state with 1-frame latency
- **Rationale**: Story-driven FPS prioritizes stability over competitive latency; eliminates risk of pipeline stalls and race conditions
- **Sync Point**: End of frame, before vsync

## Project Structure

See file tree below.

## Binary Format: S0_DATA

Memory-mapped format for instant level loading:

| Section | Content | Alignment |
|---------|---------|-----------|
| Header (64B) | Magic, version, offsets | 8-byte |
| Vertices | 32-byte packed (position, oct-normal, UV, tangent) | 16-byte |
| Indices | u16/u32 triangle list | 4-byte |
| Lights | 48-byte PackedLight structs | 16-byte |
| Collision | Convex hull vertices for Bullet | 4-byte |
| Entities | Key-value string pairs | 1-byte |

## Building

### Prerequisites
- CMake 3.20+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)
- SDL2, Bullet Physics, miniaudio

### Linux/Windows (OpenGL)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### macOS (Metal)
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Usage

### Asset Cooker
```bash
./s0_cooker level.map assets_cooked/level.s0data
```

### Running the Game
```bash
./s0_game
```

## Performance Targets

| Metric | Target |
|--------|--------|
| Frame Time | <=16.67ms (60 FPS) |
| Draw Calls | <=500 per frame |
| VRAM Usage | <=1.5GB (headroom for drivers) |
| RAM Usage | <=4GB total |
| Light Count | 256 active dynamic lights |
| Shadow Casters | 64 concurrent |

## License

Proprietary - All rights reserved.

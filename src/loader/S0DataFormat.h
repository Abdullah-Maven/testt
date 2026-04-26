#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include "core/Types.h"

namespace s0::cook {

// ------------------------------------------------------------------
// S0_DATA Binary Format Specification
// Memory-mapped friendly structure for instant loading
// ------------------------------------------------------------------

// Magic number: "S0DB" (System-0 Database)
constexpr u32 S0_MAGIC = 0x42443053;
constexpr u32 S0_VERSION = 1;

// File Header (64 bytes)
struct S0Header {
    u32 magic;              // 0x00: S0_MAGIC
    u32 version;            // 0x04: S0_VERSION
    u64 total_file_size;    // 0x08: Total size in bytes
    u64 vertex_data_offset; // 0x10: Offset to vertex block
    u64 index_data_offset;  // 0x18: Offset to index block
    u64 light_data_offset;  // 0x20: Offset to light block
    u64 collision_offset;   // 0x28: Offset to collision hulls
    u64 entity_count;       // 0x30: Number of entities
    u64 entity_data_offset; // 0x38: Offset to entity data
};
static_assert(sizeof(S0Header) == 64, "S0Header must be 64 bytes");

// Vertex Format (32 bytes, optimized for GPU)
struct PackedVertex {
    f32 position[3];        // 12 bytes: World space position
    u8 normal[4];           // 4 bytes: Octahedral encoded normal (signed normalized)
    f32 texcoord[2];        // 8 bytes: UV coordinates
    f32 tangent[4];         // 8 bytes: Tangent vector (w = bitangent sign)
};
static_assert(sizeof(PackedVertex) == 32, "PackedVertex must be 32 bytes");

// Light Types
enum class LightType : u32 {
    Point = 0,
    Spot = 1,
    Directional = 2
};

// Packed Dynamic Light (48 bytes)
struct PackedLight {
    f32 position[3];        // 12 bytes: World space position (or direction for directional)
    f32 radius;             // 4 bytes: Light influence radius
    f32 color[3];           // 12 bytes: RGB color (linear space)
    f32 intensity;          // 4 bytes: Luminous intensity (candela)
    LightType type;         // 4 bytes: LightType enum
    u32 casts_shadow;       // 4 bytes: Boolean (0 or 1)
    f32 spot_inner_angle;   // 4 bytes: Inner cone angle (radians, for spotlights)
    f32 spot_outer_angle;   // 4 bytes: Outer cone angle (radians, for spotlights)
};
static_assert(sizeof(PackedLight) == 48, "PackedLight must be 48 bytes");

// Collision Hull Header
struct CollisionHullHeader {
    u32 vertex_count;       // Number of vertices in convex hull
    u32 padding;            // Alignment padding
    // Followed by: f32 positions[vertex_count * 3]
};

// Entity Data (Variable length, key-value pairs)
struct EntityHeader {
    u32 key_value_pairs;    // Number of key-value pairs
    u32 total_string_bytes; // Total bytes of all strings
    // Followed by: null-terminated key strings, then null-terminated value strings
};

// Brush data for CSG operations (used during cooking, not in final format)
struct MapBrush {
    std::vector<std::array<f32, 4>> planes; // Plane equations (ax + by + cz + d = 0)
    u32 texture_id;
    std::string texture_name;
};

// TrenchBroom Entity
struct MapEntity {
    std::string classname;
    Vec3 origin;
    std::vector<std::pair<std::string, std::string>> properties;
};

// Cooker Configuration
struct CookerConfig {
    std::string input_map_path;
    std::string output_s0data_path;
    bool optimize_meshes = true;
    bool generate_collision = true;
    bool bake_lighting = false; // Always false for Forward+ only
    f32 mesh_simplify_threshold = 0.0f; // 0.0 = no simplification
};

} // namespace s0::cook

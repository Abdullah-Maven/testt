#pragma once

#include <cstdint>
#include <memory>
#include "core/Types.h"
#include "rhi/GraphicsDevice.h"

namespace s0::renderer {

// ------------------------------------------------------------------
// Shadow Atlas Configuration (Optimized for 2GB VRAM)
// Aggressive packing with distance-based LODs
// ------------------------------------------------------------------
constexpr u32 SHADOW_ATLAS_SIZE = 4096;          // Single 4K atlas texture
constexpr u32 SHADOW_ATLAS_CASCADE_COUNT = 1;    // No CSM for interiors
constexpr u32 MAX_SHADOWED_LIGHTS = 64;          // Max lights casting shadows
constexpr u32 MIN_SHADOW_MAP_SIZE = 256;         // Minimum resolution per light
constexpr f32 SHADOW_DISTANCE_LOD_1 = 10.0f;     // <10m: 1024x1024
constexpr f32 SHADOW_DISTANCE_LOD_2 = 20.0f;     // <20m: 512x512
constexpr f32 SHADOW_DISTANCE_LOD_3 = 50.0f;     // <50m: 256x256
                                                 // >50m: No shadow (saves VRAM/bandwidth)

// ------------------------------------------------------------------
// Shadow Map Quality Levels
// ------------------------------------------------------------------
enum class ShadowQuality : u32 {
    High = 1024,    // For close lights (<10m)
    Medium = 512,   // For medium lights (<20m)
    Low = 256,      // For far lights (<50m)
    None = 0        // Beyond render distance or disabled
};

// ------------------------------------------------------------------
// Packed Shadow Slot in Atlas
// ------------------------------------------------------------------
struct ShadowSlot {
    u32 light_index;        // Index of light using this slot
    u32 atlas_x;            // X offset in atlas
    u32 atlas_y;            // Y offset in atlas
    u32 size;               // Resolution (256, 512, or 1024)
    bool is_active;         // Slot in use
    u32 padding[3];         // Alignment
};
static_assert(sizeof(ShadowSlot) == 32, "ShadowSlot must be 32 bytes");

// ------------------------------------------------------------------
// Shadow Atlas Manager
// Handles dynamic allocation/deallocation of shadow map regions
// Uses simple grid-based packing for speed
// ------------------------------------------------------------------
class ShadowAtlas {
public:
    ShadowAtlas(rhi::GraphicsDevice* device);
    ~ShadowAtlas();
    
    // Allocate a shadow slot for a light
    // Returns slot index or -1 if atlas is full
    i32 AllocateShadow(u32 light_index, f32 light_distance);
    
    // Free a shadow slot
    void FreeShadow(u32 slot_index);
    
    // Get the atlas texture (bind to shader)
    rhi::Texture* GetAtlasTexture() const { return m_atlas_texture.get(); }
    
    // Get slot data (for GPU buffer upload)
    const ShadowSlot* GetSlots() const { return m_slots.data(); }
    u32 GetActiveSlotCount() const { return m_active_count; }
    
    // Begin rendering shadows (returns command buffer to record shadow passes)
    void BeginShadowPass();
    void EndShadowPass(std::unique_ptr<rhi::CommandBuffer> cmd);
    
    // Update shadow matrices for a light
    Mat4 GetShadowMatrix(u32 slot_index, const Vec3& light_pos, f32 radius) const;

private:
    rhi::GraphicsDevice* m_device;
    std::unique_ptr<rhi::Texture> m_atlas_texture;
    std::unique_ptr<rhi::Texture> m_atlas_depth;
    std::vector<ShadowSlot> m_slots;
    u32 m_active_count;
    
    // Simple grid allocator state
    u32 m_cursor_x;
    u32 m_cursor_y;
    u32 m_row_height;
    
    // Helper: Find best quality level based on distance
    ShadowQuality DetermineQuality(f32 distance) const;
    
    // Helper: Pack rectangle into atlas
    bool PackRectangle(u32 width, u32 height, u32& out_x, u32& out_y);
};

} // namespace s0::renderer

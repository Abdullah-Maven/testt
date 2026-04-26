#include "renderer/ShadowAtlas.h"
#include "core/Logger.h"
#include <cmath>
#include <algorithm>

namespace s0::renderer {

ShadowAtlas::ShadowAtlas(rhi::GraphicsDevice* device) 
    : m_device(device)
    , m_active_count(0)
    , m_cursor_x(0)
    , m_cursor_y(0)
    , m_row_height(0)
{
    // Create the shadow atlas texture (4K depth texture)
    rhi::TextureDesc desc;
    desc.width = SHADOW_ATLAS_SIZE;
    desc.height = SHADOW_ATLAS_SIZE;
    desc.format = rhi::TextureFormat::DepthStencil;
    desc.is_depth_stencil = true;
    desc.is_render_target = true;
    
    m_atlas_texture = m_device->CreateTexture(desc);
    
    // Initialize slots
    m_slots.resize(MAX_SHADOWED_LIGHTS);
    for (auto& slot : m_slots) {
        slot.is_active = false;
        slot.light_index = 0xFFFFFFFF;
        slot.atlas_x = 0;
        slot.atlas_y = 0;
        slot.size = 0;
    }
    
    S0_LOG_INFO("ShadowAtlas created: " + std::to_string(SHADOW_ATLAS_SIZE) + "x" + 
                std::to_string(SHADOW_ATLAS_SIZE) + ", max " + std::to_string(MAX_SHADOWED_LIGHTS) + " lights");
}

ShadowAtlas::~ShadowAtlas() {
    // Textures auto-cleanup via unique_ptr
}

ShadowQuality ShadowAtlas::DetermineQuality(f32 distance) const {
    if (distance <= SHADOW_DISTANCE_LOD_1) {
        return ShadowQuality::High;
    } else if (distance <= SHADOW_DISTANCE_LOD_2) {
        return ShadowQuality::Medium;
    } else if (distance <= SHADOW_DISTANCE_LOD_3) {
        return ShadowQuality::Low;
    }
    return ShadowQuality::None;
}

bool ShadowAtlas::PackRectangle(u32 width, u32 height, u32& out_x, u32& out_y) {
    // Simple shelf packing algorithm (fast, good enough for dynamic allocation)
    if (m_cursor_x + width > SHADOW_ATLAS_SIZE) {
        // Move to next row
        m_cursor_x = 0;
        m_cursor_y += m_row_height;
        m_row_height = 0;
    }
    
    if (m_cursor_y + height > SHADOW_ATLAS_SIZE) {
        // Atlas is full
        return false;
    }
    
    out_x = m_cursor_x;
    out_y = m_cursor_y;
    
    m_cursor_x += width;
    m_row_height = std::max(m_row_height, height);
    
    return true;
}

i32 ShadowAtlas::AllocateShadow(u32 light_index, f32 light_distance) {
    ShadowQuality quality = DetermineQuality(light_distance);
    
    if (quality == ShadowQuality::None) {
        return -1; // No shadow needed
    }
    
    u32 size = static_cast<u32>(quality);
    
    // Find a free slot
    for (u32 i = 0; i < MAX_SHADOWED_LIGHTS; ++i) {
        if (!m_slots[i].is_active) {
            u32 atlas_x, atlas_y;
            
            if (!PackRectangle(size, size, atlas_x, atlas_y)) {
                S0_LOG_WARN("ShadowAtlas full, cannot allocate " + std::to_string(size) + "x" + std::to_string(size));
                return -1;
            }
            
            m_slots[i].is_active = true;
            m_slots[i].light_index = light_index;
            m_slots[i].atlas_x = atlas_x;
            m_slots[i].atlas_y = atlas_y;
            m_slots[i].size = size;
            
            m_active_count++;
            return static_cast<i32>(i);
        }
    }
    
    S0_LOG_WARN("ShadowAtlas: no free slots (max " + std::to_string(MAX_SHADOWED_LIGHTS) + ")");
    return -1;
}

void ShadowAtlas::FreeShadow(u32 slot_index) {
    if (slot_index >= MAX_SHADOWED_LIGHTS) {
        return;
    }
    
    if (m_slots[slot_index].is_active) {
        m_slots[slot_index].is_active = false;
        m_slots[slot_index].light_index = 0xFFFFFFFF;
        m_slots[slot_index].size = 0;
        m_active_count--;
        
        // Note: We don't defragment the atlas for performance reasons
        // In a production system, you might want to rebuild periodically
    }
}

void ShadowAtlas::BeginShadowPass() {
    // Setup for shadow rendering
    // This would typically bind the atlas as FBO and clear it
}

void ShadowAtlas::EndShadowPass(std::unique_ptr<rhi::CommandBuffer> cmd) {
    m_device->SubmitAndExecute(std::move(cmd));
}

Mat4 ShadowAtlas::GetShadowMatrix(u32 slot_index, const Vec3& light_pos, f32 radius) const {
    if (slot_index >= MAX_SHADOWED_LIGHTS || !m_slots[slot_index].is_active) {
        return Mat4::Identity();
    }
    
    const auto& slot = m_slots[slot_index];
    
    // Calculate orthographic projection for the shadow map
    f32 half_size = radius * 0.5f; // Simplified - in production, calculate from light frustum
    
    // View matrix (looking at center of illuminated area)
    // For point lights, you'd need a cube map or multiple passes
    Mat4 view = Mat4::LookAt(light_pos, Vec3(light_pos.x, light_pos.y - 1.0f, light_pos.z), Vec3(0, 0, 1));
    
    // Orthographic projection
    Mat4 proj = Mat4::Identity(); // Simplified - implement proper ortho matrix
    
    return proj * view;
}

} // namespace s0::renderer

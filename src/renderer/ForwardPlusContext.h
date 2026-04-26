#pragma once

#include "core/Types.h"
#include "rhi/GraphicsDevice.h"
#include "renderer/LightGrid.h"
#include "renderer/ShadowAtlas.h"
#include <memory>
#include <vector>

namespace s0::renderer {

// Forward+ Rendering Context
// Manages the complete Forward+ rendering pipeline:
// 1. Light grid construction (CPU or compute shader)
// 2. Shadow map rendering
// 3. G-Buffer setup (minimal for forward+)
// 4. Lighting pass with clustered shading

struct ForwardPlusConfig {
    u32 width = 1280;
    u32 height = 720;
    bool enable_shadows = true;
    bool enable_volumetric = false;
    u32 max_lights = MAX_TOTAL_LIGHTS;
};

class ForwardPlusContext {
public:
    ForwardPlusContext(rhi::GraphicsDevice* device, const ForwardPlusConfig& config);
    ~ForwardPlusContext();
    
    // Begin frame - reset per-frame resources
    void BeginFrame();
    
    // Setup view for rendering
    void SetView(const Mat4& view, const Mat4& projection, const Vec3& view_position);
    
    // Update lights and rebuild grid
    void UpdateLights(const cook::PackedLight* lights, u32 count);
    
    // Render shadow maps (called before main pass)
    void RenderShadows(rhi::CommandBuffer* cmd);
    
    // Get light grid for shader binding
    const LightGrid& GetLightGrid() const { return m_light_grid; }
    
    // Get shadow atlas
    ShadowAtlas* GetShadowAtlas() { return m_shadow_atlas.get(); }
    
    // Bind resources for forward pass
    void BindForForwardPass(rhi::CommandBuffer* cmd);

private:
    rhi::GraphicsDevice* m_device;
    ForwardPlusConfig m_config;
    
    LightGrid m_light_grid;
    std::unique_ptr<ShadowAtlas> m_shadow_atlas;
    
    // GPU buffers
    std::unique_ptr<rhi::Buffer> m_light_buffer;
    std::unique_ptr<rhi::Buffer> m_grid_cells_buffer;
    std::unique_ptr<rhi::Buffer> m_light_indices_buffer;
    
    // Pipeline handles
    u64 m_shadow_pipeline = 0;
    u64 m_forward_pipeline = 0;
    
    Mat4 m_view;
    Mat4 m_projection;
    Mat4 m_view_proj;
    Vec3 m_view_position;
    
    std::vector<cook::PackedLight> m_lights;
    u32 m_light_count = 0;
};

} // namespace s0::renderer

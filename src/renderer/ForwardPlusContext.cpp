#include "renderer/ForwardPlusContext.h"
#include "core/Logger.h"

namespace s0::renderer {

ForwardPlusContext::ForwardPlusContext(rhi::GraphicsDevice* device, const ForwardPlusConfig& config)
    : m_device(device)
    , m_config(config)
    , m_shadow_atlas(std::make_unique<ShadowAtlas>(device))
{
    // Create GPU buffers for light data
    
    // Light buffer (holds PackedLight array)
    rhi::BufferDesc light_desc;
    light_desc.size = sizeof(cook::PackedLight) * m_config.max_lights;
    light_desc.usage = rhi::BufferUsage::Uniform | rhi::BufferUsage::Storage;
    light_desc.is_dynamic = true;
    m_light_buffer = m_device->CreateBuffer(light_desc);
    
    // Grid cells buffer
    rhi::BufferDesc grid_desc;
    grid_desc.size = sizeof(LightGridCell) * m_light_grid.GetTotalCells();
    grid_desc.usage = rhi::BufferUsage::Uniform | rhi::BufferUsage::Storage;
    grid_desc.is_dynamic = true;
    m_grid_cells_buffer = m_device->CreateBuffer(grid_desc);
    
    // Light indices buffer
    rhi::BufferDesc indices_desc;
    indices_desc.size = sizeof(u32) * MAX_TOTAL_LIGHTS * 4; // Estimate
    indices_desc.usage = rhi::BufferUsage::Uniform | rhi::BufferUsage::Storage;
    indices_desc.is_dynamic = true;
    m_light_indices_buffer = m_device->CreateBuffer(indices_desc);
    
    S0_LOG_INFO("ForwardPlusContext created");
}

ForwardPlusContext::~ForwardPlusContext() = default;

void ForwardPlusContext::BeginFrame() {
    m_light_count = 0;
    m_light_grid.Reset();
}

void ForwardPlusContext::SetView(const Mat4& view, const Mat4& projection, const Vec3& view_position) {
    m_view = view;
    m_projection = projection;
    m_view_proj = projection * view;
    m_view_position = view_position;
}

void ForwardPlusContext::UpdateLights(const cook::PackedLight* lights, u32 count) {
    m_light_count = std::min(count, m_config.max_lights);
    
    // Copy lights to internal storage
    if (count > 0) {
        m_lights.resize(m_light_count);
        std::memcpy(m_lights.data(), lights, sizeof(cook::PackedLight) * m_light_count);
        
        // Update GPU buffer
        void* mapped = m_light_buffer->Map();
        if (mapped) {
            std::memcpy(mapped, m_lights.data(), sizeof(cook::PackedLight) * m_light_count);
            m_light_buffer->Unmap();
        }
        
        // Rebuild light grid
        m_light_grid.Build(m_view_position, m_view_proj, m_lights.data(), m_light_count);
        
        // Update grid cell buffer
        mapped = m_grid_cells_buffer->Map();
        if (mapped) {
            std::memcpy(mapped, m_light_grid.GetCells(), 
                       sizeof(LightGridCell) * m_light_grid.GetTotalCells());
            m_grid_cells_buffer->Unmap();
        }
        
        // Update light indices buffer
        mapped = m_light_indices_buffer->Map();
        if (mapped) {
            std::memcpy(mapped, m_light_grid.GetLightIndices(),
                       sizeof(u32) * m_light_grid.GetTotalLightIndices());
            m_light_indices_buffer->Unmap();
        }
    }
}

void ForwardPlusContext::RenderShadows(rhi::CommandBuffer* cmd) {
    if (!m_config.enable_shadows || !cmd) {
        return;
    }
    
    m_shadow_atlas->BeginShadowPass();
    
    // In a full implementation, we would:
    // 1. Bind shadow atlas as render target
    // 2. For each shadow-casting light:
    //    - Set up light's view/projection
    //    - Render depth-only pass of visible geometry
    // 3. Unbind and restore main render target
    
    // This is a stub - full implementation requires geometry rendering
}

void ForwardPlusContext::BindForForwardPass(rhi::CommandBuffer* cmd) {
    if (!cmd) return;
    
    // Bind light buffers
    cmd->BindUniformBuffer(0, m_light_buffer.get());
    cmd->BindUniformBuffer(1, m_grid_cells_buffer.get());
    cmd->BindUniformBuffer(2, m_light_indices_buffer.get());
    
    // Bind shadow atlas texture
    if (m_config.enable_shadows && m_shadow_atlas) {
        cmd->BindTexture(3, m_shadow_atlas->GetAtlasTexture());
    }
}

} // namespace s0::renderer

#include "engine/World.h"
#include "loader/S0DataLoader.h"
#include "core/Logger.h"

namespace s0::engine {

World::World() = default;

World::~World() {
    Unload();
}

bool World::Load(const std::string& path) {
    if (m_is_loaded) {
        Unload();
    }
    
    S0_LOG_INFO("Loading world: " + path);
    
    // Load S0_DATA file
    loader::S0DataFile s0_file;
    if (!loader::LoadS0Data(path, s0_file)) {
        S0_LOG_ERROR("Failed to load S0_DATA file: " + path);
        return false;
    }
    
    m_world_path = path;
    
    // Copy lights from loaded data
    m_lights = s0_file.lights;
    
    S0_LOG_INFO("World loaded: " + std::to_string(s0_file.vertices.size()) + 
                " vertices, " + std::to_string(s0_file.indices.size()) + 
                " indices, " + std::to_string(m_lights.size()) + " lights");
    
    m_is_loaded = true;
    return true;
}

void World::Unload() {
    m_lights.clear();
    m_light_grid.Reset();
    m_shadow_atlas.reset();
    m_is_loaded = false;
    
    S0_LOG_INFO("World unloaded");
}

void World::Update(f32 dt) {
    // Update physics, entities, etc.
    // For now, just rebuild light grid if needed
}

void World::Render(rhi::CommandBuffer* cmd) {
    if (!cmd || !m_is_loaded) {
        return;
    }
    
    // Set up view/projection matrices (would come from camera in real implementation)
    Vec3 eye(0.0f, 1.7f, 5.0f);
    Vec3 center(0.0f, 1.7f, 0.0f);
    Vec3 up(0.0f, 1.0f, 0.0f);
    
    Mat4 view = Mat4::LookAt(eye, center, up);
    Mat4 proj = Mat4::Perspective(90.0f * 3.14159f / 180.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    Mat4 view_proj = proj * view;
    
    // Build light grid for current view
    if (!m_lights.empty()) {
        m_light_grid.Build(eye, view_proj, m_lights.data(), static_cast<u32>(m_lights.size()));
    }
    
    // Render geometry here (would use ForwardPlusContext in full implementation)
    // For now, just log that we rendered
    // S0_LOG_DEBUG("World rendered with " + std::to_string(m_light_grid.GetTotalLightIndices()) + " light-cell assignments");
}

} // namespace s0::engine

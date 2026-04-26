#pragma once

#include "core/Types.h"
#include "rhi/GraphicsDevice.h"
#include "renderer/LightGrid.h"
#include "renderer/ShadowAtlas.h"
#include <memory>
#include <vector>
#include <string>

namespace s0::engine {

class World {
public:
    World();
    ~World();
    
    bool Load(const std::string& path);
    void Unload();
    
    void Update(f32 dt);
    void Render(rhi::CommandBuffer* cmd);
    
    // Accessors
    const renderer::LightGrid& GetLightGrid() const { return m_light_grid; }
    const std::vector<cook::PackedLight>& GetLights() const { return m_lights; }
    
private:
    std::string m_world_path;
    std::vector<cook::PackedLight> m_lights;
    renderer::LightGrid m_light_grid;
    std::unique_ptr<renderer::ShadowAtlas> m_shadow_atlas;
    
    bool m_is_loaded = false;
};

} // namespace s0::engine

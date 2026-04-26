#include "renderer/LightGrid.h"
#include <cmath>
#include <cstring>

namespace s0::renderer {

LightGrid::LightGrid() 
    : m_cells(GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z)
    , m_light_indices(MAX_TOTAL_LIGHTS * MAX_LIGHTS_PER_CELL)
    , m_total_light_indices(0) 
{
    Reset();
}

LightGrid::~LightGrid() = default;

void LightGrid::Reset() {
    std::memset(m_cells.data(), 0, m_cells.size() * sizeof(LightGridCell));
    m_total_light_indices = 0;
}

void LightGrid::Build(const Vec3& view_pos, const Mat4& view_proj,
                      const cook::PackedLight* lights, u32 light_count) 
{
    Reset();
    
    // For each light, determine which grid cells it affects
    for (u32 i = 0; i < light_count; ++i) {
        const auto& light = lights[i];
        
        // Skip if light doesn't intersect view frustum
        Vec3 light_pos(light.position[0], light.position[1], light.position[2]);
        if (!IntersectsFrustum(light_pos, light.radius, view_proj)) {
            continue;
        }
        
        // Calculate bounding box of light in grid space
        f32 inv_cell_size_x = GRID_SIZE_X / (light.radius * 2.0f);
        f32 inv_cell_size_y = GRID_SIZE_Y / (light.radius * 2.0f);
        f32 inv_cell_size_z = GRID_SIZE_Z / (light.radius * 2.0f);
        
        i32 min_x = std::max(0, (i32)((light_pos.x - light.radius) * inv_cell_size_x));
        i32 max_x = std::min((i32)GRID_SIZE_X, (i32)((light_pos.x + light.radius) * inv_cell_size_x) + 1);
        i32 min_y = std::max(0, (i32)((light_pos.y - light.radius) * inv_cell_size_y));
        i32 max_y = std::min((i32)GRID_SIZE_Y, (i32)((light_pos.y + light.radius) * inv_cell_size_y) + 1);
        i32 min_z = std::max(0, (i32)((light_pos.z - light.radius) * inv_cell_size_z));
        i32 max_z = std::min((i32)GRID_SIZE_Z, (i32)((light_pos.z + light.radius) * inv_cell_size_z) + 1);
        
        // Add light to all intersected cells
        for (i32 x = min_x; x < max_x; ++x) {
            for (i32 y = min_y; y < max_y; ++y) {
                for (i32 z = min_z; z < max_z; ++z) {
                    u32 cell_idx = x + y * GRID_SIZE_X + z * GRID_SIZE_X * GRID_SIZE_Y;
                    auto& cell = m_cells[cell_idx];
                    
                    if (cell.light_count < MAX_LIGHTS_PER_CELL) {
                        m_light_indices[m_total_light_indices++] = i;
                        cell.light_offset = m_total_light_indices - 1;
                        cell.light_count++;
                    }
                }
            }
        }
    }
}

bool LightGrid::IntersectsFrustum(const Vec3& center, f32 radius, const Mat4& view_proj) const {
    // Transform light center to clip space
    f32 w = view_proj.m[3]*center.x + view_proj.m[7]*center.y + view_proj.m[11]*center.z + view_proj.m[15];
    f32 x = view_proj.m[0]*center.x + view_proj.m[4]*center.y + view_proj.m[8]*center.z + view_proj.m[12];
    f32 y = view_proj.m[1]*center.x + view_proj.m[5]*center.y + view_proj.m[9]*center.z + view_proj.m[13];
    f32 z = view_proj.m[2]*center.x + view_proj.m[6]*center.y + view_proj.m[10]*center.z + view_proj.m[14];
    
    // Simple sphere-frustum test (conservative)
    f32 margin = radius * 0.5f;
    return (x + margin > -w && x - margin < w &&
            y + margin > -w && y - margin < w &&
            z + margin > -w && z - margin < w &&
            w > 0.0f);
}

void LightGrid::CalculateCellBounds(u32 x, u32 y, u32 z, Vec3& min, Vec3& max) const {
    // Placeholder: implement based on scene bounds
    // For now, assume unit grid centered at origin
    f32 cell_size = 1.0f;
    min = Vec3(x * cell_size, y * cell_size, z * cell_size);
    max = Vec3((x+1) * cell_size, (y+1) * cell_size, (z+1) * cell_size);
}

} // namespace s0::renderer

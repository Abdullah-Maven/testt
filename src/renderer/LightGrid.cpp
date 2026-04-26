#include "renderer/LightGrid.h"
#include <cstring>
#include <cmath>

namespace s0::renderer {

LightGrid::LightGrid() 
    : m_total_light_indices(0)
{
    m_cells.resize(GetTotalCells());
    m_light_indices.reserve(MAX_TOTAL_LIGHTS * 4); // Estimate
}

LightGrid::~LightGrid() = default;

void LightGrid::Reset() {
    for (auto& cell : m_cells) {
        cell.light_offset = 0;
        cell.light_count = 0;
    }
    m_light_indices.clear();
    m_total_light_indices = 0;
}

bool LightGrid::IntersectsFrustum(const Vec3& center, f32 radius, const Mat4& view_proj) const {
    // Simplified frustum culling - in production, extract frustum planes
    // and test sphere against them
    return true; // Assume visible for now
}

void LightGrid::CalculateCellBounds(u32 x, u32 y, u32 z, Vec3& min, Vec3& max) const {
    // Define grid bounds in world space (configurable based on level size)
    constexpr f32 GRID_WORLD_MIN_X = -100.0f;
    constexpr f32 GRID_WORLD_MAX_X = 100.0f;
    constexpr f32 GRID_WORLD_MIN_Y = 0.0f;
    constexpr f32 GRID_WORLD_MAX_Y = 50.0f;
    constexpr f32 GRID_WORLD_MIN_Z = -100.0f;
    constexpr f32 GRID_WORLD_MAX_Z = 100.0f;
    
    f32 cell_size_x = (GRID_WORLD_MAX_X - GRID_WORLD_MIN_X) / static_cast<f32>(GRID_SIZE_X);
    f32 cell_size_y = (GRID_WORLD_MAX_Y - GRID_WORLD_MIN_Y) / static_cast<f32>(GRID_SIZE_Y);
    f32 cell_size_z = (GRID_WORLD_MAX_Z - GRID_WORLD_MIN_Z) / static_cast<f32>(GRID_SIZE_Z);
    
    min.x = GRID_WORLD_MIN_X + x * cell_size_x;
    min.y = GRID_WORLD_MIN_Y + y * cell_size_y;
    min.z = GRID_WORLD_MIN_Z + z * cell_size_z;
    
    max.x = min.x + cell_size_x;
    max.y = min.y + cell_size_y;
    max.z = min.z + cell_size_z;
}

void LightGrid::Build(const Vec3& view_pos, const Mat4& view_proj, 
                      const cook::PackedLight* lights, u32 light_count) {
    Reset();
    
    // Temporary storage for per-cell light assignments
    std::vector<std::vector<u32>> cell_lights(GetTotalCells());
    
    // Assign lights to cells
    for (u32 i = 0; i < light_count; ++i) {
        const auto& light = lights[i];
        
        // Skip if light doesn't intersect view frustum
        Vec3 light_center(light.position[0], light.position[1], light.position[2]);
        if (!IntersectsFrustum(light_center, light.radius, view_proj)) {
            continue;
        }
        
        // Find all cells this light overlaps
        // Simplified: calculate which cell the light center is in, then check neighbors
        constexpr f32 GRID_WORLD_MIN_X = -100.0f;
        constexpr f32 GRID_WORLD_MAX_X = 100.0f;
        constexpr f32 GRID_WORLD_MIN_Y = 0.0f;
        constexpr f32 GRID_WORLD_MAX_Y = 50.0f;
        constexpr f32 GRID_WORLD_MIN_Z = -100.0f;
        constexpr f32 GRID_WORLD_MAX_Z = 100.0f;
        
        f32 cell_size_x = (GRID_WORLD_MAX_X - GRID_WORLD_MIN_X) / static_cast<f32>(GRID_SIZE_X);
        f32 cell_size_y = (GRID_WORLD_MAX_Y - GRID_WORLD_MIN_Y) / static_cast<f32>(GRID_SIZE_Y);
        f32 cell_size_z = (GRID_WORLD_MAX_Z - GRID_WORLD_MIN_Z) / static_cast<f32>(GRID_SIZE_Z);
        
        // Calculate cell containing light center
        i32 center_x = static_cast<i32>((light.position[0] - GRID_WORLD_MIN_X) / cell_size_x);
        i32 center_y = static_cast<i32>((light.position[1] - GRID_WORLD_MIN_Y) / cell_size_y);
        i32 center_z = static_cast<i32>((light.position[2] - GRID_WORLD_MIN_Z) / cell_size_z);
        
        // Clamp to grid bounds
        center_x = std::clamp(center_x, 0, static_cast<i32>(GRID_SIZE_X) - 1);
        center_y = std::clamp(center_y, 0, static_cast<i32>(GRID_SIZE_Y) - 1);
        center_z = std::clamp(center_z, 0, static_cast<i32>(GRID_SIZE_Z) - 1);
        
        // Determine extent of light influence in cells
        i32 extent_x = static_cast<i32>(std::ceil(light.radius / cell_size_x));
        i32 extent_y = static_cast<i32>(std::ceil(light.radius / cell_size_y));
        i32 extent_z = static_cast<i32>(std::ceil(light.radius / cell_size_z));
        
        // Add light to all overlapping cells
        for (i32 dz = -extent_z; dz <= extent_z; ++dz) {
            for (i32 dy = -extent_y; dy <= extent_y; ++dy) {
                for (i32 dx = -extent_x; dx <= extent_x; ++dx) {
                    i32 cx = center_x + dx;
                    i32 cy = center_y + dy;
                    i32 cz = center_z + dz;
                    
                    if (cx >= 0 && cx < static_cast<i32>(GRID_SIZE_X) &&
                        cy >= 0 && cy < static_cast<i32>(GRID_SIZE_Y) &&
                        cz >= 0 && cz < static_cast<i32>(GRID_SIZE_Z)) {
                        
                        u32 cell_index = cx + cy * GRID_SIZE_X + cz * GRID_SIZE_X * GRID_SIZE_Y;
                        
                        // Avoid duplicates
                        bool already_added = false;
                        for (u32 existing : cell_lights[cell_index]) {
                            if (existing == i) {
                                already_added = true;
                                break;
                            }
                        }
                        
                        if (!already_added && cell_lights[cell_index].size() < MAX_LIGHTS_PER_CELL) {
                            cell_lights[cell_index].push_back(i);
                        }
                    }
                }
            }
        }
    }
    
    // Pack light indices into contiguous array
    u32 current_offset = 0;
    for (u32 cell_idx = 0; cell_idx < GetTotalCells(); ++cell_idx) {
        m_cells[cell_idx].light_offset = current_offset;
        m_cells[cell_idx].light_count = static_cast<u32>(cell_lights[cell_idx].size());
        
        for (u32 light_idx : cell_lights[cell_idx]) {
            m_light_indices.push_back(light_idx);
        }
        
        current_offset += cell_lights[cell_idx].size();
    }
    
    m_total_light_indices = static_cast<u32>(m_light_indices.size());
}

} // namespace s0::renderer

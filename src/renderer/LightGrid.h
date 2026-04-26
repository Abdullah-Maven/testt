#pragma once

#include <cstdint>
#include <memory>
#include "core/Types.h"
#include "loader/S0DataFormat.h"

namespace s0::renderer {

// ------------------------------------------------------------------
// Forward+ Light Grid Configuration
// Optimized for corridor-based indoor environments
// ------------------------------------------------------------------
constexpr u32 GRID_SIZE_X = 16;
constexpr u32 GRID_SIZE_Y = 8;   // Lower Y resolution for typical ceiling heights
constexpr u32 GRID_SIZE_Z = 16;
constexpr u32 MAX_LIGHTS_PER_CELL = 64; // Corridor scenes have limited light overlap
constexpr u32 MAX_TOTAL_LIGHTS = 1024;

// ------------------------------------------------------------------
// Light Grid Cell (stored in GPU buffer)
// ------------------------------------------------------------------
struct LightGridCell {
    u32 light_offset;     // Offset into global light index buffer
    u32 light_count;      // Number of lights affecting this cell
    u32 padding[2];       // Alignment
};

// ------------------------------------------------------------------
// Clustered Light Culling System
// Uses uniform 3D grid for fast frustum-light intersection tests
// ------------------------------------------------------------------
class LightGrid {
public:
    LightGrid();
    ~LightGrid();
    
    // Build the grid from active lights and view frustum
    void Build(const Vec3& view_pos, const Mat4& view_proj, 
               const cook::PackedLight* lights, u32 light_count);
    
    // Get grid dimensions
    u32 GetGridSizeX() const { return GRID_SIZE_X; }
    u32 GetGridSizeY() const { return GRID_SIZE_Y; }
    u32 GetGridSizeZ() const { return GRID_SIZE_Z; }
    u32 GetTotalCells() const { return GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z; }
    
    // Access grid data (for GPU upload)
    const LightGridCell* GetCells() const { return m_cells.data(); }
    const u32* GetLightIndices() const { return m_light_indices.data(); }
    u32 GetTotalLightIndices() const { return m_total_light_indices; }
    
    // Clear and reset
    void Reset();

private:
    std::vector<LightGridCell> m_cells;
    std::vector<u32> m_light_indices;
    u32 m_total_light_indices;
    
    // Helper: Test light AABB against frustum
    bool IntersectsFrustum(const Vec3& center, f32 radius, const Mat4& view_proj) const;
    
    // Helper: Calculate cell bounds in world space
    void CalculateCellBounds(u32 x, u32 y, u32 z, Vec3& min, Vec3& max) const;
};

} // namespace s0::renderer

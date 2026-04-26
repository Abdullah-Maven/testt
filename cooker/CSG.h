#pragma once

#include <vector>
#include "core/Types.h"
#include "loader/S0DataFormat.h"

namespace s0::cook {

// ------------------------------------------------------------------
// CSG (Constructive Solid Geometry) Processor
// Converts brush definitions into optimized triangle meshes
// Uses BSP-based CSG for robust boolean operations
// ------------------------------------------------------------------
class CSGProcessor {
public:
    CSGProcessor();
    ~CSGProcessor();
    
    // Process brushes and generate mesh
    bool Process(const std::vector<MapBrush>& brushes);
    
    // Get generated geometry
    const std::vector<PackedVertex>& GetVertices() const { return m_vertices; }
    const std::vector<u32>& GetIndices() const { return m_indices; }
    
    // Get collision data (simplified convex hulls)
    const std::vector<std::vector<f32>>& GetCollisionVertices() const { return m_collision_vertices; }
    
    // Statistics
    u64 GetVertexCount() const { return m_vertices.size(); }
    u64 GetIndexCount() const { return m_indices.size(); }
    
    // Clear generated data
    void Clear();

private:
    std::vector<PackedVertex> m_vertices;
    std::vector<u32> m_indices;
    std::vector<std::vector<f32>> m_collision_vertices;
    
    // CSG helpers
    struct Plane {
        f32 a, b, c, d;
    };
    
    struct Vertex {
        f32 x, y, z;
        f32 u, v;
        f32 nx, ny, nz;
    };
    
    struct Polygon {
        std::vector<Vertex> vertices;
        Plane plane;
    };
    
    // Clip polygon against plane
    std::vector<Polygon> ClipPolygon(const Polygon& poly, const Plane& clip_plane, bool keep_front);
    
    // Generate polygon from brush planes
    Polygon GeneratePolygonFromBrush(const std::vector<Plane>& brush_planes, size_t plane_idx);
    
    // Triangulate polygon
    void TriangulatePolygon(const Polygon& poly, std::vector<Vertex>& out_vertices, std::vector<u32>& out_indices, u32 index_offset);
    
    // Convert to packed vertex format
    PackedVertex PackVertex(const Vertex& v);
};

} // namespace s0::cook

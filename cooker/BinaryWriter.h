#pragma once

#include <string>
#include <vector>
#include "core/Types.h"
#include "loader/S0DataFormat.h"

namespace s0::cook {

// ------------------------------------------------------------------
// Binary Writer for S0_DATA Format
// ------------------------------------------------------------------
class BinaryWriter {
public:
    explicit BinaryWriter(const std::string& output_path);
    ~BinaryWriter();
    
    // Start a new file
    void Begin();
    
    // Add geometry data
    void AddVertex(const PackedVertex& vertex);
    void AddIndex(u32 index);
    
    // Add lighting data
    void AddLight(const PackedLight& light);
    
    // Add collision data
    void AddCollisionHull(const f32* vertices, u32 count);
    
    // Add entity data
    void AddEntity(const MapEntity& entity);
    
    // Write everything to disk
    bool Write();
    
    // Statistics
    u64 GetVertexCount() const;
    u64 GetIndexCount() const;
    u64 GetLightCount() const;

private:
    std::string m_output_path;
    std::vector<PackedVertex> m_vertices;
    std::vector<u32> m_indices;
    std::vector<PackedLight> m_lights;
    std::vector<u8> m_collision_data;
    std::vector<u8> m_entity_data;
    u64 m_entity_count = 0;
    u64 m_current_offset;
};

} // namespace s0::cook

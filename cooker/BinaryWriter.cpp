#include "cooker/BinaryWriter.h"
#include "loader/S0DataFormat.h"
#include <fstream>
#include <cstring>

namespace s0::cook {

BinaryWriter::BinaryWriter(const std::string& output_path) 
    : m_output_path(output_path)
    , m_current_offset(0) 
{
}

BinaryWriter::~BinaryWriter() = default;

void BinaryWriter::Begin() {
    m_vertices.clear();
    m_indices.clear();
    m_lights.clear();
    m_collision_data.clear();
    m_entity_data.clear();
    m_current_offset = 0;
}

void BinaryWriter::AddVertex(const PackedVertex& vertex) {
    m_vertices.push_back(vertex);
}

void BinaryWriter::AddIndex(u32 index) {
    m_indices.push_back(index);
}

void BinaryWriter::AddLight(const PackedLight& light) {
    m_lights.push_back(light);
}

void BinaryWriter::AddCollisionHull(const f32* vertices, u32 count) {
    CollisionHullHeader header;
    header.vertex_count = count;
    header.padding = 0;
    
    m_collision_data.insert(m_collision_data.end(), 
                            reinterpret_cast<const u8*>(&header),
                            reinterpret_cast<const u8*>(&header) + sizeof(header));
    m_collision_data.insert(m_collision_data.end(),
                            reinterpret_cast<const u8*>(vertices),
                            reinterpret_cast<const u8*>(vertices) + count * 3 * sizeof(f32));
}

void BinaryWriter::AddEntity(const MapEntity& entity) {
    EntityHeader header;
    header.key_value_pairs = static_cast<u32>(entity.properties.size()) + 1; // +1 for classname
    
    // Calculate total string bytes
    u32 total_bytes = 0;
    total_bytes += static_cast<u32>(entity.classname.size()) + 1;
    
    for (const auto& prop : entity.properties) {
        total_bytes += static_cast<u32>(prop.first.size()) + 1;
        total_bytes += static_cast<u32>(prop.second.size()) + 1;
    }
    
    header.total_string_bytes = total_bytes;
    
    m_entity_data.insert(m_entity_data.end(),
                         reinterpret_cast<const u8*>(&header),
                         reinterpret_cast<const u8*>(&header) + sizeof(header));
    
    // Write classname
    m_entity_data.insert(m_entity_data.end(),
                         entity.classname.begin(),
                         entity.classname.end());
    m_entity_data.push_back('\0');
    
    // Write properties
    for (const auto& prop : entity.properties) {
        m_entity_data.insert(m_entity_data.end(),
                             prop.first.begin(),
                             prop.first.end());
        m_entity_data.push_back('\0');
        
        m_entity_data.insert(m_entity_data.end(),
                             prop.second.begin(),
                             prop.second.end());
        m_entity_data.push_back('\0');
    }
    
    m_entity_count++;
}

bool BinaryWriter::Write() {
    std::ofstream file(m_output_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Calculate offsets
    u64 header_size = sizeof(S0Header);
    u64 vertex_offset = header_size;
    u64 vertex_size = m_vertices.size() * sizeof(PackedVertex);
    
    u64 index_offset = vertex_offset + vertex_size;
    u64 index_size = m_indices.size() * sizeof(u32);
    
    u64 light_offset = index_offset + index_size;
    u64 light_size = m_lights.size() * sizeof(PackedLight);
    
    u64 collision_offset = light_offset + light_size;
    u64 collision_size = m_collision_data.size();
    
    u64 entity_offset = collision_offset + collision_size;
    u64 entity_size = m_entity_data.size();
    
    u64 total_size = entity_offset + entity_size;
    
    // Write header
    S0Header header;
    header.magic = S0_MAGIC;
    header.version = S0_VERSION;
    header.total_file_size = total_size;
    header.vertex_data_offset = vertex_offset;
    header.index_data_offset = index_offset;
    header.light_data_offset = light_offset;
    header.collision_offset = collision_offset;
    header.entity_count = m_entity_count;
    header.entity_data_offset = entity_offset;
    
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    
    // Write vertex data
    if (!m_vertices.empty()) {
        file.write(reinterpret_cast<const char*>(m_vertices.data()), vertex_size);
    }
    
    // Write index data
    if (!m_indices.empty()) {
        file.write(reinterpret_cast<const char*>(m_indices.data()), index_size);
    }
    
    // Write light data
    if (!m_lights.empty()) {
        file.write(reinterpret_cast<const char*>(m_lights.data()), light_size);
    }
    
    // Write collision data
    if (!m_collision_data.empty()) {
        file.write(reinterpret_cast<const char*>(m_collision_data.data()), collision_size);
    }
    
    // Write entity data
    if (!m_entity_data.empty()) {
        file.write(reinterpret_cast<const char*>(m_entity_data.data()), entity_size);
    }
    
    file.close();
    return true;
}

u64 BinaryWriter::GetVertexCount() const {
    return m_vertices.size();
}

u64 BinaryWriter::GetIndexCount() const {
    return m_indices.size();
}

u64 BinaryWriter::GetLightCount() const {
    return m_lights.size();
}

} // namespace s0::cook

#include "loader/S0DataLoader.h"
#include "core/Logger.h"
#include <fstream>
#include <cstring>

namespace s0::loader {

bool LoadS0Data(const std::string& path, S0DataFile& out_data) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        S0_LOG_ERROR("Cannot open file: " + path);
        return false;
    }
    
    // Read header
    cook::S0Header header;
    file.read(reinterpret_cast<char*>(&header), sizeof(cook::S0Header));
    
    if (header.magic != cook::S0_MAGIC) {
        S0_LOG_ERROR("Invalid S0_DATA magic number");
        return false;
    }
    
    if (header.version != cook::S0_VERSION) {
        S0_LOG_ERROR("Unsupported S0_DATA version: " + std::to_string(header.version));
        return false;
    }
    
    // Calculate counts from offsets
    u64 index_count = (header.light_data_offset - header.index_data_offset) / sizeof(u32);
    u64 light_count = (header.collision_offset - header.light_data_offset) / sizeof(cook::PackedLight);
    
    // Read vertices
    file.seekg(static_cast<std::streamoff>(header.vertex_data_offset));
    u64 vertex_count = (header.index_data_offset - header.vertex_data_offset) / sizeof(cook::PackedVertex);
    out_data.vertices.resize(vertex_count);
    file.read(reinterpret_cast<char*>(out_data.vertices.data()), 
              static_cast<std::streamsize>(vertex_count * sizeof(cook::PackedVertex)));
    
    // Read indices
    file.seekg(static_cast<std::streamoff>(header.index_data_offset));
    out_data.indices.resize(index_count);
    file.read(reinterpret_cast<char*>(out_data.indices.data()), 
              static_cast<std::streamsize>(index_count * sizeof(u32)));
    
    // Read lights
    file.seekg(static_cast<std::streamoff>(header.light_data_offset));
    out_data.lights.resize(light_count);
    file.read(reinterpret_cast<char*>(out_data.lights.data()), 
              static_cast<std::streamsize>(light_count * sizeof(cook::PackedLight)));
    
    // Read collision hulls (simplified - would need proper parsing in production)
    // For now, skip collision data
    
    // Read entities
    file.seekg(static_cast<std::streamoff>(header.entity_data_offset));
    for (u64 i = 0; i < header.entity_count; ++i) {
        u32 key_len, value_len;
        file.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));
        
        std::string key(key_len, '\0');
        file.read(&key[0], key_len);
        
        file.read(reinterpret_cast<char*>(&value_len), sizeof(value_len));
        std::string value(value_len, '\0');
        file.read(&value[0], value_len);
        
        out_data.entities.emplace_back(std::move(key), std::move(value));
    }
    
    S0_LOG_INFO("Loaded S0_DATA: " + std::to_string(vertex_count) + " vertices, " +
                std::to_string(index_count) + " indices, " + 
                std::to_string(light_count) + " lights, " +
                std::to_string(header.entity_count) + " entities");
    
    return true;
}

bool SaveS0Data(const std::string& path, const S0DataFile& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        S0_LOG_ERROR("Cannot create file: " + path);
        return false;
    }
    
    cook::S0Header header;
    header.magic = cook::S0_MAGIC;
    header.version = cook::S0_VERSION;
    
    // Calculate offsets
    u64 offset = sizeof(cook::S0Header);
    header.vertex_data_offset = offset;
    offset += data.vertices.size() * sizeof(cook::PackedVertex);
    
    header.index_data_offset = offset;
    offset += data.indices.size() * sizeof(u32);
    
    header.light_data_offset = offset;
    offset += data.lights.size() * sizeof(cook::PackedLight);
    
    header.collision_offset = offset;
    // Collision data would go here
    
    header.entity_count = data.entities.size();
    header.entity_data_offset = offset; // Simplified - would need actual calculation
    
    // Calculate total size
    header.total_file_size = offset;
    for (const auto& kv : data.entities) {
        header.total_file_size += sizeof(u32) * 2 + kv.first.size() + kv.second.size();
    }
    
    // Write header
    file.write(reinterpret_cast<const char*>(&header), sizeof(cook::S0Header));
    
    // Write vertices
    file.write(reinterpret_cast<const char*>(data.vertices.data()), 
               static_cast<std::streamsize>(data.vertices.size() * sizeof(cook::PackedVertex)));
    
    // Write indices
    file.write(reinterpret_cast<const char*>(data.indices.data()), 
               static_cast<std::streamsize>(data.indices.size() * sizeof(u32)));
    
    // Write lights
    file.write(reinterpret_cast<const char*>(data.lights.data()), 
               static_cast<std::streamsize>(data.lights.size() * sizeof(cook::PackedLight)));
    
    // Write entities
    for (const auto& kv : data.entities) {
        u32 key_len = static_cast<u32>(kv.first.size());
        file.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
        file.write(kv.first.c_str(), key_len);
        
        u32 value_len = static_cast<u32>(kv.second.size());
        file.write(reinterpret_cast<const char*>(&value_len), sizeof(value_len));
        file.write(kv.second.c_str(), value_len);
    }
    
    S0_LOG_INFO("Saved S0_DATA: " + path);
    return true;
}

} // namespace s0::loader

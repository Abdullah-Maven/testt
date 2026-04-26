#pragma once

#include "loader/S0DataFormat.h"
#include <vector>
#include <string>
#include <memory>

namespace s0::loader {

// Loaded data structure for runtime use
struct S0DataFile {
    std::vector<cook::PackedVertex> vertices;
    std::vector<u32> indices;  // Always converted to u32 for simplicity
    std::vector<cook::PackedLight> lights;
    std::vector<std::vector<Vec3>> collision_hulls;  // One hull per brush
    std::vector<std::pair<std::string, std::string>> entities;  // Key-value pairs
};

// Load S0_DATA file from disk
bool LoadS0Data(const std::string& path, S0DataFile& out_data);

// Save S0_DATA file (used by cooker)
bool SaveS0Data(const std::string& path, const S0DataFile& data);

} // namespace s0::loader

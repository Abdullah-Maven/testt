#pragma once

#include <string>
#include <vector>
#include "loader/S0DataFormat.h"

namespace s0::cook {

// ------------------------------------------------------------------
// TrenchBroom .map File Parser
// Parses brush definitions and entity data from Quake-style .map files
// ------------------------------------------------------------------
class MapParser {
public:
    MapParser();
    ~MapParser();
    
    // Parse a .map file
    bool Parse(const std::string& filepath);
    
    // Access parsed data
    const std::vector<MapBrush>& GetBrushes() const { return m_brushes; }
    const std::vector<MapEntity>& GetEntities() const { return m_entities; }
    u64 GetBrushCount() const { return m_brushes.size(); }
    u64 GetEntityCount() const { return m_entities.size(); }
    
    // Clear parsed data
    void Clear();

private:
    std::vector<MapBrush> m_brushes;
    std::vector<MapEntity> m_entities;
    
    // Parsing helpers
    bool ParseBrush(const std::string& brush_data);
    bool ParseEntity(const std::string& entity_data);
    Vec3 ParseVector(const std::string& vec_str);
    std::vector<std::string> SplitLines(const std::string& content);
};

} // namespace s0::cook

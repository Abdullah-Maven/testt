#include "cooker/BinaryWriter.h"
#include "cooker/MapParser.h"
#include "cooker/CSG.h"
#include "core/Types.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: s0_cooker <input.map> <output.s0data>\n";
        std::cout << "System-0 Asset Cooker - Converts TrenchBroom .map files to optimized S0_DATA format\n";
        return 1;
    }
    
    std::string input_path = argv[1];
    std::string output_path = argv[2];
    
    std::cout << "[INFO] System-0 Asset Cooker\n";
    std::cout << "[INFO] Input:  " << input_path << "\n";
    std::cout << "[INFO] Output: " << output_path << "\n";
    
    // Parse the map file
    s0::cook::MapParser parser;
    if (!parser.Parse(input_path)) {
        std::cerr << "[ERROR] Failed to parse map file\n";
        return 1;
    }
    
    std::cout << "[INFO] Parsed " << parser.GetBrushCount() << " brushes and " 
              << parser.GetEntityCount() << " entities\n";
    
    // Run CSG operations to generate world geometry
    s0::cook::CSGProcessor csg;
    if (!csg.Process(parser.GetBrushes())) {
        std::cerr << "[ERROR] CSG processing failed\n";
        return 1;
    }
    
    std::cout << "[INFO] CSG generated " << csg.GetVertexCount() << " vertices and "
              << csg.GetIndexCount() << " indices\n";
    
    // Write binary output
    s0::cook::BinaryWriter writer(output_path);
    writer.Begin();
    
    // Add geometry
    const auto& vertices = csg.GetVertices();
    for (const auto& v : vertices) {
        writer.AddVertex(v);
    }
    
    const auto& indices = csg.GetIndices();
    for (auto idx : indices) {
        writer.AddIndex(idx);
    }
    
    // Add lights from entities
    for (const auto& entity : parser.GetEntities()) {
        if (entity.classname == "light" || entity.classname == "light_spot") {
            s0::cook::PackedLight light;
            light.position[0] = entity.origin.x;
            light.position[1] = entity.origin.y;
            light.position[2] = entity.origin.z;
            
            // Parse properties
            light.radius = 10.0f;
            light.color[0] = 1.0f;
            light.color[1] = 1.0f;
            light.color[2] = 1.0f;
            light.intensity = 100.0f;
            light.type = (entity.classname == "light_spot") 
                ? s0::cook::LightType::Spot 
                : s0::cook::LightType::Point;
            light.casts_shadow = 1;
            light.spot_inner_angle = 0.5f;
            light.spot_outer_angle = 0.6f;
            
            for (const auto& prop : entity.properties) {
                if (prop.first == "_radius") light.radius = std::stof(prop.second);
                if (prop.first == "_color") {
                    // Parse "r g b" format
                    sscanf(prop.second.c_str(), "%f %f %f", &light.color[0], &light.color[1], &light.color[2]);
                }
                if (prop.first == "_shadow") light.casts_shadow = (prop.second == "1") ? 1 : 0;
            }
            
            writer.AddLight(light);
        }
        
        // Add all entities (including triggers, player starts, etc.)
        writer.AddEntity(entity);
    }
    
    // Add collision hulls
    const auto& collision_vertices = csg.GetCollisionVertices();
    for (const auto& hull : collision_vertices) {
        writer.AddCollisionHull(hull.data(), static_cast<u32>(hull.size() / 3));
    }
    
    if (!writer.Write()) {
        std::cerr << "[ERROR] Failed to write output file\n";
        return 1;
    }
    
    std::cout << "[INFO] Successfully cooked level:\n";
    std::cout << "  - Vertices: " << writer.GetVertexCount() << "\n";
    std::cout << "  - Indices:  " << writer.GetIndexCount() << "\n";
    std::cout << "  - Lights:   " << writer.GetLightCount() << "\n";
    std::cout << "  - Entities: " << parser.GetEntityCount() << "\n";
    
    return 0;
}

#pragma once

#include "core/Types.h"
#include "rhi/GraphicsDevice.h"
#include "engine/World.h"
#include <memory>
#include <string>

struct SDL_Window;
typedef void* SDL_GLContext;

namespace s0::engine {

struct ApplicationConfig {
    std::string title = "System-0 Engine";
    u32 width = 1280;
    u32 height = 720;
    bool vsync = true;
    bool fullscreen = false;
};

class Application {
public:
    Application();
    virtual ~Application();
    
    static Application* GetInstance() { return s_instance; }
    
    int Run();
    void Shutdown();
    
    rhi::GraphicsDevice* GetDevice() const { return m_device.get(); }
    World* GetWorld() const { return m_world.get(); }
    
    void LoadWorld(const std::string& world_path);
    
protected:
    virtual bool Initialize(const ApplicationConfig& config);
    virtual void OnResize(u32 width, u32 height);
    virtual void OnUpdate(f32 dt);
    virtual void OnRender();
    
    f32 CalculateDeltaTime();
    
    ApplicationConfig m_config;
    std::unique_ptr<rhi::GraphicsDevice> m_device;
    std::unique_ptr<World> m_world;
    
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_gl_context = nullptr;
    
private:
    static Application* s_instance;
};

} // namespace s0::engine

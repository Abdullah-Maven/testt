#include "engine/Application.h"
#include "core/Logger.h"
#include "core/Memory.h"
#include <SDL2/SDL.h>

namespace s0::engine {

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

Application::~Application() {
    s_instance = nullptr;
}

bool Application::Initialize(const ApplicationConfig& config) {
    m_config = config;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        S0_LOG_ERROR("SDL initialization failed: " + std::string(SDL_GetError()));
        return false;
    }
    
    // Create window
    Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    m_window = SDL_CreateWindow(
        m_config.title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(m_config.width),
        static_cast<int>(m_config.height),
        window_flags
    );
    
    if (!m_window) {
        S0_LOG_ERROR("Failed to create window: " + std::string(SDL_GetError()));
        return false;
    }
    
    m_gl_context = SDL_GL_CreateContext(m_window);
    if (!m_gl_context) {
        S0_LOG_ERROR("Failed to create GL context: " + std::string(SDL_GetError()));
        return false;
    }
    
    SDL_GL_MakeCurrent(m_window, m_gl_context);
    SDL_GL_SetSwapInterval(m_config.vsync ? 1 : 0);
    
    // Create graphics device
    m_device = rhi::CreateGraphicsDevice(m_window);
    if (!m_device) {
        S0_LOG_ERROR("Failed to create graphics device");
        return false;
    }
    
    S0_LOG_INFO("Application initialized successfully");
    return true;
}

void Application::Shutdown() {
    m_world.reset();
    m_device.reset();
    
    if (m_gl_context) {
        SDL_GL_DeleteContext(m_gl_context);
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    
    SDL_Quit();
    
    S0_LOG_INFO("Application shutdown complete");
}

int Application::Run() {
    if (!Initialize(m_config)) {
        return -1;
    }
    
    bool running = true;
    SDL_Event event;
    
    // Main loop
    while (running) {
        // Process events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    u32 new_width = event.window.data1;
                    u32 new_height = event.window.data2;
                    m_device->ResizeSwapchain(new_width, new_height);
                    OnResize(new_width, new_height);
                }
            }
        }
        
        // Update
        f32 dt = CalculateDeltaTime();
        OnUpdate(dt);
        
        // Render
        OnRender();
        
        // Present
        m_device->EndFrame();
    }
    
    Shutdown();
    return 0;
}

f32 Application::CalculateDeltaTime() {
    static Uint64 last_time = SDL_GetPerformanceCounter();
    Uint64 current_time = SDL_GetPerformanceCounter();
    f32 dt = static_cast<f32>((current_time - last_time) * 1000.0 / SDL_GetPerformanceFrequency()) / 1000.0f;
    last_time = current_time;
    return dt;
}

void Application::OnResize(u32 width, u32 height) {
    // Override in derived class
}

void Application::OnUpdate(f32 dt) {
    // Override in derived class
    if (m_world) {
        m_world->Update(dt);
    }
}

void Application::OnRender() {
    // Override in derived class
    m_device->BeginFrame();
    
    auto cmd = m_device->CreateCommandBuffer();
    cmd->Begin();
    
    cmd->ClearColor(0, 0.1f, 0.1f, 0.1f, 1.0f);
    cmd->ClearDepth(1.0f);
    
    if (m_world) {
        m_world->Render(cmd.get());
    }
    
    cmd->End();
    m_device->SubmitAndExecute(std::move(cmd));
}

void Application::LoadWorld(const std::string& world_path) {
    m_world = std::make_unique<World>();
    if (!m_world->Load(world_path)) {
        S0_LOG_ERROR("Failed to load world: " + world_path);
        m_world.reset();
    }
}

} // namespace s0::engine

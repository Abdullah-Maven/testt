#include "rhi/GraphicsDevice.h"
#include "core/Logger.h"
#include <SDL2/SDL.h>

#ifdef __APPLE__
// Metal implementation will be in .mm file
#else
#include <glad/glad.h>

namespace s0::rhi {

// ------------------------------------------------------------------
// OpenGL Command Buffer Implementation
// ------------------------------------------------------------------
class GLCommandBuffer : public CommandBuffer {
public:
    GLCommandBuffer() : m_begun(false) {}
    
    void Begin() override {
        m_begun = true;
        m_commands.clear();
    }
    
    void End() override {
        m_begun = false;
        ExecuteImmediate();
    }
    
    void BindPipeline(u64 pipeline_handle) override {
        m_commands.push_back([pipeline_handle]() {
            GLuint program = static_cast<GLuint>(pipeline_handle);
            glUseProgram(program);
        });
    }
    
    void BindVertexBuffer(u32 slot, Buffer* buffer, size_t offset = 0) override {
        m_commands.push_back([buffer, offset, slot]() {
            if (buffer) {
                GLuint gl_buffer = *static_cast<GLuint*>(buffer->GetNativeHandle());
                glBindVertexBuffer(slot, gl_buffer, static_cast<GLintptr>(offset), 0);
            }
        });
    }
    
    void BindIndexBuffer(Buffer* buffer, IndexType type, size_t offset = 0) override {
        m_commands.push_back([buffer, type, offset]() {
            if (buffer) {
                GLuint gl_buffer = *static_cast<GLuint*>(buffer->GetNativeHandle());
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffer);
            }
        });
    }
    
    void BindUniformBuffer(u32 slot, Buffer* buffer) override {
        m_commands.push_back([buffer, slot]() {
            if (buffer) {
                GLuint gl_buffer = *static_cast<GLuint*>(buffer->GetNativeHandle());
                glBindBufferBase(GL_UNIFORM_BUFFER, slot, gl_buffer);
            }
        });
    }
    
    void BindTexture(u32 slot, Texture* texture) override {
        m_commands.push_back([texture, slot]() {
            if (texture) {
                texture->Bind(slot);
            }
        });
    }
    
    void SetViewport(f32 x, f32 y, f32 w, f32 h) override {
        m_commands.push_back([x, y, w, h]() {
            glViewport(static_cast<GLint>(x), static_cast<GLint>(y), 
                      static_cast<GLsizei>(w), static_cast<GLsizei>(h));
        });
    }
    
    void SetScissor(i32 x, i32 y, i32 w, i32 h) override {
        m_commands.push_back([x, y, w, h]() {
            glScissor(x, y, w, h);
        });
    }
    
    void Draw(u32 vertex_count, u32 instance_count = 1, u32 first_vertex = 0, u32 first_instance = 0) override {
        m_commands.push_back([vertex_count, instance_count, first_vertex, first_instance]() {
            if (instance_count > 1) {
                glDrawArraysInstanced(GL_TRIANGLES, first_vertex, vertex_count, instance_count);
            } else {
                glDrawArrays(GL_TRIANGLES, first_vertex, vertex_count);
            }
        });
    }
    
    void DrawIndexed(u32 index_count, u32 instance_count = 1, u32 first_index = 0, i32 vertex_offset = 0, u32 first_instance = 0) override {
        m_commands.push_back([index_count, instance_count, first_index, vertex_offset, first_instance]() {
            GLenum type = GL_UNSIGNED_INT;
            void* offset = reinterpret_cast<void*>(static_cast<uintptr_t>(first_index * sizeof(u32)));
            
            if (instance_count > 1) {
                glDrawElementsInstanced(GL_TRIANGLES, index_count, type, offset, instance_count);
            } else {
                glDrawElements(GL_TRIANGLES, index_count, type, offset);
            }
        });
    }
    
    void DispatchCompute(u32 group_x, u32 group_y, u32 group_z) override {
        m_commands.push_back([group_x, group_y, group_z]() {
            glDispatchCompute(group_x, group_y, group_z);
        });
    }
    
    void ClearColor(u32 attachment, f32 r, f32 g, f32 b, f32 a) override {
        m_commands.push_back([r, g, b, a]() {
            glClearColor(r, g, b, a);
            glClear(GL_COLOR_BUFFER_BIT);
        });
    }
    
    void ClearDepth(f32 depth) override {
        m_commands.push_back([depth]() {
            glClearDepth(depth);
            glClear(GL_DEPTH_BUFFER_BIT);
        });
    }

private:
    void ExecuteImmediate() {
        for (auto& cmd : m_commands) {
            cmd();
        }
        m_commands.clear();
    }
    
    bool m_begun;
    std::vector<std::function<void()>> m_commands;
};

// ------------------------------------------------------------------
// OpenGL Buffer Implementation
// ------------------------------------------------------------------
class GLBuffer : public Buffer {
public:
    GLBuffer(const BufferDesc& desc) : m_size(desc.size), m_usage(desc.usage) {
        glGenBuffers(1, &m_gl_buffer);
        
        GLenum target = GL_ARRAY_BUFFER;
        if (desc.usage & BufferUsage::Index) target = GL_ELEMENT_ARRAY_BUFFER;
        if (desc.usage & BufferUsage::Uniform) target = GL_UNIFORM_BUFFER;
        if (desc.usage & BufferUsage::Storage) target = GL_SHADER_STORAGE_BUFFER;
        
        glBindBuffer(target, m_gl_buffer);
        GLenum usage_hint = desc.is_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
        glBufferData(target, static_cast<GLsizeiptr>(desc.size), desc.initial_data, usage_hint);
        glBindBuffer(target, 0);
    }
    
    ~GLBuffer() override {
        glDeleteBuffers(1, &m_gl_buffer);
    }
    
    void Bind(BufferUsage usage) override {
        GLenum target = GL_ARRAY_BUFFER;
        if (usage & BufferUsage::Index) target = GL_ELEMENT_ARRAY_BUFFER;
        if (usage & BufferUsage::Uniform) target = GL_UNIFORM_BUFFER;
        if (usage & BufferUsage::Storage) target = GL_SHADER_STORAGE_BUFFER;
        glBindBuffer(target, m_gl_buffer);
    }
    
    void Update(const void* data, size_t size, size_t offset = 0) override {
        GLenum target = GL_ARRAY_BUFFER;
        if (m_usage & BufferUsage::Index) target = GL_ELEMENT_ARRAY_BUFFER;
        if (m_usage & BufferUsage::Uniform) target = GL_UNIFORM_BUFFER;
        if (m_usage & BufferUsage::Storage) target = GL_SHADER_STORAGE_BUFFER;
        
        glBindBuffer(target, m_gl_buffer);
        glBufferSubData(target, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
        glBindBuffer(target, 0);
    }
    
    void* Map() override {
        GLenum target = GL_ARRAY_BUFFER;
        if (m_usage & BufferUsage::Index) target = GL_ELEMENT_ARRAY_BUFFER;
        if (m_usage & BufferUsage::Uniform) target = GL_UNIFORM_BUFFER;
        if (m_usage & BufferUsage::Storage) target = GL_SHADER_STORAGE_BUFFER;
        
        glBindBuffer(target, m_gl_buffer);
        return glMapBuffer(target, GL_WRITE_ONLY);
    }
    
    void Unmap() override {
        GLenum target = GL_ARRAY_BUFFER;
        if (m_usage & BufferUsage::Index) target = GL_ELEMENT_ARRAY_BUFFER;
        if (m_usage & BufferUsage::Uniform) target = GL_UNIFORM_BUFFER;
        if (m_usage & BufferUsage::Storage) target = GL_SHADER_STORAGE_BUFFER;
        
        glUnmapBuffer(target);
        glBindBuffer(target, 0);
    }
    
    size_t GetSize() const override { return m_size; }
    void* GetNativeHandle() override { return &m_gl_buffer; }

private:
    GLuint m_gl_buffer;
    size_t m_size;
    BufferUsage m_usage;
};

// ------------------------------------------------------------------
// OpenGL Texture Implementation
// ------------------------------------------------------------------
class GLTexture : public Texture {
public:
    GLTexture(const TextureDesc& desc) : m_desc(desc) {
        glGenTextures(1, &m_gl_texture);
        glBindTexture(GL_TEXTURE_2D, m_gl_texture);
        
        GLenum internal_format = GL_RGBA8;
        GLenum format = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;
        
        switch (desc.format) {
            case TextureFormat::RGBA8_UNorm:
                internal_format = GL_RGBA8;
                break;
            case TextureFormat::RGBA16_Float:
                internal_format = GL_RGBA16F;
                type = GL_HALF_FLOAT;
                break;
            case TextureFormat::DepthStencil:
                internal_format = GL_DEPTH24_STENCIL8;
                format = GL_DEPTH_STENCIL;
                type = GL_UNSIGNED_INT_24_8;
                break;
            case TextureFormat::R8_UNorm:
                internal_format = GL_R8;
                format = GL_RED;
                break;
            case TextureFormat::RGBA32_Float:
                internal_format = GL_RGBA32F;
                type = GL_FLOAT;
                break;
            default:
                break;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, 
                    static_cast<GLsizei>(desc.width), static_cast<GLsizei>(desc.height),
                    0, format, type, desc.initial_data);
        
        if (desc.is_render_target || desc.is_depth_stencil) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } else {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    ~GLTexture() override {
        glDeleteTextures(1, &m_gl_texture);
    }
    
    void Bind(u32 slot) override {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_gl_texture);
    }
    
    u32 GetWidth() const override { return m_desc.width; }
    u32 GetHeight() const override { return m_desc.height; }
    u32 GetDepth() const override { return m_desc.depth; }
    TextureFormat GetFormat() const override { return m_desc.format; }
    void* GetNativeHandle() override { return &m_gl_texture; }

private:
    TextureDesc m_desc;
    GLuint m_gl_texture;
};

// ------------------------------------------------------------------
// OpenGL Graphics Device Implementation
// ------------------------------------------------------------------
class GLGraphicsDevice : public GraphicsDevice {
public:
    GLGraphicsDevice(void* window_handle) : m_window_handle(window_handle) {
        if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
            S0_LOG_ERROR("Failed to initialize GLAD");
            return;
        }
        
        S0_LOG_INFO("OpenGL initialized");
        S0_LOG_INFO("Vendor: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
        S0_LOG_INFO("Renderer: " + std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
        S0_LOG_INFO("Version: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))));
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    
    ~GLGraphicsDevice() override = default;
    
    std::unique_ptr<Texture> CreateTexture(const TextureDesc& desc) override {
        return std::make_unique<GLTexture>(desc);
    }
    
    std::unique_ptr<Buffer> CreateBuffer(const BufferDesc& desc) override {
        return std::make_unique<GLBuffer>(desc);
    }
    
    u64 CreatePipeline(const PipelineDesc& desc) override {
        GLuint program = glCreateProgram();
        
        for (const auto& shader_desc : desc.shaders) {
            GLuint shader = glCreateShader(
                shader_desc.stage == ShaderStage::Vertex ? GL_VERTEX_SHADER :
                shader_desc.stage == ShaderStage::Fragment ? GL_FRAGMENT_SHADER : GL_COMPUTE_SHADER
            );
            
            const char* source = shader_desc.source.data();
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);
            
            GLint success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                char log[512];
                glGetShaderInfoLog(shader, 512, nullptr, log);
                S0_LOG_ERROR("Shader compilation failed: " + std::string(log));
            }
            
            glAttachShader(program, shader);
            glDeleteShader(shader);
        }
        
        glLinkProgram(program);
        
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char log[512];
            glGetProgramInfoLog(program, 512, nullptr, log);
            S0_LOG_ERROR("Program linking failed: " + std::string(log));
        }
        
        return static_cast<u64>(program);
    }
    
    std::unique_ptr<CommandBuffer> CreateCommandBuffer() override {
        return std::make_unique<GLCommandBuffer>();
    }
    
    void BeginFrame() override {}
    
    void EndFrame() override {
        SDL_GL_SwapWindow(static_cast<SDL_Window*>(m_window_handle));
    }
    
    void SubmitAndExecute(std::unique_ptr<CommandBuffer> cmd) override {
        cmd.reset();
    }
    
    void WaitIdle() override {
        glFinish();
    }
    
    void ResizeSwapchain(u32 width, u32 height) override {
        m_swapchain_width = width;
        m_swapchain_height = height;
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    }
    
    u32 GetSwapchainWidth() const override { return m_swapchain_width; }
    u32 GetSwapchainHeight() const override { return m_swapchain_height; }
    
    u32 GetMaxTextureSize() const override {
        GLint max_size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
        return static_cast<u32>(max_size);
    }
    
    u32 GetMaxUniformBuffers() const override {
        GLint max_buffers;
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_buffers);
        return static_cast<u32>(max_buffers);
    }

private:
    void* m_window_handle;
    u32 m_swapchain_width = 0;
    u32 m_swapchain_height = 0;
};

std::unique_ptr<GraphicsDevice> CreateGraphicsDevice(void* window_handle) {
    return std::make_unique<GLGraphicsDevice>(window_handle);
}

} // namespace s0::rhi
#endif

#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>
#include <string_view>
#include "core/Types.h"

namespace s0::rhi {

// ------------------------------------------------------------------
// Enums
// ------------------------------------------------------------------
enum class PrimitiveTopology { TriangleList, LineList };
enum class IndexType { UInt16, UInt32 };
enum class TextureFormat { 
    RGBA8_UNorm, 
    RGBA16_Float, 
    DepthStencil, 
    BC1, 
    BC7,
    R8_UNorm,
    RGBA32_Float
};
enum class ShaderStage { Vertex, Fragment, Compute };
enum class BufferUsage { None = 0, Vertex = 1 << 0, Index = 1 << 1, Uniform = 1 << 2, Storage = 1 << 3 };
inline BufferUsage operator|(BufferUsage a, BufferUsage b) { return static_cast<BufferUsage>(static_cast<int>(a) | static_cast<int>(b)); }

// ------------------------------------------------------------------
// Descriptors
// ------------------------------------------------------------------
struct ShaderDesc {
    std::string_view source;
    ShaderStage stage;
    std::string entry_point = "main";
    std::string_view file_path; // For loading from disk
};

struct PipelineDesc {
    std::vector<ShaderDesc> shaders;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    bool depth_test_enabled = true;
    bool depth_write_enabled = true;
    bool blend_enabled = false;
    TextureFormat color_format = TextureFormat::RGBA8_UNorm;
    TextureFormat depth_format = TextureFormat::DepthStencil;
};

struct TextureDesc {
    u32 width = 0;
    u32 height = 0;
    u32 depth = 1;
    TextureFormat format = TextureFormat::RGBA8_UNorm;
    bool is_render_target = false;
    bool is_depth_stencil = false;
    bool generate_mipmaps = false;
    const void* initial_data = nullptr;
};

struct BufferDesc {
    size_t size = 0;
    BufferUsage usage = BufferUsage::None;
    bool is_dynamic = false;
    const void* initial_data = nullptr;
};

// ------------------------------------------------------------------
// Forward Declarations
// ------------------------------------------------------------------
class Texture;
class Buffer;
class CommandBuffer;
class GraphicsDevice;

// ------------------------------------------------------------------
// Texture Interface
// ------------------------------------------------------------------
class Texture {
public:
    virtual ~Texture() = default;
    virtual void Bind(u32 slot) = 0;
    virtual u32 GetWidth() const = 0;
    virtual u32 GetHeight() const = 0;
    virtual u32 GetDepth() const = 0;
    virtual TextureFormat GetFormat() const = 0;
    virtual void* GetNativeHandle() = 0;
    
    // Prevent copying
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
};

// ------------------------------------------------------------------
// Buffer Interface
// ------------------------------------------------------------------
class Buffer {
public:
    virtual ~Buffer() = default;
    virtual void Bind(BufferUsage usage) = 0;
    virtual void Update(const void* data, size_t size, size_t offset = 0) = 0;
    virtual void* Map() = 0;
    virtual void Unmap() = 0;
    virtual size_t GetSize() const = 0;
    virtual void* GetNativeHandle() = 0;
    
    // Prevent copying
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
};

// ------------------------------------------------------------------
// Command Buffer Interface (Stateless Recording)
// ------------------------------------------------------------------
class CommandBuffer {
public:
    virtual ~CommandBuffer() = default;
    virtual void Begin() = 0;
    virtual void End() = 0;
    
    // State Binding
    virtual void BindPipeline(u64 pipeline_handle) = 0;
    virtual void BindVertexBuffer(u32 slot, Buffer* buffer, size_t offset = 0) = 0;
    virtual void BindIndexBuffer(Buffer* buffer, IndexType type, size_t offset = 0) = 0;
    virtual void BindUniformBuffer(u32 slot, Buffer* buffer) = 0;
    virtual void BindTexture(u32 slot, Texture* texture) = 0;
    
    // Viewport & Scissor
    virtual void SetViewport(f32 x, f32 y, f32 w, f32 h) = 0;
    virtual void SetScissor(i32 x, i32 y, i32 w, i32 h) = 0;
    
    // Draw Calls
    virtual void Draw(u32 vertex_count, u32 instance_count = 1, u32 first_vertex = 0, u32 first_instance = 0) = 0;
    virtual void DrawIndexed(u32 index_count, u32 instance_count = 1, u32 first_index = 0, i32 vertex_offset = 0, u32 first_instance = 0) = 0;
    virtual void DispatchCompute(u32 group_x, u32 group_y, u32 group_z) = 0;
    
    // Clear
    virtual void ClearColor(u32 attachment, f32 r, f32 g, f32 b, f32 a) = 0;
    virtual void ClearDepth(f32 depth) = 0;
};

// ------------------------------------------------------------------
// Graphics Device Interface (API Agnostic)
// ------------------------------------------------------------------
class GraphicsDevice {
public:
    virtual ~GraphicsDevice() = default;

    // Resource Creation
    virtual std::unique_ptr<Texture> CreateTexture(const TextureDesc& desc) = 0;
    virtual std::unique_ptr<Buffer> CreateBuffer(const BufferDesc& desc) = 0;
    virtual u64 CreatePipeline(const PipelineDesc& desc) = 0;
    virtual std::unique_ptr<CommandBuffer> CreateCommandBuffer() = 0;
    
    // Frame Management
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void SubmitAndExecute(std::unique_ptr<CommandBuffer> cmd) = 0;
    virtual void WaitIdle() = 0;
    
    // Swapchain
    virtual void ResizeSwapchain(u32 width, u32 height) = 0;
    virtual u32 GetSwapchainWidth() const = 0;
    virtual u32 GetSwapchainHeight() const = 0;
    
    // Limits
    virtual u32 GetMaxTextureSize() const = 0;
    virtual u32 GetMaxUniformBuffers() const = 0;
};

// Factory function implemented per-platform
std::unique_ptr<GraphicsDevice> CreateGraphicsDevice(void* window_handle);

} // namespace s0::rhi

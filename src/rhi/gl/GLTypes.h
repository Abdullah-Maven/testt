#pragma once

#include <glad/glad.h>

// OpenGL-specific type aliases and utilities
namespace s0::rhi::gl {

inline GLenum ToGLTextureFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_UNorm: return GL_RGBA8;
        case TextureFormat::RGBA16_Float: return GL_RGBA16F;
        case TextureFormat::DepthStencil: return GL_DEPTH24_STENCIL8;
        case TextureFormat::R8_UNorm: return GL_R8;
        case TextureFormat::RGBA32_Float: return GL_RGBA32F;
        default: return GL_RGBA8;
    }
}

inline GLenum ToGLInternalFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_UNorm: return GL_RGBA;
        case TextureFormat::RGBA16_Float: return GL_RGBA;
        case TextureFormat::DepthStencil: return GL_DEPTH_STENCIL;
        case TextureFormat::R8_UNorm: return GL_RED;
        case TextureFormat::RGBA32_Float: return GL_RGBA;
        default: return GL_RGBA;
    }
}

inline GLenum ToGLType(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_UNorm: return GL_UNSIGNED_BYTE;
        case TextureFormat::RGBA16_Float: return GL_HALF_FLOAT;
        case TextureFormat::DepthStencil: return GL_UNSIGNED_INT_24_8;
        case TextureFormat::R8_UNorm: return GL_UNSIGNED_BYTE;
        case TextureFormat::RGBA32_Float: return GL_FLOAT;
        default: return GL_UNSIGNED_BYTE;
    }
}

} // namespace s0::rhi::gl

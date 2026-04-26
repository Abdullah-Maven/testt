#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include <string_view>
#include <array>

namespace s0 {

// ------------------------------------------------------------------
// Basic Types
// ------------------------------------------------------------------
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

// ------------------------------------------------------------------
// Math Utilities (Minimal Implementation)
// ------------------------------------------------------------------
struct Vec3 {
    f32 x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}
};

struct Vec4 {
    f32 x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}
};

struct Mat4 {
    std::array<f32, 16> m;
    Mat4() { m.fill(0.0f); }
    static Mat4 Identity();
    static Mat4 Perspective(f32 fov, f32 aspect, f32 near, f32 far);
    static Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
};

// ------------------------------------------------------------------
// Logging
// ------------------------------------------------------------------
enum class LogLevel { Debug, Info, Warning, Error };

void Log(LogLevel level, const char* message);
#define S0_LOG_INFO(msg) s0::Log(s0::LogLevel::Info, msg)
#define S0_LOG_ERROR(msg) s0::Log(s0::LogLevel::Error, msg)
#define S0_LOG_WARN(msg) s0::Log(s0::LogLevel::Warning, msg)
#define S0_LOG_DEBUG(msg) s0::Log(s0::LogLevel::Debug, msg)

} // namespace s0

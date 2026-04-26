#include "core/Types.h"
#include <cmath>

namespace s0 {

constexpr f32 PI = 3.14159265358979323846f;

Mat4 Mat4::Identity() {
    Mat4 result;
    result.m[0] = 1.0f;  result.m[4] = 0.0f;  result.m[8] = 0.0f;  result.m[12] = 0.0f;
    result.m[1] = 0.0f;  result.m[5] = 1.0f;  result.m[9] = 0.0f;  result.m[13] = 0.0f;
    result.m[2] = 0.0f;  result.m[6] = 0.0f;  result.m[10] = 1.0f; result.m[14] = 0.0f;
    result.m[3] = 0.0f;  result.m[7] = 0.0f;  result.m[11] = 0.0f; result.m[15] = 1.0f;
    return result;
}

Mat4 Mat4::Perspective(f32 fov, f32 aspect, f32 near_plane, f32 far_plane) {
    Mat4 result;
    
    f32 tan_half_fov = std::tan(fov / 2.0f);
    
    result.m[0] = 1.0f / (aspect * tan_half_fov);
    result.m[1] = 0.0f;
    result.m[2] = 0.0f;
    result.m[3] = 0.0f;
    
    result.m[4] = 0.0f;
    result.m[5] = 1.0f / tan_half_fov;
    result.m[6] = 0.0f;
    result.m[7] = 0.0f;
    
    result.m[8] = 0.0f;
    result.m[9] = 0.0f;
    result.m[10] = -(far_plane + near_plane) / (far_plane - near_plane);
    result.m[11] = -1.0f;
    
    result.m[12] = 0.0f;
    result.m[13] = 0.0f;
    result.m[14] = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
    result.m[15] = 0.0f;
    
    return result;
}

Mat4 Mat4::LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Mat4 result;
    
    // Forward vector
    Vec3 f = {eye.x - center.x, eye.y - center.y, eye.z - center.z};
    f32 len = std::sqrt(f.x*f.x + f.y*f.y + f.z*f.z);
    f.x /= len; f.y /= len; f.z /= len;
    
    // Right vector
    Vec3 r = {up.y * f.z - up.z * f.y, up.z * f.x - up.x * f.z, up.x * f.y - up.y * f.x};
    len = std::sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
    if (len > 0.0f) {
        r.x /= len; r.y /= len; r.z /= len;
    }
    
    // Up vector
    Vec3 u = {f.y * r.z - f.z * r.y, f.z * r.x - f.x * r.z, f.x * r.y - f.y * r.x};
    
    result.m[0] = r.x;  result.m[4] = r.y;  result.m[8] = r.z;  result.m[12] = -(r.x*eye.x + r.y*eye.y + r.z*eye.z);
    result.m[1] = u.x;  result.m[5] = u.y;  result.m[9] = u.z;  result.m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
    result.m[2] = f.x;  result.m[6] = f.y;  result.m[10] = f.z; result.m[14] = -(f.x*eye.x + f.y*eye.y + f.z*eye.z);
    result.m[3] = 0.0f; result.m[7] = 0.0f; result.m[11] = 0.0f; result.m[15] = 1.0f;
    
    return result;
}

} // namespace s0

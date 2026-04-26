#include "core/Types.h"

namespace s0 {

Mat4 Mat4::Identity() {
    Mat4 result;
    result.m[0] = 1.0f;  result.m[5] = 1.0f;  
    result.m[10] = 1.0f; result.m[15] = 1.0f;
    return result;
}

Mat4 Mat4::Perspective(f32 fov, f32 aspect, f32 near, f32 far) {
    Mat4 result;
    f32 tan_half_fov = tanf(fov / 2.0f);
    
    result.m[0] = 1.0f / (aspect * tan_half_fov);
    result.m[5] = 1.0f / tan_half_fov;
    result.m[10] = -(far + near) / (far - near);
    result.m[11] = -1.0f;
    result.m[14] = -(2.0f * far * near) / (far - near);
    
    return result;
}

Mat4 Mat4::LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = Vec3(center.x - eye.x, center.y - eye.y, center.z - eye.z);
    f32 len = sqrtf(f.x*f.x + f.y*f.y + f.z*f.z);
    f.x /= len; f.y /= len; f.z /= len;
    
    Vec3 r = Vec3(f.y * up.z - f.z * up.y, 
                  f.z * up.x - f.x * up.z, 
                  f.x * up.y - f.y * up.x);
    len = sqrtf(r.x*r.x + r.y*r.y + r.z*r.z);
    r.x /= len; r.y /= len; r.z /= len;
    
    Vec3 u = Vec3(f.z * r.y - f.y * r.z, 
                  f.x * r.z - f.z * r.x, 
                  f.y * r.x - f.x * r.y);
    
    Mat4 result;
    result.m[0] = r.x;   result.m[1] = u.x;   result.m[2] = -f.x;  result.m[3] = 0.0f;
    result.m[4] = r.y;   result.m[5] = u.y;   result.m[6] = -f.y;  result.m[7] = 0.0f;
    result.m[8] = r.z;   result.m[9] = u.z;   result.m[10] = -f.z; result.m[11] = 0.0f;
    result.m[12] = -(r.x*eye.x + r.y*eye.y + r.z*eye.z);
    result.m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
    result.m[14] = (f.x*eye.x + f.y*eye.y + f.z*eye.z);
    result.m[15] = 1.0f;
    
    return result;
}

} // namespace s0

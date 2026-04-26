#include "core/Types.h"
#include <cstdio>

namespace s0 {

void Log(LogLevel level, const char* message) {
    const char* prefix = "";
    switch (level) {
        case LogLevel::Debug:   prefix = "[DEBUG]"; break;
        case LogLevel::Info:    prefix = "[INFO] "; break;
        case LogLevel::Warning: prefix = "[WARN] "; break;
        case LogLevel::Error:   prefix = "[ERROR]"; break;
    }
    
    std::printf("%s %s\n", prefix, message);
}

} // namespace s0

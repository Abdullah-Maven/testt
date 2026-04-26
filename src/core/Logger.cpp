#include "core/Types.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace s0 {

void Log(LogLevel level, const char* message) {
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    const char* level_str = "";
    switch (level) {
        case LogLevel::Debug:   level_str = "DEBUG"; break;
        case LogLevel::Info:    level_str = "INFO "; break;
        case LogLevel::Warning: level_str = "WARN "; break;
        case LogLevel::Error:   level_str = "ERROR"; break;
    }
    
    std::cout << "[" << ss.str() << "] [" << level_str << "] " << message << std::endl;
}

} // namespace s0

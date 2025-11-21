#pragma once
#include <cmath>

inline bool inSector(float ax, float ay, float bx, float by) {
    float dx = bx - ax;
    float dy = by - ay;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > 2.0f) return false;
    float angle = std::atan2(dy, dx); // angle relative to +x
    // allow +/- 30 degrees = pi/6
    return std::abs(angle) <= (3.14159265f / 6.0f);
}

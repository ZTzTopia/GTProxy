#pragma once

namespace math {
    template <typename T>
    struct Vec2 {
        T x, y;

        Vec2() : x(0), y(0) {}
        Vec2(T x, T y) : x(x), y(y) {}
        Vec2(const Vec2& v) : x(v.x), y(v.y) {}
    };

    template <typename T>
    struct Vec3 {
        T x, y, z;

        Vec3() : x(0), y(0), z(0) {}
        Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
        Vec3(const Vec3& v) : x(v.x), y(v.y), z(v.z) {}
    };
}

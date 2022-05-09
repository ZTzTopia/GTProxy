#pragma once
#include <utility>

namespace math {
    template <typename T>
    struct Vec2 {
        T x, y;

        Vec2() : x(0), y(0) {}
        explicit Vec2(T f) : x(f), y(f) {}
        Vec2(T x, T y) : x(x), y(y) {}
        Vec2(const Vec2& v) : x(v.x), y(v.y) {}

        std::pair<T, T> to_pair() const {
            return std::make_pair(x, y);
        }
    };

    template <typename T>
    struct Vec3 {
        T x, y, z;

        Vec3() : x(0), y(0), z(0) {}
        explicit Vec3(T f) : x(f), y(f), z(f) {}
        Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
        Vec3(const Vec3& v) : x(v.x), y(v.y), z(v.z) {}

        std::tuple<T, T, T> to_tuple() const {
            return std::make_tuple(x, y, z);
        }
    };
}

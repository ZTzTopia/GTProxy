#pragma once
#include <algorithm>
#include <cmath>
#include <utility>

namespace utils {
namespace math {
template <typename T>
struct Vec2 {
    Vec2() : x(0), y(0) {}
    explicit Vec2(T f) : x(f), y(f) {}
    Vec2(T x, T y) : x(x), y(y) {}
    Vec2(const Vec2& v) : x(v.x), y(v.y) {}

    bool operator==(const Vec2& v) const
    {
        return x == v.x && y == v.y;
    }

    Vec2& operator=(const Vec2& v)
    {
        x = v.x;
        y = v.y;
        return *this;
    }

    std::pair<T, T> to_pair() const
    {
        return std::make_pair(x, y);
    }

    T x, y;
};

template <typename T>
struct Vec3 {
    Vec3() : x(0), y(0), z(0) {}
    explicit Vec3(T f) : x(f), y(f), z(f) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    Vec3(const Vec3& v) : x(v.x), y(v.y), z(v.z) {}

    bool operator==(const Vec3& v) const
    {
        return x == v.x && y == v.y && z == v.z;
    }

    Vec3& operator=(const Vec3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    std::tuple<T, T, T> to_tuple() const
    {
        return std::make_tuple(x, y, z);
    }

    T x, y, z;
};

template <typename T>
T distance(const Vec2<T>& a, const Vec2<T>& b)
{
    return T(std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2)));
}

template <typename T>
T manhattan_heuristic(const Vec2<T>& a, const Vec2<T>& b)
{
    return T(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

template <typename T>
T euclidean_heuristic(const Vec2<T>& a, const Vec2<T>& b)
{
    return distance<T>(a, b);
}
}
}

#pragma once

typedef struct _CL_Vec2f {
    float x;
    float y;

    _CL_Vec2f() {
        x = 0.0f;
        y = 0.0f;
    }

    explicit _CL_Vec2f(float f) {
        x = f;
        y = f;
    }

    _CL_Vec2f(float x, float y) {
        this->x = x;
        this->y = y;
    }

    bool operator==(const _CL_Vec2f& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const _CL_Vec2f& other) const {
        return x != other.x || y != other.y;
    }

    _CL_Vec2f operator+(const _CL_Vec2f& other) const {
        return { x + other.x, y + other.y };
    }

    _CL_Vec2f operator-(const _CL_Vec2f& other) const {
        return { x - other.x, y - other.y };
    }

    _CL_Vec2f operator*(const float& other) const {
        return { x * other, y * other };
    }

    _CL_Vec2f operator/(const float& other) const {
        return { x / other, y / other };
    }

    [[nodiscard]] std::pair<float, float> get_pair() const {
        return { x, y };
    }
} CL_Vec2f;

typedef struct _CL_Vec3f {
    float x;
    float y;
    float z;

    _CL_Vec3f() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    explicit _CL_Vec3f(float f) {
        x = f;
        y = f;
        z = f;
    }

    _CL_Vec3f(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    bool operator==(const _CL_Vec3f& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const _CL_Vec3f& other) const {
        return x != other.x || y != other.y || z != other.z;
    }

    _CL_Vec3f operator+(const _CL_Vec3f& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    _CL_Vec3f operator-(const _CL_Vec3f& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    _CL_Vec3f operator*(const float& other) const {
        return { x * other, y * other, z * other };
    }

    _CL_Vec3f operator/(const float& other) const {
        return { x / other, y / other, z / other };
    }
} CL_Vec3f;

typedef struct CL_Rectf {
    float x;
    float y;
    float width;
    float height;

    CL_Rectf() {
        x = 0.0f;
        y = 0.0f;
        width = 0.0f;
        height = 0.0f;
    }

    explicit CL_Rectf(float f) {
        x = f;
        y = f;
        width = f;
        height = f;
    }

    CL_Rectf(float x, float y, float width, float height) {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
    }

    bool operator==(const CL_Rectf& other) const {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    bool operator!=(const CL_Rectf& other) const {
        return x != other.x || y != other.y || width != other.width || height != other.height;
    }

    CL_Rectf operator+(const CL_Rectf& other) const {
        return { x + other.x, y + other.y, width + other.width, height + other.height };
    }

    CL_Rectf operator-(const CL_Rectf& other) const {
        return { x - other.x, y - other.y, width - other.width, height - other.height };
    }

    CL_Rectf operator*(const float& other) const {
        return { x * other, y * other, width * other, height * other };
    }

    CL_Rectf operator/(const float& other) const {
        return { x / other, y / other, width / other, height / other };
    }
} CL_Rectf;

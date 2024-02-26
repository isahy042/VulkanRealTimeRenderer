#pragma once
/**
    A vector math file.
    Inspired by the Dartmouth Introductory Ray Tracer.
*/

#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <type_traits>

// basic vector type
template <size_t N, typename T>
struct Vec
{
    std::array<T, N> e;

    Vec() = default;

    explicit Vec(T e0)
    {
        for (size_t i = 0; i < N; ++i)
        {
            e[i] = e0;
        }
    }

    Vec(const std::initializer_list<T>& list)
    {
        int i = 0;
        for (auto& element : list)
        {
            e[i] = element;
            if (++i > N)
            {
                break;
            }
        }
    }

    T operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i) { return e[i]; }
};

/**
* Vector basic types implementation
*/

/// 2 vector
template <typename T>
struct Vec<2, T>
{
    union
    {
        std::array<T, 2> e;
        struct
        {
            T x, y;
        };
        struct
        {
            T u, v;
        };
    };

    constexpr Vec() = default;
    constexpr explicit Vec(T e0) : x(e0), y(e0) {}
    constexpr Vec(T e0, T e1) : x(e0), y(e1) {}

    T operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i) { return e[i]; }

};

/// 3 vector
template <typename T>
struct Vec<3, T>
{
    union
    {
        std::array<T, 3> e;
        struct
        {
            T x, y, z;
        };
        struct
        {
            T r, g, b;
        };
        Vec<2, T> xy;
    };

    constexpr Vec() = default;
    constexpr explicit Vec(T e0) : x(e0), y(e0), z(e0) {}
    constexpr Vec(T e0, T e1, T e2) : x(e0), y(e1), z(e2) {}
    constexpr Vec(const Vec<2, T> xy, T _z) : x(xy.x), y(xy.y), z(_z) {}

    T operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i) { return e[i]; }
};

/// 3 vector crossing
template <typename T>
inline Vec<3, T> cross(const Vec<3, T>& v1, const Vec<3, T>& v2)
{
    return Vec<3, T>((v1.y * v2.z - v1.z * v2.y), -(v1.x * v2.z - v1.z * v2.x), (v1.x * v2.y - v1.y * v2.x));
}

/// A 4-vector
template <typename T>
struct Vec<4, T>
{
    union
    {
        std::array<T, 4> e;
        struct
        {
            T x, y, z, w;
        };
        struct
        {
            T r, g, b, a;
        };
        Vec<3, T> xyz;
        Vec<3, T> rgb;
        Vec<2, T> xy;
    };

    constexpr Vec() = default;
    constexpr explicit Vec(T e0) : x(e0), y(e0), z(e0), w(e0) {}
    constexpr Vec(T e0, T e1, T e2, T e3) : x(e0), y(e1), z(e2), w(e3) {}
    constexpr Vec(const Vec<3, T> xyz, T _w) : x(xyz.x), y(xyz.y), z(xyz.z), w(_w) {}

    T operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i) { return e[i]; }
};

/**
* Declare 2,3,4 vectors using the following data types:
*/
template <typename T>
using Vec2 = Vec<2, T>;
template <typename T>
using Vec3 = Vec<3, T>;
template <typename T>
using Vec4 = Vec<4, T>;

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;
using Vec4c = Vec4<std::uint8_t>;
using Vec4uc = Vec4<unsigned char>;

// relevant matrices used.
using Vec44f = Vec4<Vec4f>;
using Vec33f = Vec3<Vec3f>;

/**
    Basic vector operations
*/
template <size_t N, typename T>
inline T dot(const Vec<N, T>& v1, const Vec<N, T>& v2)
{
    T t(0);
    for (size_t i = 0; i < N; ++i)
    {
        t += v1[i] * v2[i];
    }
    return t;
}

/// Return the geometric length (\f$l_2\f$-norm) of the vector
template <size_t N, typename T>
T length(const Vec<N, T>& v) { return std::sqrt(dot(v, v)); }

template <size_t N, typename T>
Vec<N, T> normalize(const Vec<N, T>& v) { return v / length(v); }

// negate vector
template <size_t N, typename T>
Vec<N, T> operator-(const Vec<N, T>& v)
{
    Vec<N, T> v2;
    for (size_t i = 0; i < N; ++i)
    {
        v2[i] = -v[i];
    }
    return v2;
}

/**
    vector operations
*/
template <size_t N, typename T>
Vec<N, T> operator+(const Vec<N, T>& v1, const Vec<N, T>& v2)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = v1[i] + v2[i];
    }
    return v3;
}

template <size_t N, typename T>
Vec<N, T> operator-(const Vec<N, T>& v1, const Vec<N, T>& v2)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = v1[i] - v2[i];
    }
    return v3;
}

template <size_t N, typename T>
Vec<N, T> operator*(const Vec<N, T>& v1, const Vec<N, T>& v2)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = v1[i] * v2[i];
    }
    return v3;
}

template <size_t N, typename T>
Vec<N, T> operator/(const Vec<N, T>& v1, const Vec<N, T>& v2)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = v1[i] / v2[i];
    }
    return v3;
}

/**
    scalar operations
*/
template <size_t N, typename T, typename S>
Vec<N, T> operator+(const Vec<N, T>& v1, S s)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = v1[i] + s;
    }
    return v3;
}

template <size_t N, typename T, typename S>
Vec<N, T> operator-(const Vec<N, T>& v1, S s)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = v1[i] - s;
    }
    return v3;
}

template <size_t N, typename T, typename S>
Vec<N, T> operator*(const Vec<N, T>& v1, S s)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = v1[i] * s;
    }
    return v3;
}

template <size_t N, typename T, typename S>
Vec<N, T> operator/(const Vec<N, T>& v1, S s)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = v1[i] / s;
    }
    return v3;
}

template <size_t N, typename T, typename S>
Vec<N, T> operator+(S s, const Vec<N, T>& v1)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = s + v1[i];
    }
    return v3;
}

template <size_t N, typename T, typename S>
Vec<N, T> operator-(S s, const Vec<N, T>& v1)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = s - v1[i];
    }
    return v3;
}

template <size_t N, typename T, typename S>
Vec<N, T> operator*(S s, const Vec<N, T>& v1)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = s * v1[i];
    }
    return v3;
}

template <size_t N, typename T, typename S>
Vec<N, T> operator/(S s, const Vec<N, T>& v1)
{
    Vec<N, T> v3;
    for (size_t i = 0; i < N; ++i)
    {
        v3[i] = s / v1[i];
    }
    return v3;
}


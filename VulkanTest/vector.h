#pragma once
/**
    A vector math file.
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

/// An array of N values of type T for storing colors or mathematical vectors
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

template <class C, size_t N, typename T>
std::basic_ostream<C>& operator<<(std::basic_ostream<C>& out, const Vec<N, T>& v)
{
    std::ios_base::fmtflags oldFlags = out.flags();
    auto width = out.precision() + 2;

    out.setf(std::ios_base::right);
    if (!(out.flags() & std::ios_base::scientific))
    {
        out.setf(std::ios_base::fixed);
    }
    width += 5;

    out << '{';
    for (size_t i = 0; i < N - 1; ++i)
    {
        out << std::setw(width) << v[i] << ',';
    }
    out << std::setw(width) << v[N - 1] << '}';

    out.flags(oldFlags);
    return out;
}

/// Return the geometric squared length (\f$l_2\f$-norm) of the vector
template <size_t N, typename T>
T length2(const Vec<N, T>& v) { return dot(v, v); }
/// Return the geometric length (\f$l_2\f$-norm) of the vector
template <size_t N, typename T>
T length(const Vec<N, T>& v) { return std::sqrt(length2(v)); }
/// Return a unit-length copy of the vector
template <size_t N, typename T>
Vec<N, T> normalize(const Vec<N, T>& v) { return v / length(v); }

/// Return the index (dimension) of the largest component of the vector
template <size_t N, typename T>
size_t maxDim(const Vec<N, T>& v)
{
    T m = v[0];
    size_t idx = 0;
    for (size_t i = 1; i < N; ++i)
    {
        if (v[i] > m)
        {
            m = v[i];
            idx = i;
        }
    }
    return idx;
}

/// Return the index (dimension) of the smallest component of the vector
template <size_t N, typename T>
size_t minDim(const Vec<N, T>& v)
{
    T m = v[0];
    size_t idx = 0;
    for (size_t i = 1; i < N; ++i)
    {
        if (v[i] < m)
        {
            m = v[i];
            idx = i;
        }
    }
    return idx;
}

/// Compute the element-wise absolute value
template <size_t N, typename T>
Vec<N, T> abs(const Vec<N, T>& v)
{
    Vec<N, T> v2;
    for (size_t i = 0; i < N; ++i)
    {
        v2[i] = std::abs(v[i]);
    }
    return v2;
}

/// Return the largest component of the vector
template <size_t N, typename T>
T max(const Vec<N, T>& v)
{
    T m = v[0];
    for (size_t i = 1; i < N; ++i)
    {
        if (v[i] > m)
        {
            m = v[i];
        }
    }
    return m;
}

/// Return the smallest component of the vector
template <size_t N, typename T>
T min(const Vec<N, T>& v)
{
    T m = v[0];
    for (size_t i = 1; i < N; ++i)
    {
        if (v[i] < m)
        {
            m = v[i];
        }
    }
    return m;
}

/// Return the componentwise maximum of the two vectors
template <size_t N, typename T>
Vec<N, T> max(const Vec<N, T>& a, const Vec<N, T>& b)
{
    Vec<N, T> m;
    for (size_t i = 0; i < N; ++i)
    {
        m[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    return m;
}

/// Return the componentwise minimum of the two vectors
template <size_t N, typename T>
Vec<N, T> min(const Vec<N, T>& a, const Vec<N, T>& b)
{
    Vec<N, T> m;
    for (size_t i = 0; i < N; ++i)
    {
        m[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    return m;
}

/// component-wise vector-vector assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N, T>& operator+=(Vec<N, T>& v1, const Vec<N, S>& v2)
{
    for (size_t i = 0; i < N; ++i)
    {
        v1[i] += v2[i];
    }
    return v1;
}
/// component-wise vector-vector assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N, T>& operator-=(Vec<N, T>& v1, const Vec<N, S>& v2)
{
    for (size_t i = 0; i < N; ++i)
    {
        v1[i] -= v2[i];
    }
    return v1;
}
/// component-wise vector-vector assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N, T>& operator*=(Vec<N, T>& v1, const Vec<N, S>& v2)
{
    for (size_t i = 0; i < N; ++i)
    {
        v1[i] *= v2[i];
    }
    return v1;
}
/// component-wise vector-vector assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N, T>& operator/=(Vec<N, T>& v1, const Vec<N, S>& v2)
{
    for (size_t i = 0; i < N; ++i)
    {
        v1[i] /= v2[i];
    }
    return v1;
}

/// component-wise vector-scalar assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N, T>& operator+=(Vec<N, T>& v1, S s)
{
    for (size_t i = 0; i < N; ++i)
    {
        v1[i] += s;
    }
    return v1;
}
/// component-wise vector-scalar assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N, T>& operator-=(Vec<N, T>& v1, S s)
{
    for (size_t i = 0; i < N; ++i)
    {
        v1[i] -= s;
    }
    return v1;
}

/// component-wise vector-scalar assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N, T>& operator*=(Vec<N, T>& v1, S s)
{
    for (size_t i = 0; i < N; ++i)
    {
        v1[i] *= s;
    }
    return v1;
}

/// component-wise vector-scalar assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N, T>& operator/=(Vec<N, T>& v1, S s)
{
    for (size_t i = 0; i < N; ++i)
    {
        v1[i] /= s;
    }
    return v1;
}

// positive and negative
template <size_t N, typename T>
inline const Vec<N, T>& operator+(const Vec<N, T>& v) { return v; }

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

/// component-wise vector-vector addition
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
/// component-wise vector-vector subtraction
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
/// component-wise vector-vector multiplication
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
/// component-wise vector-vector division
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

/// vector-scalar addition
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
/// vector-scalar subtraction
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
/// vector-scalar multiplication
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
/// vector-scalar division
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

/// scalar-vector addition
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
/// scalar-vector subtraction
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
/// scalar-vector multiplication
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
/// scalar-vector division
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

/// A mathematical 2-vector
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

    static inline Vec Zero() { return Vec(T(0)); }
    static inline Vec UnitX() { return Vec(T(1), T(0)); }
    static inline Vec UnitY() { return Vec(T(0), T(1)); }
};

/// A mathematical 3-vector
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

    static inline Vec Zero() { return Vec(T(0)); }
    static inline Vec UnitX() { return Vec(T(1), T(0), T(0)); }
    static inline Vec UnitY() { return Vec(T(0), T(1), T(0)); }
    static inline Vec UnitZ() { return Vec(T(0), T(0), T(1)); }
};

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

    static inline Vec Zero() { return Vec(T(0)); }
    static inline Vec UnitX() { return Vec(T(1), T(0), T(0), T(0)); }
    static inline Vec UnitY() { return Vec(T(0), T(1), T(0), T(0)); }
    static inline Vec UnitZ() { return Vec(T(0), T(0), T(1), T(0)); }
    static inline Vec UnitW() { return Vec(T(0), T(0), T(0), T(1)); }
};


template <typename T>
using Vec2 = Vec<2, T>;
template <typename T>
using Vec3 = Vec<3, T>;
template <typename T>
using Vec4 = Vec<4, T>;

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;
using Vec4d = Vec4<double>;
using Vec4c = Vec4<std::uint8_t>;
using Vec4uc = Vec4<unsigned char>;

using Vec44f = Vec4<Vec4f>;
using Vec33f = Vec3<Vec3f>;
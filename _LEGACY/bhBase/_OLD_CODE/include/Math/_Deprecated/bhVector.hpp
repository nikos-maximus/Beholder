#ifndef BH_VECTOR_HPP
#define BH_VECTOR_HPP

#include "bhMath.h"
#include <assert.h>

#pragma warning(disable : 4201) // Suppress "nonstandard extension used: nameless struct/union"

////////////////////////////////////////////////////////////////////////////////
// bhVec2
template<typename Scalar_t>
struct bhVec2
{
    union
    {
        struct
        {
            Scalar_t x, y;
        };
        struct
        {
            Scalar_t r, g;
        };
        Scalar_t _v[2];
    };

    bhVec2()
        : x(Scalar_t(0)), y(Scalar_t(0))
    {}

    bhVec2(Scalar_t _x, Scalar_t _y)
        : x(_x), y(_y)
    {}

    bhVec2(const bhVec2& v)
        : x(v.x), y(v.y)
    {}

    inline bool IsZero() const
    {
        return (x == Scalar_t(0)) && (y == Scalar_t(0));
    }

    bhVec2& operator+=(const bhVec2& v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    bhVec2& operator-=(const bhVec2& v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    bhVec2& operator*=(const bhVec2& v)
    {
        x *= v.x;
        y *= v.y;
        return *this;
    }
};

typedef bhVec2<float> bhVec2f;

template<typename Scalar_t>
inline bhVec2<Scalar_t> operator-(const bhVec2<Scalar_t>& v0)
{
    return bhVec2<Scalar_t> (-v0.x, -v0.y);
}

template<typename Scalar_t>
inline bhVec2<Scalar_t> operator+(const bhVec2<Scalar_t>& v0, const bhVec2<Scalar_t>& v1)
{
    return bhVec2<Scalar_t>(v0.x + v1.x, v0.y + v1.y);
}

template<typename Scalar_t>
inline bhVec2<Scalar_t> operator-(const bhVec2<Scalar_t>& v0, const bhVec2<Scalar_t>& v1)
{
    return bhVec2<Scalar_t>(v0.x - v1.x, v0.y - v1.y);
}

template<typename Scalar_t>
inline bhVec2<Scalar_t> operator*(const bhVec2<Scalar_t>& v, Scalar_t f)
{
    return bhVec2<Scalar_t>(v.x * f, v.y * f);
}

template<typename Scalar_t>
inline bhVec2<Scalar_t> operator/(const bhVec2<Scalar_t>& v, Scalar_t f)
{
    assert(f != 0.f);
    return bhVec2<Scalar_t>(v.x / f, v.y / f);
}

template<typename Scalar_t>
inline Scalar_t LengthSquared(const bhVec2<Scalar_t>& v)
{
    return (v.x * v.x) + (v.y * v.y);
}

inline float Length(const bhVec2f& v)
{
    return sqrtf(LengthSquared(v));
}

template<typename Scalar_t>
inline Scalar_t DistanceSquared(const bhVec2<Scalar_t>& v0, const bhVec2<Scalar_t>& v1)
{
    return LengthSquared(v1 - v0);
}

inline float Distance(const bhVec2f& v0, const bhVec2f& v1)
{
    return sqrtf(DistanceSquared(v0, v1));
}

template<typename Scalar_t>
inline bhVec2<Scalar_t> Normalize(const bhVec2<Scalar_t>& v)
{
    if (v.IsZero())
    {
        return v;
    }
    return v / Length(v);
}

template<typename Scalar_t>
inline Scalar_t Dot(const bhVec2<Scalar_t>& v0, const bhVec2<Scalar_t>& v1)
{
    return (v0.x * v1.x) + (v0.y * v1.y);
}

inline float AngleRads(const bhVec2f& v0, const bhVec2f& v1)
{
    if (v0.IsZero() || v1.IsZero())
    {
        return 0.f;
    }
    bhVec2f na = Normalize(v0);
    bhVec2f nb = Normalize(v1);
    return acosf(Dot(na, nb));
}

template<typename Scalar_t>
inline Scalar_t AngleDegs(const bhVec2<Scalar_t>& v0, const bhVec2<Scalar_t>& v1)
{
    return bhMath_Rad2Deg(AngleRads(v0, v1));
}

////////////////////////////////////////////////////////////////////////////////
// bhVec3
template<typename Scalar_t>
struct bhVec3
{
    union
    {
        struct
        {
            Scalar_t x, y, z;
        };
        struct
        {
            Scalar_t r, g, b;
        };
        Scalar_t _v[3];
    };

    bhVec3()
        : x(Scalar_t(0)), y(Scalar_t(0)), z(Scalar_t(0))
    {}

    bhVec3(Scalar_t _x, Scalar_t _y, Scalar_t _z)
        : x(_x), y(_y), z(_z)
    {}

    bhVec3(const bhVec3& v)
        : x(v.x), y(v.y), z(v.z)
    {}

    inline bool IsZero() const
    {
        return (x == Scalar_t(0)) && (y == Scalar_t(0)) && (z == Scalar_t(0));
    }

    bhVec3& operator+=(const bhVec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    bhVec3& operator-=(const bhVec3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    bhVec3& operator*=(const bhVec3& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
};

typedef bhVec3<float> bhVec3f;

template<typename Scalar_t>
inline bhVec3<Scalar_t> operator-(const bhVec3<Scalar_t>& v)
{
    return bhVec3<Scalar_t>(-v.x, -v.y, -v.z);
}

template<typename Scalar_t>
inline bhVec3<Scalar_t> operator+(const bhVec3<Scalar_t>& v0, const bhVec3<Scalar_t>& v1)
{
    return bhVec3<Scalar_t>(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

template<typename Scalar_t>
inline bhVec3<Scalar_t> operator-(const bhVec3<Scalar_t>& v0, const bhVec3<Scalar_t>& v1)
{
    return bhVec3<Scalar_t>(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

template<typename Scalar_t>
inline bhVec3<Scalar_t> operator*(const bhVec3<Scalar_t>& v, Scalar_t f)
{
    return bhVec3<Scalar_t>(v.x * f, v.y * f, v.z * f);
}

template<typename Scalar_t>
inline bhVec3<Scalar_t> operator/(const bhVec3<Scalar_t>& v, Scalar_t f)
{
    assert(f != 0.f);
    return bhVec3<Scalar_t>(v.x / f, v.y / f, v.z / f);
}

template<typename Scalar_t>
inline Scalar_t LengthSquared(const bhVec3<Scalar_t>& v)
{
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

inline float Length(const bhVec3f& v)
{
    return sqrtf(LengthSquared(v));
}

template<typename Scalar_t>
inline Scalar_t DistanceSquared(const bhVec3<Scalar_t>& v0, const bhVec3<Scalar_t>& v1)
{
    return LengthSquared(v1 - v0);
}

inline float Distance(const bhVec3f& v0, const bhVec3f& v1)
{
    return sqrtf(DistanceSquared(v0, v1));
}

template<typename Scalar_t>
inline bhVec3<Scalar_t> Normalize(const bhVec3<Scalar_t>& v)
{
    if (v.IsZero())
    {
        return v;
    }
    return v / Length(v);
}

template<typename Scalar_t>
inline Scalar_t Dot(const bhVec3<Scalar_t>& v0, const bhVec3<Scalar_t>& v1)
{
    return (v0.x * v1.x) + (v0.y * v1.y) + (v0.z * v1.z);
}

inline bhVec3f Cross(const bhVec3f& v0, const bhVec3f& v1)
{
    return bhVec3f(
        (v0.y * v1.z) - (v1.y * v0.z),
        (v1.x * v0.z) - (v0.x * v1.z),
        (v0.x * v1.y) - (v1.x * v0.y)
    );
}

inline float AngleRads(const bhVec3f& v0, const bhVec3f& v1)
{
    if (v0.IsZero() || v1.IsZero())
    {
        return 0.f;
    }
    bhVec3f na = Normalize(v0);
    bhVec3f nb = Normalize(v1);
    return acosf(Dot(na, nb));
}

template<typename Scalar_t>
inline Scalar_t AngleDegs(const bhVec3<Scalar_t>& v0, const bhVec3<Scalar_t>& v1)
{
    return bhMath_Rad2Deg(AngleRads(v0, v1));
}

template<typename Scalar_t>
inline bhVec3<Scalar_t> Project(const bhVec3<Scalar_t>& v0, const bhVec3<Scalar_t>& v1) // Project a onto b
{
    if (v1.IsZero())
    {
        return bhVec3<Scalar_t>();
    }
    return v1 * (Dot(v0, v1) / Length(v1));
}

////////////////////////////////////////////////////////////////////////////////
// bhVec4
template<typename Scalar_t>
struct bhVec4
{
    union
    {
        struct
        {
            Scalar_t x, y, z, w;
        };
        struct
        {
            Scalar_t r, g, b, a;
        };
        Scalar_t _v[4];
    };

    bhVec4()
        : x(Scalar_t(0)), y(Scalar_t(0)), z(Scalar_t(0)), w(Scalar_t(1))
    {}

    bhVec4(Scalar_t _x, Scalar_t _y, Scalar_t _z)
        : x(_x), y(_y), z(_z), w(1.f)
    {}

    bhVec4(Scalar_t _x, Scalar_t _y, Scalar_t _z, Scalar_t _w)
        : x(_x), y(_y), z(_z), w(_w)
    {}

    bhVec4(const bhVec4<Scalar_t>& v)
        : x(v.x), y(v.y), z(v.z), w(v.w)
    {}

    bhVec4(const bhVec3<Scalar_t>& v)
        :x(v.x), y(v.y), z(v.z), w(Scalar_t(1))
    {}

    inline bool IsZero() const
    {
        return (x == Scalar_t(0)) && (y == Scalar_t(0)) && (z == Scalar_t(0)) && (w == Scalar_t(0));
    }

    bhVec4& operator+=(const bhVec4& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        //w += v.w; //??
        return *this;
    }

    bhVec4& operator-=(const bhVec4& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        //w -= v.w; //??
        return *this;
    }

    bhVec4& operator*=(const bhVec4& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        //w *= v.w; //??
        return *this;
    }
};

typedef bhVec4<float> bhVec4f;

template<typename Scalar_t>
inline Scalar_t Dot(const bhVec4<Scalar_t>& v0, const bhVec4<Scalar_t>& v1)
{
    return (v0.x * v1.x) + (v0.y * v1.y) + (v0.z * v1.z) + (v0.w * v1.w);
}

template<typename Scalar_t>
inline bhVec3<Scalar_t> ToVec3f(const bhVec4<Scalar_t>& v)
{
    return bhVec3<Scalar_t>(v.x, v.y, v.z);
}

////////////////////////////////////////////////////////////////////////////////
#pragma warning(default : 4201)

#endif //BH_VECTOR_HPP

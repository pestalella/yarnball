#include "Point.h"
#include <cmath>

Vec3::Vec3(float x, float y, float z) :
    x(x), y(y), z(z)
{
}

float Vec3::distSq(const Vec3 &p) const
{
    return (p.x - x)*(p.x - x) + (p.y - y)*(p.y - y);
}

float Vec3::len() const
{
    return sqrt((*this)*(*this));
}

Vec3 Vec3::operator+(const Vec3 &p) const
{
    return Vec3(p.x + x, p.y + y, p.z + z);
}

Vec3 Vec3::operator-(const Vec3 &p) const
{
    return Vec3(x - p.x, y - p.y, z - p.z);
}

float Vec3::operator*(const Vec3 &p) const
{
    return x*p.x + y*p.y + z*p.z;
}

Vec3 Vec3::operator^(const Vec3 &p) const
{
    return Vec3(y*p.z - z*p.y,
                z*p.x - x*p.z,
                x*p.y - y*p.x);
}

Vec3 Vec3::operator*(float f) const
{
    return Vec3(x*f, y*f, z*f);
}

// unit vector
Vec3 Vec3::normalized() const
{
    return *this * (1.0/len());
}

#pragma once
class Vec3
{
public:
    float x, y, z;

public:
    Vec3(float x, float y, float z);
    
    // Operations
    float distSq(const Vec3 &p) const;
    // addition
    Vec3 operator+(const Vec3 &p) const;
    // subtraction
    Vec3 operator-(const Vec3 &p) const;
    // Dot product
    float operator*(const Vec3 &p) const;
    // cross product
    Vec3 operator^(const Vec3 &p) const;
    Vec3 operator*(float f) const;
};


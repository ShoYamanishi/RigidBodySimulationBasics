#ifndef __VEC2_HPP__
#define __VEC2_HPP__

#include <stdio.h>
#include <cmath>

struct Vec2 {

    Vec2()
        :x{ 0.0f }
        ,y{ 0.0f }
    {
    }

    Vec2( const float cx, const float cy )
        :x{ cx }
        ,y{ cy }
    {
    }

    void reset()
    {
        x = 0.0f;
        y = 0.0f;
    }

    float x;
    float y;

    float dot( const Vec2& rhs ) const
    {
        return this->x * rhs.x + this->y * rhs.y;
    }

    Vec2 operator+( const Vec2& rhs ) const
    {
        return Vec2{ this->x + rhs.x, this->y + rhs.y };
    }

    Vec2 operator-( const Vec2& rhs ) const
    {
        return Vec2{ this->x - rhs.x, this->y - rhs.y };
    }

    Vec2& operator+=( const Vec2& rhs )
    {
        this->x += rhs.x;
        this->y += rhs.y;
        return *this;
    }

    Vec2& operator*=( const float rhs )
    {
        this->x *= rhs;
        this->y *= rhs;
        return *this;
    }

    Vec2 operator*( const float rhs ) const
    {
        return Vec2{ this->x * rhs, this->y * rhs };
    }

    Vec2 operator/( const float rhs ) const
    {
        return Vec2{ this->x / rhs, this->y / rhs };
    }
    
    Vec2 perp() const
    {
        return Vec2{ -1.0f * y, x };
    }
    
    float length() const
    {
        return sqrt( this->x * this->x + this->y * this->y );
    }

    float sq_length() const
    {
        return this->x * this->x + this->y * this->y;
    }

    void normalize()
    {
        const auto len = length();
        x /= len;
        y /= len;
    }
};
#endif /* __VEC2_HPP__*/



#ifndef __VEC3_HPP__
#define __VEC3_HPP__

struct Vec3 {

    Vec3()
        :x{ 0.0f }
        ,y{ 0.0f }
        ,z{ 0.0f }
    {
    }

    Vec3( const float cx, const float cy, const float cz )
        :x{ cx }
        ,y{ cy }
        ,z{ cx }
    {
    }

    void reset()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    float x;
    float y;
    float z;
};

#endif /*__VEC#_HPP__*/


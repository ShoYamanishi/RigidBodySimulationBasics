#ifndef __VEC4_HPP__
#define __VEC4_HPP__

struct Vec4 {

    Vec4()
        :x{ 0.0f }
        ,y{ 0.0f }
        ,z{ 0.0f }
        ,w{ 0.0f }
    {
    }

    Vec4( const float cx, const float cy, const float cz, const float cw )
        :x{ cx }
        ,y{ cy }
        ,z{ cx }
        ,w{ cw }
    {
    }

    void reset()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;
    }

    float x;
    float y;
    float z;
    float w;
};

#endif /*__VEC4_HPP__*/

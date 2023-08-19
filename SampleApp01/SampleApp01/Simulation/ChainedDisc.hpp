#ifndef __DISC_HPP__
#define __DISC_HPP__

#include "RigidBody.hpp"
#include "Vec4.hpp"

class ChainedDisc : public RigidBody {

public:
    ChainedDisc( const float mass, const float radius, const Vec4& color )
        :RigidBody{ mass }
        ,m_radius { radius }
        ,m_prev   { nullptr }
        ,m_next   { nullptr }
        ,m_color  { color }
    {
    }

    virtual ~ChainedDisc()
    {
    }

    float m_radius;
    Vec4  m_color;

    ChainedDisc* m_prev;
    ChainedDisc* m_next;
};

#endif /*__DISC_HPP__*/

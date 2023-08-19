#ifndef __VELOCITY_CONSTRAINT_HPP__
#define __VELOCITY_CONSTRAINT_HPP__

#include "Vec2.hpp"
#include "RigidBody.hpp"

class VelocityConstraint {

public:
    typedef enum _Type {
        Unilateral,
        Bilateral
    } Type;

    VelocityConstraint(
        Type       type,
        RigidBody* body_0,
        RigidBody* body_1,
        Vec2       n0,
        Vec2       n1,
        float      b
    )
        :m_type   { type }
        ,m_body_0 { body_0 }
        ,m_body_1 { body_1 }
        ,m_n0     { n0 }
        ,m_n1     { n1 }
        ,m_b      { b }
        ,m_lambda { 0.0f }
    {
    }

    Type       m_type;
    RigidBody* m_body_0;
    RigidBody* m_body_1;
    Vec2       m_n0;
    Vec2       m_n1;
    float      m_b;
    float      m_lambda;
};

#endif /*__VELOCITY_CONSTRAINT_HPP__*/

#ifndef __RIGID_BODY_HPP__
#define __RIGID_BODY_HPP__
#include "Vec2.hpp"

class RigidBody {

public:
    RigidBody( const float mass )
        :m_mass        { mass }
        ,m_mass_inv    { 1.0f / mass }
        ,m_com         { }
        ,m_lin_vel     { }
        ,m_com_tmp     { }
        ,m_lin_vel_tmp { }
        ,m_force       { }
        ,m_lin_impulse { }
    {
    }

    virtual ~RigidBody()
    {
    }

    virtual void resetForcesAndImpulses()
    {
        m_force.reset();
        m_lin_impulse.reset();
    }

    void accumulateForce( const Vec2& f )
    {
        m_force += f;
    }

    void addImpulse( const Vec2& imp )
    {
        m_lin_impulse += imp;
    }

    void setPosition( const Vec2& p )
    {
        m_com = p;
    }

    virtual void updatePhaseSpaceTmp( const float delta_t )
    {
        m_lin_vel_tmp = m_lin_vel + m_force * delta_t * m_mass_inv;
        m_com_tmp     = m_com + m_lin_vel_tmp * delta_t;
    }

    virtual void updatePhaseSpace( const float delta_t )
    {
        m_lin_vel = m_lin_vel + ( m_force * delta_t + m_lin_impulse ) * m_mass_inv ;
        m_com     = m_com + m_lin_vel * delta_t;
        m_lin_vel *= 0.999f;
    }

    const float m_mass;
    const float m_mass_inv;

    Vec2        m_com;
    Vec2        m_lin_vel;
    Vec2        m_com_tmp;
    Vec2        m_lin_vel_tmp;
    Vec2        m_force;
    Vec2        m_lin_impulse;
};

#endif /*__RIGID_BODY_HPP__*/

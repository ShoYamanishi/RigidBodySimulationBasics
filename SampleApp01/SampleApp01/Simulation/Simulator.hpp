#ifndef __SIMULATOR_HPP__
#define __SIMULATOR_HPP__

#include <vector>
#include <random>
#include <iostream>

#include "ChainedDisc.hpp"
#include "Vec3.hpp"

#include "ConstraintsSolver.hpp"

class Simulator {

public:
    static constexpr float G = 9.81f; // G = 9.81 [m/(s^2)]

    static constexpr float EPSILON = 1.0e-04;

    static constexpr float AREA_WIDTH             = 1.5f;
    static constexpr float AREA_HEIGHT            = 1.0f;
    static constexpr float AREA_DEPTH             = -2.0f;
    static constexpr int   MAX_DISCS              = 100;
    static constexpr int   MAX_TRIANGLES_PER_DISC = 32;

    Simulator()
        :m_area_width        { AREA_WIDTH }
        ,m_area_height       { AREA_HEIGHT }
        ,m_area_width_target { AREA_WIDTH }
        ,m_area_height_target{ AREA_HEIGHT }
    {
        buildDiscs();
    }

    ~Simulator()
    {
        for ( auto* disc : m_discs ) {
            delete disc;
        }
    }

    void setTargetAreaSize( const float width, const float height )
    {
        m_area_width_target  = width;
        m_area_height_target = height;
    }

    void update( const float delta_t, const Vec2& accel, float torsional_spring_strength )
    {
        updateAreaSize();

        for ( auto* disc : m_discs ) {

            disc->resetForcesAndImpulses();
            disc->accumulateForce( accel * disc->m_mass * G );
            addTorsionalSpringForce( disc, torsional_spring_strength );
        }

        for ( auto* disc : m_discs ) {

            disc->updatePhaseSpaceTmp( delta_t );
        }

        detectCollisions( delta_t );

        constructBilateralConstraints( delta_t );

        m_constraints_solver.reset();
        for ( auto* c : m_constraints ) {

            m_constraints_solver.add( c );
        }

        m_constraints_solver.run( delta_t );

        for ( auto* c : m_constraints ) {

            if ( c->m_body_0 != nullptr ) {
                 c->m_body_0->addImpulse( c->m_n0 * c->m_lambda );
            }

            if ( c->m_body_1 != nullptr ) {
                 c->m_body_1->addImpulse( c->m_n1 * c->m_lambda );
            }
            delete c;
        }
        m_constraints.clear();

        for ( auto* disc : m_discs ) {

            disc->updatePhaseSpace( delta_t );
        }
    }

    std::vector< ChainedDisc* >& getDiscs()
    {
        return m_discs;
    }

private:

    void detectCollisions( const float delta_t )
    {
        for ( int i = 0; i < m_discs.size(); i++ ) {

            for ( int j = i + 1; j < m_discs.size(); j++ ) {

                detectCollisionPair( m_discs[i], m_discs[j], delta_t );

            }

            detectCollisionAgainstWalls( m_discs[i], delta_t );
        }
    }

    void detectCollisionPair( ChainedDisc* d0,  ChainedDisc* d1, const float delta_t )
    {
        if ( d0->m_next == d1 || d0->m_prev == d1 || d1->m_next == d0 || d1->m_prev == d0 ) {
            return;
        }

        const auto v_1_to_0_tmp = d0->m_com_tmp - d1->m_com_tmp;
        const auto sq_len_tmp   = v_1_to_0_tmp.sq_length();
        const auto min_dist = d0->m_radius + d1->m_radius;

        if ( sq_len_tmp <= min_dist * min_dist + EPSILON ) {

            const auto v_1_to_0 = d0->m_com - d1->m_com;
            const auto len = v_1_to_0.length();
            if ( len >= EPSILON ) {
                const auto signed_dist = len - min_dist;
            
                const auto n0 = v_1_to_0 / len;
                const auto n1 = n0 * -1.0f;

                auto* constraint = new VelocityConstraint{ VelocityConstraint::Unilateral, d0, d1, n0, n1, -1.0f * signed_dist / delta_t };
                m_constraints.push_back( constraint );
            }
        }
    }

    void detectCollisionAgainstWalls( ChainedDisc* d0, const float delta_t )
    {
        if ( d0->m_com_tmp.x - d0->m_radius <= -0.5f * m_area_width ) {

            const auto signed_dist = d0->m_com.x - d0->m_radius + 0.5f * m_area_width;
            const Vec2 n0{ 1.0f, 0.0f };

            auto* constraint = new VelocityConstraint{ VelocityConstraint::Unilateral, d0, nullptr, n0, n0, -1.0f * signed_dist / delta_t };
            m_constraints.push_back( constraint );
        }

        if ( d0->m_com_tmp.x + d0->m_radius >= 0.5f * m_area_width ) {

            const auto signed_dist = -1.0f * ( d0->m_com.x + d0->m_radius - 0.5f * m_area_width );
            const Vec2 n0{ -1.0f, 0.0f };

            auto* constraint = new VelocityConstraint{ VelocityConstraint::Unilateral, d0, nullptr, n0, n0, -1.0f * signed_dist / delta_t };
            m_constraints.push_back( constraint );
        }

        if ( d0->m_com_tmp.y - d0->m_radius <= -0.5f * m_area_height ) {

            const auto signed_dist = d0->m_com.y - d0->m_radius + 0.5f * m_area_height;
            const Vec2 n0{ 0.0f, 1.0f };

            auto* constraint = new VelocityConstraint{ VelocityConstraint::Unilateral, d0, nullptr, n0, n0, -1.0f * signed_dist / delta_t };
            m_constraints.push_back( constraint );
        }

        if ( d0->m_com_tmp.y + d0->m_radius >= 0.5f * m_area_height ) {

            const auto signed_dist = -1.0f * ( d0->m_com.y + d0->m_radius - 0.5f * m_area_height );
            const Vec2 n0{ 0.0f, -1.0f };

            auto* constraint = new VelocityConstraint{ VelocityConstraint::Unilateral, d0, nullptr, n0, n0, -1.0f * signed_dist / delta_t };
            m_constraints.push_back( constraint );
        }
    }

    void addTorsionalSpringForce( ChainedDisc* d1, float intensity )
    {
        if ( d1->m_next != nullptr && d1->m_prev != nullptr ) {

            auto* d0 = d1->m_prev;
            auto* d2 = d1->m_next;

            auto vec_01 = d1->m_com - d0->m_com;
            auto vec_12 = d2->m_com - d1->m_com;
            const auto len_01 = vec_01.length();
            const auto len_12 = vec_12.length();

            vec_01.normalize();
            vec_12.normalize();
            auto vec_01_perp = vec_01.perp();
            auto vec_12_perp = vec_12.perp();

            auto v01_rel = ( d0->m_lin_vel - d1->m_lin_vel ) / len_01;
            auto v21_rel = ( d2->m_lin_vel - d1->m_lin_vel ) / len_12;
            auto ang_vel_0 = v01_rel.dot( vec_01_perp ) * -1.0f;
            auto ang_vel_2 = v21_rel.dot( vec_12_perp );

            auto rel_ang_vel = ang_vel_2 - ang_vel_0;

            float signed_magnitude = 0.0f;
           
            // angular friction
            const float friction_coeff = 0.01f;
            d0->accumulateForce( vec_01_perp * (rel_ang_vel * friction_coeff * -1.0f * d0->m_mass ) );
            d1->accumulateForce( vec_01_perp * (rel_ang_vel * friction_coeff *  1.0f * d1->m_mass ) );
            d1->accumulateForce( vec_12_perp * (rel_ang_vel * friction_coeff *  1.0f * d1->m_mass ) );
            d2->accumulateForce( vec_12_perp * (rel_ang_vel * friction_coeff * -1.0f * d2->m_mass ) );

            if ( vec_01.dot( vec_12 ) > 0.0f ) {
                signed_magnitude = vec_01.dot( vec_12_perp );
            }
            else {
                if ( vec_01.dot( vec_12_perp ) > 0.0f ) {
                    signed_magnitude = 1.0f;
                }
                else {
                    signed_magnitude = -1.0f;
                }
            }
            signed_magnitude *= ( G * intensity );

            d0->accumulateForce( vec_01_perp * (signed_magnitude *  10.0f * d0->m_mass ) );
            d1->accumulateForce( vec_01_perp * (signed_magnitude * -10.0f * d1->m_mass ) );
            d1->accumulateForce( vec_12_perp * (signed_magnitude * -10.0f * d1->m_mass ) );
            d2->accumulateForce( vec_12_perp * (signed_magnitude *  10.0f * d2->m_mass ) );
        }
    }

    void constructBilateralConstraints( const float delta_t )
    {
        for ( auto* disc : m_discs ) {

            if ( disc->m_next != nullptr ) {

                linkTwoDiscs( disc, disc->m_next, delta_t );
            }
        }
    }

    void linkTwoDiscs( ChainedDisc* d0, ChainedDisc* d1, const float delta_t )
    {
        const auto v_1_to_0    = d0->m_com - d1->m_com;
        const auto len         = v_1_to_0.length();
        const auto signed_dist = len - ( d0->m_radius + d1->m_radius );
            
        const auto n0 = v_1_to_0 / len;
        const auto n1 = n0 * -1.0f;

        auto* constraint = new VelocityConstraint{ VelocityConstraint::Bilateral, d0, d1, n0, n1, -1.0f * signed_dist / delta_t };
        m_constraints.push_back( constraint );
    }

    void buildDiscs()
    {
        auto* p0 = new ChainedDisc{ 0.1, 0.05, randomColor() };
        p0->setPosition( Vec2{ -0.3f, -0.1f } );
        m_discs.push_back( p0 );

        auto* p1 = new ChainedDisc{ 0.1, 0.05, randomColor() };
        p1->setPosition( Vec2{ -0.2f, -0.1f } );
        m_discs.push_back( p1 );

        auto* p2 = new ChainedDisc{ 0.1, 0.05, randomColor() };
        p2->setPosition( Vec2{ -0.1f, -0.1f } );
        m_discs.push_back( p2 );

        auto* p3 = new ChainedDisc{ 0.1, 0.05, randomColor() };
        p3->setPosition( Vec2{ 0.0f, -0.1f } );
        m_discs.push_back( p3 );

        auto* p4 = new ChainedDisc{ 0.1, 0.05, randomColor() };
        p4->setPosition( Vec2{ 0.10f, -0.1f } );
        m_discs.push_back( p4 );

        auto* p5 = new ChainedDisc{ 0.1, 0.05, randomColor() };
        p5->setPosition( Vec2{ 0.20f, -0.1f } );
        m_discs.push_back( p5 );

        auto* p6 = new ChainedDisc{ 0.1, 0.05, randomColor() };
        p6->setPosition( Vec2{ 0.30f, -0.1f } );
        m_discs.push_back( p6 );

        p0->m_next = p1;
        p1->m_prev = p0;
        p1->m_next = p2;
        p2->m_prev = p1;
        p2->m_next = p3;
        p3->m_prev = p2;
        p3->m_next = p4;
        p4->m_prev = p3;
        p4->m_next = p5;
        p5->m_prev = p4;
        p5->m_next = p6;
        p6->m_prev = p5;

        auto* p7 = new ChainedDisc{ 0.2, 0.1, randomColor() };
        p7->setPosition( Vec2{ -0.30f, 0.3f } );
        m_discs.push_back( p7 );

        auto* p8 = new ChainedDisc{ 0.4, 0.15, randomColor() };
        p8->setPosition( Vec2{ 0.0f, 0.3f } );
        m_discs.push_back( p8 );

        auto* p9 = new ChainedDisc{ 0.2, 0.08, randomColor() };
        p9->setPosition( Vec2{ 0.4f, 0.3f } );
        m_discs.push_back( p9 );
    }

    void updateAreaSize()
    {
        // gradually change the area size.
        const auto diff_width  = std::max( -0.001f, std::min( 0.001f, m_area_width_target  - m_area_width  ) );
        const auto diff_height = std::max( -0.001f, std::min( 0.001f, m_area_height_target - m_area_height ) );
        m_area_width  += diff_width;
        m_area_height += diff_height;
    }

    Vec4 randomColor()
    {
        std::uniform_real_distribution<float> dist{ 120.0f, 180.0f };

        return Vec4{
            dist( m_random_engine ) / 256.0f,
            dist( m_random_engine ) / 256.0f,
            dist( m_random_engine ) / 256.0f,
            1.0f
        };
    }

    float                              m_area_width;
    float                              m_area_height;
    float                              m_area_width_target;
    float                              m_area_height_target;

    std::vector< ChainedDisc* >        m_discs;
    std::vector< VelocityConstraint* > m_constraints;

    ConstraintsSolver                  m_constraints_solver;

    std::default_random_engine         m_random_engine;
};

#endif /*__SIMULATOR_HPP__*/



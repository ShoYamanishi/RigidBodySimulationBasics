#ifndef __CONSTRAINTS_SOLVER_HPP__
#define __CONSTRAINTS_SOLVER_HPP__

#include <vector>
#include <map>

#include "VelocityConstraint.hpp"
#include "MLCPSolverVanillaPGS.hpp"

class ConstraintsSolver {

public:

    ConstraintsSolver()
        :m_mlcp{ 1.0e-8 /* epsilon */, 1000 /* max iter */, 5 /* error stagnation */ }
        ,m_cfm_sigma{ 1.0e-6 }
        ,m_cfm_gamma{ 0.999 }
    {
    }

    ~ConstraintsSolver()
    {
    }

    void reset()
    {
        m_unilateral.clear();
        m_bilateral.clear();
    }

    void add( VelocityConstraint* c )
    {
        if ( c->m_type == VelocityConstraint::Unilateral ) {

            m_unilateral.push_back(c);
        }
        else {
            m_bilateral.push_back(c);
        }
    }

    void run( const float delta_t )
    {
        const auto dim_bi  = (int32_t)m_bilateral.size();
        const auto dim_uni = (int32_t)m_unilateral.size();

        m_mlcp.prepare( dim_bi + dim_uni );

        constructMandQ( delta_t );

        m_mlcp.run();

        assignLambdas();
    }
    
    void constructMandQ( const float delta_t )
    {
        const auto dim_bi  = m_bilateral.size();
        const auto dim_uni = m_unilateral.size();

        for ( int i = 0; i < dim_bi + dim_uni; i++ ) {

            auto& c_i = (i < dim_bi) ? m_bilateral[ i ] : m_unilateral[ i - dim_bi ];

            for ( int j = i; j < dim_bi + dim_uni; j++ ) {

                float M_ij = 0.0f;

                auto& c_j = (j < dim_bi) ? m_bilateral[ j ] : m_unilateral[ j - dim_bi ];

                if ( c_i->m_body_0 != nullptr ) {

                    if ( c_i->m_body_0 == c_j->m_body_0 ) {

                        M_ij += ( c_i->m_n0.dot( c_j->m_n0 ) * c_i->m_body_0->m_mass_inv );
                    }
                    else if ( c_i->m_body_0 == c_j->m_body_1 ) {

                        M_ij += ( c_i->m_n0.dot( c_j->m_n1 ) * c_i->m_body_0->m_mass_inv );
                    }
                }

                if ( c_i->m_body_1 != nullptr ) {

                    if ( c_i->m_body_1 == c_j->m_body_0 ) {

                        M_ij += ( c_i->m_n1.dot( c_j->m_n0 ) * c_i->m_body_1->m_mass_inv );
                    }
                    else if ( c_i->m_body_1 == c_j->m_body_1 ) {

                        M_ij += ( c_i->m_n1.dot( c_j->m_n1 ) * c_i->m_body_1->m_mass_inv );
                    }
                }

                if ( i == j ) {
                    m_mlcp.setM( i, j, M_ij + m_cfm_sigma );
                }
                else {
                    m_mlcp.setM( i, j, M_ij );
                    m_mlcp.setM( j, i, M_ij );
                }
            }

            float q_i = -1.0f * c_i->m_b;

            if ( c_i->m_body_0 != nullptr ) {

                q_i += c_i->m_n0.dot( c_i->m_body_0->m_lin_vel + c_i->m_body_0->m_force * delta_t * c_i->m_body_0->m_mass_inv );
            }

            if ( c_i->m_body_1 != nullptr ) {

                q_i += c_i->m_n1.dot( c_i->m_body_1->m_lin_vel + c_i->m_body_1->m_force * delta_t * c_i->m_body_1->m_mass_inv );
            }

            q_i *= m_cfm_gamma;
            m_mlcp.setQ( i, q_i );

            if ( i < dim_bi ) {

                m_mlcp.setNoLimits( i );
            }
            else {
                m_mlcp.setUnilateralLimits( i );
            }
        }
    }

    void assignLambdas()
    {
        const auto dim_bi  = m_bilateral.size();
        const auto dim_uni = m_unilateral.size();

        for ( int i = 0; i < dim_bi + dim_uni; i++ ) {

            auto& c = (i < dim_bi) ? m_bilateral[ i ] : m_unilateral[ i - dim_bi ];
            c->m_lambda = m_mlcp.getZ( i );
         }
    }

private:

    MLCPSolverVanillaPGS<float>        m_mlcp;
    const float                        m_cfm_sigma;
    const float                        m_cfm_gamma;

    std::vector< VelocityConstraint* > m_unilateral;
    std::vector< VelocityConstraint* > m_bilateral;
};

#endif /*__CONSTRAINTS_SOLVER_HPP__*/

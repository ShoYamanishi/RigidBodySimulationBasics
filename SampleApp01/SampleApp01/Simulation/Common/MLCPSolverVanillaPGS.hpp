#ifndef __MLCP_SOLVER_VANILLA_PGS_HPP__
#define __MLCP_SOLVER_VANILLA_PGS_HPP__

#include <vector>
#include <cstring>
template<class T>
class MLCPSolverVanillaPGS {

    // Type of problem that can be solved:
    //
    //   M z + q = w
    //
    //   s.t. 0 <= z cmpl. w >= 0
    //
    //   M must be PD.
    //
    //   The iteration formula
    //   z^{r+1} = - (q + L z^{r+1} + U z^r) / D

public:

    MLCPSolverVanillaPGS(
        const T       epsilon,
        const int32_t max_num_iterations,
        const int32_t max_stagnation
    )
        :m_epsilon            { epsilon }
        ,m_base               { nullptr }
        ,m_allocated_dim      { 0 }
        ,m_max_num_iterations { max_num_iterations }
        ,m_max_stagnation     { max_stagnation }
    {
        static_assert(    std::is_same< float, T >::value
                       || std::is_same< double,T >::value );
    }

    ~MLCPSolverVanillaPGS()
    {
        releaseMemory();
    }

    void prepare( const int32_t dim )
    {
        m_dim = dim;
        allocateMemory( dim );
        m_error_history.clear();
        m_iterations = 0;

        memset( m_M, 0, sizeof(T) * m_dim * m_dim );
        memset( m_q, 0, sizeof(T) * m_dim );
        memset( m_z, 0, sizeof(T) * this->m_dim );
    }

    void setNoLimits( const int32_t i )
    {
        m_z_lo[i] = -1.0 * std::numeric_limits<T>::max();
        m_z_hi[i] = std::numeric_limits<T>::max();
    }

    void setUnilateralLimits( const int32_t i )
    {
        m_z_lo[i] = 0.0;
        m_z_hi[i] = std::numeric_limits<T>::max();
    }

    void setLimits( const int32_t i, const float lo, const float hi )
    {
        m_z_lo[i] = lo;
        m_z_hi[i] = hi;
    }

    void setM( const int32_t i, const int32_t j, const float v )
    {
        m_M[ i * m_dim + j ] = v;
    }

    void setQ( const int32_t i, const float v )
    {
        m_q[ i ] = v;
    }

    void run()
    {
        for ( m_iterations = 0; m_iterations < m_max_num_iterations; m_iterations++ ) {
        
            calcZ();

            calcMeritError();

            if ( checkForErrorStagnation() ) {
                break;
            }
        }
    }

    const T getZ( const int32_t i ) const
    {
        return this->m_z[i];
    }

    T getError() const
    {
        if ( m_error_history.empty() ) {
            return 0.0;
        }
        return *m_error_history.rbegin();
    }

private:

    void allocateMemory( const int32_t requested_dim )
    {
        if ( m_allocated_dim < requested_dim ) {

            releaseMemory();

            const int32_t dim = requested_dim * 2;

            m_base = new T[ ( dim + 5 ) * dim ];

            m_M    = m_base;
            m_q    = &(m_base[ ( dim     ) * dim ]);
            m_z    = &(m_base[ ( dim + 1 ) * dim ]);
            m_w    = &(m_base[ ( dim + 2 ) * dim ]);
            m_z_lo = &(m_base[ ( dim + 3 ) * dim ]);
            m_z_hi = &(m_base[ ( dim + 4 ) * dim ]);

            m_allocated_dim = dim;
        }
    }

    void releaseMemory()
    {
        if ( m_base != nullptr ) {

            delete[] m_base;
            m_base = nullptr;
        }
    }

    void calcZ()
    {
        // calc z^{r+1} = - (q + L z^{r+1} + U z^r) / D

        for ( int32_t row = 0; row < m_dim; row++ ) {

            T diag = this->m_M[ row * this->m_dim + row ];

            T dot = 0.0;

            for ( int32_t col = 0; col < m_dim; col++ ) {

                dot += m_M[ row * m_dim + col ] * m_z[ col ];
            }

            m_z[row] = clamp(
                ( diag * m_z[ row ] - dot - m_q[ row ] ) / diag,
                m_z_lo[ row ],
                m_z_hi[ row ]
            );
        }
    }

    void calcMeritError()
    {
        T error = 0.0;

        for ( int32_t row = 0; row < m_dim; row++ ) {

            if (    ( m_z[row] > m_z_lo[row] + m_epsilon )
                 && ( m_z[row] < m_z_hi[row] - m_epsilon )
            ) {
                T dot = 0.0;

                for ( int32_t col = 0; col < m_dim; col++ ) {

                    dot += m_M[ row * m_dim + col ] * m_z[ col ];
                }
                const T mzq = dot + m_q[row];

                error += std::abs( mzq );
            }
        }

        m_error_history.push_back( error );
    }

    bool checkForErrorStagnation()
    {
        int32_t count{ 0 };

        for ( int i = 1; i < m_error_history.size(); i++ ) {

            if ( m_error_history[ i - 1 ] <  m_error_history[ i ] ) {
                count++;
            }
        }

        return count > m_max_stagnation;
    }

    T clamp( const T val, const T lo, const T hi )
    {
        return std::min ( std::max ( val, lo ), hi );
    }

    const T        m_epsilon;
    const int32_t  m_max_num_iterations;
    const int32_t  m_max_stagnation;
    std::vector<T> m_error_history;

    T*             m_base;
    int32_t        m_allocated_dim;

    int32_t        m_dim;
    T*             m_M;
    T*             m_q;
    T*             m_z;
    T*             m_w;
    T*             m_z_lo;
    T*             m_z_hi;
    int32_t        m_iterations;
};

#endif /*__MLCP_SOLVER_VANILLA_PGS_HPP__*/

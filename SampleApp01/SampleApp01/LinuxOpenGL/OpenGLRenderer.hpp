#ifndef __SAMPLE_APP_OPENGL_RENDERER_HPP__
#define __SAMPLE_APP_OPENGL_RENDERER_HPP__

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "OpenGLBasicShaders.hpp"

class OpenGLRenderer {

public:

    OpenGLRenderer(
        int max_num_discs,
        int num_triangles_per_disc
    )
        :m_max_num_discs          { max_num_discs }
        ,m_num_triangles_per_disc { num_triangles_per_disc }
        ,m_vertex_in              { new float   [ (num_triangles_per_disc + 1) * 2 ] }
        ,m_indices                { new uint16_t[ num_triangles_per_disc * 3 ] }
        ,m_coms                   { new float   [ max_num_discs * 2 ] }
        ,m_colors                 { new float   [ max_num_discs * 4 ] }
        ,m_radii                  { new float   [ max_num_discs ] }
    {
        constructBufferContents();

        glewExperimental = GL_TRUE;
        if ( glewInit() != GLEW_OK ) {
            exit(1);
        }

        m_shaders = new OpenGLBasicShaders();

        glGenVertexArrays( 1, &m_vao_01 );
        glBindVertexArray( m_vao_01 );

        glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );

        glGenBuffers( 1, &m_vertex_in_buffer );
        glBindBuffer( GL_ARRAY_BUFFER, m_vertex_in_buffer );
        glBufferData( GL_ARRAY_BUFFER, (m_num_triangles_per_disc + 1) * 2 * sizeof(float), m_vertex_in, GL_STATIC_DRAW );

        glGenBuffers( 1, &m_indices_buffer );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices_buffer );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_num_triangles_per_disc * 3 * sizeof(uint16_t), m_indices , GL_STATIC_DRAW );

        glGenBuffers( 1, &m_coms_buffer );
        glBindBuffer( GL_ARRAY_BUFFER, m_coms_buffer );
        glBufferData( GL_ARRAY_BUFFER, m_max_num_discs * 2 * sizeof(float), m_coms, GL_STATIC_DRAW );

        glGenBuffers( 1, &m_colors_buffer );
        glBindBuffer( GL_ARRAY_BUFFER, m_colors_buffer );
        glBufferData( GL_ARRAY_BUFFER, m_max_num_discs * 4 * sizeof(float), m_colors, GL_STATIC_DRAW );

        glGenBuffers( 1, &m_radii_buffer );
        glBindBuffer( GL_ARRAY_BUFFER, m_radii_buffer );
        glBufferData( GL_ARRAY_BUFFER, m_max_num_discs * sizeof(float), m_radii, GL_STATIC_DRAW );

	glGenBuffers( 1, &m_coms_buffer );
        glBindBuffer( GL_ARRAY_BUFFER, m_coms_buffer );
        glBufferData( GL_ARRAY_BUFFER, m_max_num_discs * 2 * sizeof(float), m_coms, GL_STREAM_DRAW );

	glGenBuffers( 1, &m_colors_buffer );
        glBindBuffer( GL_ARRAY_BUFFER, m_colors_buffer );
        glBufferData( GL_ARRAY_BUFFER, m_max_num_discs * 4 * sizeof(float), m_colors, GL_STREAM_DRAW );

	glGenBuffers( 1, &m_radii_buffer );
        glBindBuffer( GL_ARRAY_BUFFER, m_radii_buffer );
        glBufferData( GL_ARRAY_BUFFER, m_max_num_discs * sizeof(float), m_radii, GL_STREAM_DRAW );

        m_MVP = glGetUniformLocation( m_shaders->progID(), "MVP" );

    }

    ~OpenGLRenderer()
    {
        glDeleteBuffers( 1, &m_vertex_in_buffer );
        glDeleteBuffers( 1, &m_indices_buffer );
        glDeleteBuffers( 1, &m_coms_buffer );
        glDeleteBuffers( 1, &m_colors_buffer );
        glDeleteBuffers( 1, &m_radii_buffer );

        glDeleteVertexArrays( 1, &m_vao_01 );

        delete m_shaders;

        delete[] m_coms;
        delete[] m_colors;
        delete[] m_radii;
        delete[] m_vertex_in;
        delete[] m_indices;
    }

    void render( const float frame_width, const float frame_height, std::vector< ChainedDisc* >& discs )
    {
        updateInstanceBuffers( discs );

        const auto hw = frame_width  * 0.5f;
        const auto hh = frame_height * 0.5f;

        glUseProgram( m_shaders->progID() );

        glm::mat4 MVP = glm::ortho( -hw, hw, -hh, hh, 0.0f, 1000.0f );

        glUniformMatrix4fv( m_MVP, 1, GL_FALSE, &MVP[0][0] );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, m_vertex_in_buffer );
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, m_coms_buffer );
        glBufferSubData( GL_ARRAY_BUFFER, 0, discs.size() * 2 * sizeof(float), m_coms );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, m_colors_buffer );
        glBufferSubData( GL_ARRAY_BUFFER, 0, discs.size() * 4 * sizeof(float), m_colors );
        glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );

        glEnableVertexAttribArray( 3 );
        glBindBuffer( GL_ARRAY_BUFFER, m_radii_buffer );
        glBufferSubData( GL_ARRAY_BUFFER, 0, discs.size() * sizeof(float), m_radii );
        glVertexAttribPointer( 3, 1, GL_FLOAT, GL_FALSE, 0, (void*)0 );
           
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_indices_buffer );

        glClear( GL_COLOR_BUFFER_BIT );

        glVertexAttribDivisor(0, 0);
        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);

        glDrawElementsInstanced( GL_TRIANGLES, m_num_triangles_per_disc * 3, GL_UNSIGNED_SHORT, (void*)0, discs.size() );

        glDisableVertexAttribArray( 3 );
        glDisableVertexAttribArray( 2 );
        glDisableVertexAttribArray( 1 );
        glDisableVertexAttribArray( 0 );
    }

private:

    void constructBufferContents()
    {
        m_vertex_in[  0 ] = 0.0f;
        m_vertex_in[  1 ] = 0.0f;

        for ( int32_t i = 0; i < m_num_triangles_per_disc; i++ ) {

            m_vertex_in[ 2*(i+1)     ] = 1.0f;
            m_vertex_in[ 2*(i+1) + 1 ] = 2.0f * M_PI * (float)i / (float)m_num_triangles_per_disc;
        }

        for ( uint16_t i = 0; i < (uint16_t)m_num_triangles_per_disc; i++ ) {

            m_indices[ i * 3     ]  = 0;
            m_indices[ i * 3 + 1 ]  = i + 1;
            m_indices[ i * 3 + 2 ]  = ( i + 1 ) % m_num_triangles_per_disc + 1;
        }

        for ( int32_t i = 0; i < m_max_num_discs; i++ ) {

            m_coms[ 2*i     ] = 0.0f;
            m_coms[ 2*i + 1 ] = 0.0f;

            m_colors[ 4*i     ] = 0.0f;
            m_colors[ 4*i + 1 ] = 0.0f;
            m_colors[ 4*i + 2 ] = 0.0f;
            m_colors[ 4*i + 3 ] = 0.0f;

            m_radii[ i ] = 0.0f;
        }
    }

    void updateInstanceBuffers( std::vector< ChainedDisc* >& discs )
    {
        for ( int32_t i = 0; i < discs.size(); i++ ) {

            m_coms[ 2*i     ] = discs[i]->m_com.x;
            m_coms[ 2*i + 1 ] = discs[i]->m_com.y;

            m_colors[ 4*i     ] = discs[i]->m_color.x;
            m_colors[ 4*i + 1 ] = discs[i]->m_color.y;
            m_colors[ 4*i + 2 ] = discs[i]->m_color.z;
            m_colors[ 4*i + 3 ] = discs[i]->m_color.w;

            m_radii[ i ] =  discs[i]->m_radius;
        }
    }

    OpenGLBasicShaders* m_shaders;

    int       m_max_num_discs;
    int       m_num_triangles_per_disc;

    GLuint    m_vao_01;

    float*    m_vertex_in;
    uint16_t* m_indices;
    float*    m_coms;
    float*    m_colors;
    float*    m_radii;

    GLuint    m_vertex_in_buffer;
    GLuint    m_indices_buffer;
    GLuint    m_coms_buffer;
    GLuint    m_colors_buffer;
    GLuint    m_radii_buffer;
    GLuint    m_MVP;
};
#endif /*__SAMPLE_APP_OPENGL_RENDERER_HPP__*/

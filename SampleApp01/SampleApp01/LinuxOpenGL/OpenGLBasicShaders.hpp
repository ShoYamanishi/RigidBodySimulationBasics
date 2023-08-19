#ifndef __SAMPLE_APP_OPENGL_BASIC_SHADERS_HPP__
#define __SAMPLE_APP_OPENGL_BASIC_SHADERS_HPP__

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class OpenGLBasicShaders {

public:

    OpenGLBasicShaders()
    {
        m_vertex_id = glCreateShader( GL_VERTEX_SHADER   );
        m_frag_id   = glCreateShader( GL_FRAGMENT_SHADER );

        compile( m_vertex_id, VERTEX_STR );
        compile( m_frag_id,   FRAG_STR   );

        m_prog_id = link( m_vertex_id, m_frag_id );
    }

    ~OpenGLBasicShaders()
    {
        glDeleteProgram( m_prog_id   );
        glDeleteShader ( m_vertex_id );
        glDeleteShader ( m_frag_id   );
    }

    void compile( const GLuint id, const char* str )
    {
        GLint result = GL_FALSE;
        int   info_len = 0;

        glShaderSource( id, 1, &str , nullptr );
        glCompileShader( id );
        glGetShaderiv( id, GL_COMPILE_STATUS, &result );
        glGetShaderiv( id, GL_INFO_LOG_LENGTH, &info_len );

        if ( info_len > 0 ) {

            auto* p = new char[ info_len + 1 ];
            glGetShaderInfoLog( id, info_len, nullptr, p );
            std::cerr << p << "\n";
            delete[] p;
        }
    }

    GLuint link( const GLuint vertex_id,  const GLuint frag_id )
    {
        GLint result = GL_FALSE;
        int   info_len = 0;

        GLuint prog_id = glCreateProgram();

        glAttachShader( prog_id, vertex_id );
        glAttachShader( prog_id, frag_id   );

        glLinkProgram( prog_id );

        glGetProgramiv( prog_id, GL_LINK_STATUS, &result);
        glGetProgramiv( prog_id, GL_INFO_LOG_LENGTH, &info_len );

        if ( info_len > 0 ) {

            auto* p = new char[ info_len + 1 ];
            glGetProgramInfoLog( prog_id, info_len, NULL, p );
            delete[] p;
        }

        glDetachShader( prog_id, vertex_id );
        glDetachShader( prog_id, frag_id );

        glDeleteShader( vertex_id );
        glDeleteShader( frag_id );

        return prog_id;
    }

    GLuint progID() const { return m_prog_id; }

private:

    GLuint m_vertex_id;
    GLuint m_frag_id;
    GLuint m_prog_id;

    static constexpr const char* VERTEX_STR = "#version 330 core\n\
\n\
layout( location = 0 ) in vec2  vertex_in;\n\
layout( location = 1 ) in vec2  com;\n\
layout( location = 2 ) in vec4  color;\n\
layout( location = 3 ) in float radius;\n\
out vec4 color_vout;\n\
uniform mat4 MVP;\n\
\n\
void main() {\n\
    float rad = vertex_in.x * radius;\n\
    float ang = vertex_in.y;\n\
    vec4  pos_lcs = vec4( com.x + rad * cos(ang), com.y + rad * sin(ang), 0.0f, 1.0f );\n\
    gl_Position = MVP * pos_lcs;\n\
    color_vout   = color;\n\
}\n\
\n\
";

    static constexpr const char* FRAG_STR = "#version 330 core\n\
\n\
in  vec4 color_vout;\n\
out vec4 color_fout;\n\
\n\
void main() {\n\
    color_fout = color_vout;\n\
}\n\
\n\
";

};
#endif /*__SAMPLE_APP_OPENGL_BASIC_SHADERS_HPP__*/

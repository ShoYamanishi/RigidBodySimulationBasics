#ifndef __SAMPLE_APP_OPENGL_USER_INPUT_HPP__
#define __SAMPLE_APP_OPENGL_USER_INPUT_HPP__

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class UserInput {

public:
    UserInput( GLFWwindow* window )
        :m_window   { window }
        ,m_delta_t  { 0.0f }
        ,m_time_prev{ (float)glfwGetTime() }
        ,m_time_cur { (float)glfwGetTime() }
        ,m_x        { 0.0f }
        ,m_y        { 0.0f }
        ,m_left     { false }
        ,m_right    { false }
        ,m_up       { false }
        ,m_down     { false }
    {
    }

    ~UserInput()
    {
    }

    void update()
    {
        // timestamp
        m_time_cur = (float)glfwGetTime();
        m_delta_t = std::min( 1.0f / 60.0f, m_time_cur - m_time_prev );

        // mouse
        double x, y;
        glfwGetCursorPos( m_window, &x, &y );
        m_x = x;
        m_y = y;

        // keys
        m_left  = false;
        m_right = false;
        m_up    = false;
        m_down  = false;

        if ( glfwGetKey( m_window, GLFW_KEY_UP ) == GLFW_PRESS ) {
            m_up = true;
        }
        if ( glfwGetKey( m_window, GLFW_KEY_DOWN ) == GLFW_PRESS ) {
            m_down = true;
        }
        if ( glfwGetKey( m_window, GLFW_KEY_RIGHT ) == GLFW_PRESS ) {
            m_right = true;
        }
        if ( glfwGetKey( m_window, GLFW_KEY_LEFT ) == GLFW_PRESS ) {
            m_left = true;
        }

        glfwGetFramebufferSize( m_window, &m_frame_width, &m_frame_height );
        glfwGetWindowSize( m_window, &m_window_width, &m_window_height );
    }

    void advanceTime()
    {
        m_time_prev = m_time_cur;
    }

    float deltaT()       const { return m_delta_t; }
    float x()            const { return m_x; }
    float y()            const { return m_y; }
    bool  left()         const { return m_left; }
    bool  right()        const { return m_right; }
    bool  up()           const { return m_up; }
    bool  down()         const { return m_down; }
    int   windowWidth()  const { return m_window_width;  }
    int   windowHeight() const { return m_window_height; }
    int   frameWidth()   const { return m_frame_width;  }
    int   frameHeight()  const { return m_frame_height; }

    float normalizedWidth() const
    {
        if ( m_window_width > m_window_height ) {
            return (float)m_window_width / (float)m_window_height;
        }
        else {
            return 1.0f;
        }
    }

    float normalizedHeight() const
    {
        if ( m_window_width < m_window_height ) {
            return (float)m_window_height / (float)m_window_width;
        }
        else {
            return 1.0f;
        }
    }

    Vec2 normalizedCursorFromCenter() const
    {
        const auto x_offset = m_x - (float)m_window_width  / 2.0f;    
        const auto y_offset = m_y - (float)m_window_height / 2.0f;    
        const auto edge_len = std::max( (float)m_window_width, (float)m_window_height );

        return Vec2{ x_offset / edge_len, -1.0f * y_offset / edge_len };
    }

private:

    GLFWwindow* m_window;

    float       m_delta_t;
    float       m_time_prev;
    float       m_time_cur;

    float       m_x;
    float       m_y;

    bool        m_left;
    bool        m_right;
    bool        m_up;
    bool        m_down;

    int         m_window_width;
    int         m_window_height;
    int         m_frame_width;
    int         m_frame_height;
};
#endif /*__SAMPLE_APP_OPENGL_USER_INPUT_HPP__*/

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Simulator.hpp"
#include "UserInput.hpp"
#include "OpenGLRenderer.hpp"

int main( int argc, char* argv[] )
{
    Simulator sim;

    if( !glfwInit() ) {
        exit(1);
    }

    glfwWindowHint( GLFW_SAMPLES, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    GLFWwindow* window = glfwCreateWindow( 1152, 768, "SampleApp01", nullptr, nullptr );

    if( window == nullptr ) {
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent( window );

    OpenGLRenderer renderer( 100 /* max num discs */, 64 /*triangles per disc*/ );
    UserInput      ui( window );

    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    float torsional_spring_strength{ 0.0f };

    do {
        ui.update();
        if ( ui.deltaT() < 0.015f ) {
            glfwPollEvents();
            continue;
        }
        ui.advanceTime();

        glViewport( 0,0,ui.frameWidth(), ui.frameHeight() );

        sim.setTargetAreaSize( ui.normalizedWidth(), ui.normalizedHeight() );

        Vec2 accel = ui.normalizedCursorFromCenter();
        if ( accel.sq_length() > 1.0f ) {

            accel.normalize();
        }

        if ( ui.up() || ui.right() ) {

            torsional_spring_strength = std::min( 1.0f, torsional_spring_strength + 0.01f );
            std::cerr << "torsional spring strength: " << torsional_spring_strength << "\n";
        }

        else if ( ui.down() || ui.left() ) {

            torsional_spring_strength = std::max( 0.0f, torsional_spring_strength - 0.01f );
            std::cerr << "torsional spring strength: " << torsional_spring_strength << "\n";
        }

        sim.update( ui.deltaT(), accel, torsional_spring_strength );

        renderer.render( ui.normalizedWidth(), ui.normalizedHeight(), sim.getDiscs() );

        glfwSwapBuffers( window );

        glfwPollEvents();
    }
    while(    glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS
           && glfwWindowShouldClose(window) == 0
    );

    glfwTerminate();

    return 0;
}

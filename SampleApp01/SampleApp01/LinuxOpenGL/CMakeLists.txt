cmake_minimum_required( VERSION 3.10 )

project( sample_app_01 )

add_executable( sample_app_01 SampleApp01.cpp )

target_include_directories( sample_app_01 PRIVATE . ../Simulation/ ../Simulation/Common )

find_package( GLEW REQUIRED   )
find_package( OpenGL REQUIRED )

if( ${CMAKE_SYSTEM_NAME} MATCHES Darwin )

    target_compile_features( sample_app_01 PRIVATE cxx_std_17 )

    target_link_directories( sample_app_01 PRIVATE "glfw/build/src/" )

    target_include_directories( sample_app_01 PRIVATE "glfw/include" )

    target_link_libraries( sample_app_01 libglfw3.a GLEW::glew )
    target_link_libraries( sample_app_01 "-framework Cocoa" )
    target_link_libraries( sample_app_01 "-framework IOKit" )

else()

    find_package( glfw3 REQUIRED )
    target_link_libraries( sample_app_01 glfw GLEW::glew OpenGL )

endif()

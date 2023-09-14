cmake_minimum_required( VERSION 3.10 )

project( sample_app_01 )

add_executable( sample_app_01 SampleApp01.cpp )

target_include_directories( sample_app_01 PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../Simulation/
    ${PROJECT_SOURCE_DIR}/../Simulation/Common
)

find_package( GLEW   REQUIRED )
find_package( OpenGL REQUIRED )
find_package( glfw3  REQUIRED )

target_compile_features( sample_app_01 PRIVATE cxx_std_17 )

target_link_directories( sample_app_01 PRIVATE "/usr/local/lib" )


target_link_libraries( sample_app_01 GLEW::glew )

if( ${CMAKE_SYSTEM_NAME} MATCHES Darwin )
    target_link_libraries( sample_app_01 glfw3 )
    target_link_libraries( sample_app_01 "-framework Cocoa" )
    target_link_libraries( sample_app_01 "-framework IOKit" )
    target_link_libraries( sample_app_01 "-framework OpenGL" )

else()
    target_link_libraries( sample_app_01 glfw )
    target_link_libraries( sample_app_01 OpenGL )

endif()

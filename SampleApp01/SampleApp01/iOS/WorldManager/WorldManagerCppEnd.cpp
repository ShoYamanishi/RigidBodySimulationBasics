#include <iostream>

#include <simd/simd.h>

class WorldManagerShim; // This declaration is needed to keep WorldManagerReverseShim.h ObjC-free.
#include "WorldManagerCppEnd.hpp"
#include "WorldManagerReverseShim.h"
#include "AppleUtil.hpp"

#include "Vec3.hpp"

WorldManagerCppEnd::WorldManagerCppEnd( WorldManagerReverseShim* reverse_shim, MTL::Device* device )
    :m_reverse_shim{ reverse_shim     }
    ,m_device      { device->retain() }
    ,m_Mmodel      { identity() }
    ,m_Mview       { identity() }
    ,m_Mproj       { identity() }
    ,m_Mtrans      { identity() }
    ,m_accel       { 0.0f, 0.0f, 0.0f }
    ,m_simulator   { }
    ,m_renderer    { device }
{
}

WorldManagerCppEnd::~WorldManagerCppEnd()
{
    m_device ->release();
}

void WorldManagerCppEnd::createPipelineStates( const MTL::PixelFormat pixelFormat )
{
    m_renderer.createPipelineStates( pixelFormat );
}

void WorldManagerCppEnd::updateScreenSizes( float width, float height )
{
    ;
}

void WorldManagerCppEnd::setCameraMatrices( float* arrView, float* arrProj, float* arrTrans )
{
    m_Mview  = fromArray( arrView );
    m_Mproj  = fromArray( arrProj );
    m_Mtrans = fromArray( arrTrans );
    m_renderer.setCameraMatrices( m_Mview, m_Mproj, m_Mtrans );
}

void WorldManagerCppEnd::udpateWorld( float* acceleration, float torsionalSpringStrength )
{
    m_accel.x = acceleration[0];
    m_accel.y = acceleration[1];
    m_accel.z = acceleration[2];

    const Vec2 accel{ m_accel.x,  m_accel.y };

    // NOTE: ARKit's camera coordinate system has positive Y rightward of the device
    //       while the CoreMotion's coordinate system has positive Y uppward of the device
    //       the perp() below is to transform the CoreMotion's accel into ARKit's camera's.

    m_simulator.update( 1.0f / 60.0f, accel.perp(), torsionalSpringStrength );
}

void WorldManagerCppEnd::encode( MTL::RenderCommandEncoder* encoder )
{
    encoder->retain();

    m_renderer.renderFrame( encoder );
    m_renderer.renderDiscs( encoder, m_simulator.getDiscs() );

    encoder->release();
}

#ifndef __WORLD_MANAGER_CPP_END_HPP__
#define __WORLD_MANAGER_CPP_END_HPP__

#include <Foundation/Foundation.hpp>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "MetalRenderer.hpp"
#include "Simulator.hpp"

class WorldManagerReverseShim;

class WorldManagerCppEnd {
public:
     WorldManagerCppEnd( WorldManagerReverseShim* reverse_shim, MTL::Device* device );
    ~WorldManagerCppEnd();
    void createPipelineStates( const MTL::PixelFormat pixelFormat );
    void updateScreenSizes( float width, float height );
    void setCameraMatrices( float* Mview, float* Mproj, float* Mtrans );
    void udpateWorld( float* acceleration, float torsionalSpringStrength );
    void encode( MTL::RenderCommandEncoder* encoder );

private:

    WorldManagerReverseShim* m_reverse_shim;
    MTL::Device*             m_device;
    float4x4                 m_Mmodel;
    float4x4                 m_Mview;
    float4x4                 m_Mproj;
    float4x4                 m_Mtrans;
    float3                   m_accel;

    MetalRenderer            m_renderer;
    Simulator                m_simulator;
};

#endif /*  __WORLD_MANAGER_CPP_END_HPP__ */

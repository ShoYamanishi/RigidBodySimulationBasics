#ifndef __METAL_RENDERER_HPP__
#define __METAL_RENDERER_HPP__

#include <simd/simd.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "ChainedDisc.hpp"
#include "Simulator.hpp"

using namespace simd;

struct VertexInPositionNormalColor {
    float4 position;
    float4 normal;
    float4 color;
};

struct VertexInDiscInst {
    float unit_radius;
    float angle;
};

struct DiscInstance {
    float4 com;
    float4 color;
    float  radius;
};

class MetalRenderer {

public:

    MetalRenderer( MTL::Device*  device );
    ~MetalRenderer();

    void createPipelineStates( const MTL::PixelFormat pixelFormat );

    void setCameraMatrices(
        const matrix_float4x4& Mview,
        const matrix_float4x4& Mproj,
        const matrix_float4x4& Mtrans
    );

    void renderFrame( MTL::RenderCommandEncoder* encoder );
    void renderDiscs( MTL::RenderCommandEncoder* encoder, std::vector< ChainedDisc* >& discs );

private:

    void createDepthStecilState();

    void setupDiscBuffers();

    void setupAreaBuffers();

    void createPipelineStatesDiscInst( const MTL::PixelFormat pixelFormat );
    void createPipelineStatesPositionNormalColor( const MTL::PixelFormat pixelFormat );

    MTL::Device*               m_device;
    MTL::DepthStencilState*    m_depth_stencil_state;

    matrix_float4x4            m_Mmodel;
    matrix_float4x4            m_Mview;
    matrix_float4x4            m_Mproj;
    matrix_float4x4            m_Mtrans;

    float                      m_area_width;
    float                      m_area_height;
    float                      m_area_depth;

    const int32_t              m_max_num_discs;
    const int32_t              m_num_triangles_per_disc;
    MTL::RenderPipelineState*  m_pipeline_state_disc_inst;
    MTL::Buffer*               m_vertex_buffer_disc_inst;
    MTL::Buffer*               m_instance_buffer_disc_inst;
    MTL::Buffer*               m_index_buffer_disc_inst;

    MTL::RenderPipelineState*  m_pipeline_state_position_normal_color;
    MTL::Buffer*               m_vertex_buffer_position_normal_color;
    MTL::Buffer*               m_index_buffer_position_normal_color;
};

#endif /*__AD_HOC_METAL_RENDERER_HPP__*/

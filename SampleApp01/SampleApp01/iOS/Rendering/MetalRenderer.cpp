#include <math.h>
#include <iostream>

#include "MetalRenderer.hpp"
#include "AppleUtil.hpp"

MetalRenderer::MetalRenderer( MTL::Device* device )
    :m_device                 { device }
    ,m_Mmodel                 { identity() }
    ,m_Mview                  { identity() }
    ,m_Mproj                  { identity() }
    ,m_Mtrans                 { identity() }
    ,m_area_width             { Simulator::AREA_WIDTH }
    ,m_area_height            { Simulator::AREA_HEIGHT }
    ,m_area_depth             { Simulator::AREA_DEPTH }
    ,m_max_num_discs          { Simulator::MAX_DISCS }
    ,m_num_triangles_per_disc { Simulator::MAX_TRIANGLES_PER_DISC }
{
    setupDiscBuffers();
    setupAreaBuffers();

    createDepthStecilState();
}

MetalRenderer::~MetalRenderer()
{
    m_vertex_buffer_disc_inst->release();
    m_instance_buffer_disc_inst->release();
    m_index_buffer_disc_inst->release();
    m_pipeline_state_position_normal_color->release();

    m_vertex_buffer_position_normal_color->release();
    m_index_buffer_position_normal_color->release();
    m_pipeline_state_disc_inst->release();
}

void MetalRenderer::setCameraMatrices(
    const matrix_float4x4& Mview,
    const matrix_float4x4& Mproj,
    const matrix_float4x4& Mtrans
) {
    m_Mview  = Mview;
    m_Mproj  = Mproj;
    m_Mtrans = Mtrans;
    m_Mmodel = Mtrans; //inverseTranslationMatrix( Mtrans );
}

void MetalRenderer::renderFrame( MTL::RenderCommandEncoder* encoder )
{
    encoder->setRenderPipelineState( m_pipeline_state_position_normal_color );
    encoder->setDepthStencilState( m_depth_stencil_state );
    encoder->setVertexBuffer( m_vertex_buffer_position_normal_color,  0, 0 );
    encoder->setVertexBytes( &m_Mmodel, sizeof( float4x4 ), 1 );
    encoder->setVertexBytes( &m_Mview,  sizeof( float4x4 ), 2 );
    encoder->setVertexBytes( &m_Mproj,  sizeof( float4x4 ), 3 );
    encoder->setFragmentBytes( &m_Mtrans,  sizeof( float4x4 ), 1 );
    encoder->drawIndexedPrimitives( MTL::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, m_index_buffer_position_normal_color, 0 );
}

void MetalRenderer::renderDiscs( MTL::RenderCommandEncoder* encoder, std::vector< ChainedDisc* >& discs )
{
    if ( discs.empty() ) {
        return;
    }

    auto* inst_p = static_cast< DiscInstance* >( m_instance_buffer_disc_inst->contents() );

    for ( int i = 0; i < discs.size(); i++ ) {

        inst_p[i].com[0] = discs[i]->m_com.x;
        inst_p[i].com[1] = discs[i]->m_com.y;
        inst_p[i].com[2] = m_area_depth + 0.01f; // 1cm above the floor.
        inst_p[i].com[3] = 1.0f;

        inst_p[i].radius = discs[i]->m_radius;

        inst_p[i].color[0]  = discs[i]->m_color.x;
        inst_p[i].color[1]  = discs[i]->m_color.y;
        inst_p[i].color[2]  = discs[i]->m_color.z;
        inst_p[i].color[3]  = discs[i]->m_color.w;
    }

    encoder->setRenderPipelineState( m_pipeline_state_disc_inst );
    encoder->setDepthStencilState( m_depth_stencil_state );
    encoder->setVertexBuffer( m_vertex_buffer_disc_inst,  0, 0 );
    encoder->setVertexBytes( &m_Mmodel, sizeof( float4x4 ), 1 );
    encoder->setVertexBytes( &m_Mview,  sizeof( float4x4 ), 2 );
    encoder->setVertexBytes( &m_Mproj,  sizeof( float4x4 ), 3 );
    encoder->setVertexBuffer( m_instance_buffer_disc_inst,  0, 4 );
    encoder->setFragmentBytes( &m_Mtrans,  sizeof( float4x4 ), 1 );

    encoder->drawIndexedPrimitives( MTL::PrimitiveTypeTriangle, m_num_triangles_per_disc * 3, MTL::IndexTypeUInt16, m_index_buffer_disc_inst, 0, discs.size(), 0, 0 );
}

void MetalRenderer::setupDiscBuffers()
{
    m_vertex_buffer_disc_inst   = m_device->newBuffer( sizeof(VertexInDiscInst) * ( m_num_triangles_per_disc + 1 ), MTL::ResourceStorageModeShared );
    m_index_buffer_disc_inst    = m_device->newBuffer( sizeof(uint16_t) * m_num_triangles_per_disc * 3 , MTL::ResourceStorageModeShared );
    m_instance_buffer_disc_inst = m_device->newBuffer( sizeof(DiscInstance) * m_max_num_discs, MTL::ResourceStorageModeShared );

    auto* vertex_p = static_cast< VertexInDiscInst* >( m_vertex_buffer_disc_inst->contents() );

    vertex_p[0].unit_radius = 0.0f;
    vertex_p[0].angle       = 0.0f;

    for ( int32_t i = 0; i < m_num_triangles_per_disc; i++ ) {

        vertex_p[i+1].unit_radius = 1.0f;
        vertex_p[i+1].angle       = 2.0f * M_PI * (float)i / (float)m_num_triangles_per_disc;
    }

    auto* index_p = static_cast< uint16_t* >( m_index_buffer_disc_inst->contents() );

    for ( int32_t i = 0; i < m_num_triangles_per_disc; i++ ) {

        *index_p = 0;
        index_p++;
        *index_p = (uint16_t)i + 1;
        index_p++;
        *index_p = (uint16_t)( ( i + 1 ) % m_num_triangles_per_disc ) + 1;
        index_p++;
    }
}

void MetalRenderer::setupAreaBuffers()
{
    m_vertex_buffer_position_normal_color = m_device->newBuffer( sizeof(VertexInPositionNormalColor) * 4, MTL::ResourceStorageModeShared );
    m_index_buffer_position_normal_color  = m_device->newBuffer( sizeof(uint16_t) * 6, MTL::ResourceStorageModeShared );

    const float4 color { 0.1f, 0.2f, 0.1f, 0.3f };
    const float4 normal{ 0.0f, 0.0f, 1.0f, 0.0f };

    auto* vertex_p = static_cast< VertexInPositionNormalColor* >( m_vertex_buffer_position_normal_color->contents() );

    for ( int i = 0; i < 4; i++ ) {
        vertex_p[i].normal   = normal;
        vertex_p[i].color    = color;
    }

    vertex_p[0].position = float4{ -0.5f * m_area_width, -0.5f * m_area_height, m_area_depth, 1.0f };
    vertex_p[1].position = float4{  0.5f * m_area_width, -0.5f * m_area_height, m_area_depth, 1.0f };
    vertex_p[2].position = float4{  0.5f * m_area_width,  0.5f * m_area_height, m_area_depth, 1.0f };
    vertex_p[3].position = float4{ -0.5f * m_area_width,  0.5f * m_area_height, m_area_depth, 1.0f };

    auto* index_p = static_cast< uint16_t* >( m_index_buffer_position_normal_color->contents() );

    index_p[0] = 0;
    index_p[1] = 1;
    index_p[2] = 2;

    index_p[3] = 0;
    index_p[4] = 2;
    index_p[5] = 3;
}

void MetalRenderer::createPipelineStates( const MTL::PixelFormat pixelFormat )
{
    createPipelineStatesDiscInst( pixelFormat );
    createPipelineStatesPositionNormalColor( pixelFormat );
}

void MetalRenderer::createPipelineStatesDiscInst( const MTL::PixelFormat pixelFormat )
{
    MTL::Library* library = m_device->newDefaultLibrary();
    MTL::RenderPipelineDescriptor* pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();

    pipeline_descriptor->setVertexFunction  ( library->newFunction( NS::String::string( (const char*)"vertex_disc_inst", NS::UTF8StringEncoding) ) );
    pipeline_descriptor->setFragmentFunction( library->newFunction( NS::String::string( (const char*)"fragment_common",  NS::UTF8StringEncoding) ) );

    MTL::VertexDescriptor* vertex_descriptor = MTL::VertexDescriptor::alloc()->init();

    MTL::VertexAttributeDescriptor* attribute0 = MTL::VertexAttributeDescriptor::alloc()->init();
    attribute0->setFormat( MTL::VertexFormatFloat );
    attribute0->setOffset( offsetof(VertexInDiscInst, unit_radius) );
    attribute0->setBufferIndex( 0 );
    vertex_descriptor->attributes()->setObject( attribute0, 0 );

    MTL::VertexAttributeDescriptor* attribute1 = MTL::VertexAttributeDescriptor::alloc()->init();
    attribute1->setFormat( MTL::VertexFormatFloat );
    attribute1->setOffset( offsetof(VertexInDiscInst, angle) );
    attribute1->setBufferIndex( 0 );
    vertex_descriptor->attributes()->setObject( attribute1, 1 );

    MTL::VertexBufferLayoutDescriptor* layout0 = MTL:: VertexBufferLayoutDescriptor::alloc()->init();
    layout0->setStride( sizeof(struct VertexInDiscInst) );
    layout0->setStepFunction( MTL::VertexStepFunctionPerVertex );
    layout0->setStepRate( 1 );
    vertex_descriptor->layouts()->setObject( layout0, 0 );
    pipeline_descriptor->setVertexDescriptor( vertex_descriptor );

    auto* color_attachment0 = MTL::RenderPipelineColorAttachmentDescriptor::alloc()->init();
    color_attachment0->setPixelFormat( pixelFormat );
    color_attachment0->setBlendingEnabled( true );
    color_attachment0->setRgbBlendOperation( MTL::BlendOperationAdd );
    color_attachment0->setSourceRGBBlendFactor( MTL::BlendFactorSourceAlpha );
    color_attachment0->setDestinationRGBBlendFactor( MTL::BlendFactorOneMinusSourceAlpha );

    pipeline_descriptor->colorAttachments()->setObject( color_attachment0, 0 );
    pipeline_descriptor->setDepthAttachmentPixelFormat( MTL::PixelFormatDepth32Float );
    pipeline_descriptor->setSampleCount( 1 );
    NS::Error* error;
    m_pipeline_state_disc_inst = m_device->newRenderPipelineState( pipeline_descriptor, &error );
    if (error != nullptr) {
        NS::String* error_str = error->localizedDescription();
        std::cerr << "NSError: " << error_str->cString(NS::UTF8StringEncoding) << "\n";
    }

    color_attachment0->release();
    layout0->release();
    attribute1->release();
    attribute0->release();
    vertex_descriptor->release();
    pipeline_descriptor->release();
    library->release();
}

void MetalRenderer::createPipelineStatesPositionNormalColor( const MTL::PixelFormat pixelFormat )
{
    MTL::Library* library = m_device->newDefaultLibrary();
    MTL::RenderPipelineDescriptor* pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();

    pipeline_descriptor->setVertexFunction  ( library->newFunction( NS::String::string( (const char*)"vertex_position_normal_color", NS::UTF8StringEncoding) ) );
    pipeline_descriptor->setFragmentFunction( library->newFunction( NS::String::string( (const char*)"fragment_common",  NS::UTF8StringEncoding) ) );

    MTL::VertexDescriptor* vertex_descriptor = MTL::VertexDescriptor::alloc()->init();

    MTL::VertexAttributeDescriptor* attribute0 = MTL::VertexAttributeDescriptor::alloc()->init();
    attribute0->setFormat( MTL::VertexFormatFloat4 );
    attribute0->setOffset( offsetof(VertexInPositionNormalColor, position) );
    attribute0->setBufferIndex( 0 );
    vertex_descriptor->attributes()->setObject( attribute0, 0 );

    MTL::VertexAttributeDescriptor* attribute1 = MTL::VertexAttributeDescriptor::alloc()->init();
    attribute1->setFormat( MTL::VertexFormatFloat4 );
    attribute1->setOffset( offsetof(VertexInPositionNormalColor, normal) );
    attribute1->setBufferIndex( 0 );
    vertex_descriptor->attributes()->setObject( attribute1, 1 );

    MTL::VertexAttributeDescriptor* attribute2 = MTL::VertexAttributeDescriptor::alloc()->init();
    attribute2->setFormat( MTL::VertexFormatFloat4 );
    attribute2->setOffset( offsetof(VertexInPositionNormalColor, color) );
    attribute2->setBufferIndex( 0 );
    vertex_descriptor->attributes()->setObject( attribute2, 2 );

    MTL::VertexBufferLayoutDescriptor* layout0 = MTL:: VertexBufferLayoutDescriptor::alloc()->init();
    layout0->setStride( sizeof(struct VertexInPositionNormalColor) );
    layout0->setStepFunction( MTL::VertexStepFunctionPerVertex );
    layout0->setStepRate( 1 );
    vertex_descriptor->layouts()->setObject( layout0, 0 );
    pipeline_descriptor->setVertexDescriptor( vertex_descriptor );

    auto* color_attachment0 = MTL::RenderPipelineColorAttachmentDescriptor::alloc()->init();
    color_attachment0->setPixelFormat( pixelFormat );
    color_attachment0->setBlendingEnabled( true );
    color_attachment0->setRgbBlendOperation( MTL::BlendOperationAdd );
    color_attachment0->setSourceRGBBlendFactor( MTL::BlendFactorSourceAlpha );
    color_attachment0->setDestinationRGBBlendFactor( MTL::BlendFactorOneMinusSourceAlpha );

    pipeline_descriptor->colorAttachments()->setObject( color_attachment0, 0 );
    pipeline_descriptor->setDepthAttachmentPixelFormat( MTL::PixelFormatDepth32Float );
    pipeline_descriptor->setSampleCount( 1 );
    NS::Error* error;
    m_pipeline_state_position_normal_color = m_device->newRenderPipelineState( pipeline_descriptor, &error );
    if (error != nullptr) {
        NS::String* error_str = error->localizedDescription();
        std::cerr << "NSError: " << error_str->cString(NS::UTF8StringEncoding) << "\n";
    }

    color_attachment0->release();
    layout0->release();
    attribute2->release();
    attribute1->release();
    attribute0->release();
    vertex_descriptor->release();
    pipeline_descriptor->release();
    library->release();
}

void MetalRenderer::createDepthStecilState()
{
    auto* descriptor = MTL::DepthStencilDescriptor::alloc()->init();
//    descriptor->setDepthCompareFunction( MTL::CompareFunctionAlways );
//    descriptor->setDepthWriteEnabled( false );
    descriptor->setDepthCompareFunction( MTL::CompareFunctionLess );
    descriptor->setDepthWriteEnabled( true );
    m_depth_stencil_state = m_device->newDepthStencilState( descriptor );
    descriptor->release();
}

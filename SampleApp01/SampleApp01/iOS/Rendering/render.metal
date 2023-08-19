#include <metal_stdlib>
using namespace metal;

struct VertexInPositionNormalColor {
    float4 position [[ attribute( 0 ) ]];
    float4 normal   [[ attribute( 1 ) ]];
    float4 color    [[ attribute( 2 ) ]];
};

struct VertexInDiscInst {
    float unit_radius [[ attribute( 0 ) ]];
    float angle       [[ attribute( 1 ) ]];
};

struct DiscInstance {
    float4 com;
    float4 color;
    float  radius;
};

struct VertexOut {

    float4 position_proj [[ position ]];
    float4 position_gcs;
    float4 normal_gcs;
    float4 color;
};

vertex VertexOut vertex_disc_inst(

    const        VertexInDiscInst vertex_in  [[ stage_in ]],
    device const float4x4&        M_model    [[ buffer( 1 ) ]],
    device const float4x4&        M_view     [[ buffer( 2 ) ]],
    device const float4x4&        M_proj     [[ buffer( 3 ) ]],
    device const DiscInstance*    inst_array [[ buffer( 4 ) ]],
    const        uint32_t         inst_id    [[ instance_id ]]
) {
    device const auto& inst = inst_array[ inst_id ];

    const float4 position_raw{
        vertex_in.unit_radius * inst.radius * cos( vertex_in.angle ),
        vertex_in.unit_radius * inst.radius * sin( vertex_in.angle ),
        0.0,
        1.0
    };

    const float4 normal_raw{ 0.0f, 0.0f, 1.0f, 0.0f };
    
    const auto position_gcs = M_model * ( position_raw + inst.com );
    const auto normal_gcs   = M_model * normal_raw;

    const auto position_proj = M_proj * M_view * position_gcs;

    VertexOut out {
        .position_proj = position_proj,
        .position_gcs  = position_gcs,
        .normal_gcs    = normal_gcs,
        .color         = inst.color
    };

    return out;
}

vertex VertexOut vertex_position_normal_color(

    const        VertexInPositionNormalColor
                           vertex_in [[ stage_in ]],
    device const float4x4& M_model   [[ buffer( 1 ) ]],
    device const float4x4& M_view    [[ buffer( 2 ) ]],
    device const float4x4& M_proj    [[ buffer( 3 ) ]]
) {
    const auto position_gcs = M_model * vertex_in.position;
    const auto normal_gcs   = M_model * vertex_in.normal;

    const auto position_proj = M_proj * M_view * position_gcs;

    VertexOut out {
        .position_proj = position_proj,
        .position_gcs  = position_gcs,
        .normal_gcs    = normal_gcs,
        .color         = vertex_in.color
    };

    return out;
}

fragment float4 fragment_common(
    VertexOut          in       [[ stage_in ]],
    constant float4x4& M_camera [[ buffer( 1 ) ]]
) {
    const float3 camera_gcs{ M_camera[3][0],  M_camera[3][1],  M_camera[3][2] };
    const float3 material_color = in.color.xyz;
    const float  specular_intensity{0.5f};

    const float  opacity{ 1.0f - in.color.w };

    const float3 light_direction{ 0.0f, -1.0f, 0.0f };

    const auto dot_normal_light = dot( in.normal_gcs.xyz, light_direction * -1.0f );

    // diffuse
    const auto diffuse_coeff = clamp( dot_normal_light, 0.0f, 1.0f );
    const auto diffuse_color = material_color * diffuse_coeff * 0.7f;

    // ambient
    const auto ambient_color = material_color * 0.3f;

    // specular
    float3 specular_color{ 0.0f, 0.0f, 0.0f };

    const auto dir_vertex_to_camera = normalize( in.position_gcs.xyz - camera_gcs );
    const auto normal_reflected = reflect( light_direction, in.normal_gcs.xyz );
    const auto cos_alpha = clamp( -1.0f * dot( normal_reflected, dir_vertex_to_camera ), 0.0f, 1.0f );
    specular_color = material_color * cos_alpha * specular_intensity;

    const auto color = saturate( diffuse_color + ambient_color + specular_color );
    return float4( color, 1.0f - opacity );
}

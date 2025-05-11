#pragma shader_stage(vertex)

struct VSInput {
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 tex      : TEXCOORD0;
};

struct VSOutput {
    float4 position     : SV_POSITION;
    float2 outTex       : TEXCOORD0;
    float3 outPosition  : TEXCOORD1;
};

cbuffer Ubo : register(b1) {
    float4x4 projection;
    float4x4 view;
    float4x4 transform;
};

Texture2D<float4> images[] : register(t0, space0);

VSOutput main(VSInput input) {
    VSOutput output;

    float4 worldPos = mul(float4(input.position, 1.0), transform);
    worldPos = mul(worldPos, view);
    worldPos = mul(worldPos, projection);

    output.position = worldPos;
    output.outTex = input.tex;
    output.outPosition = input.position;

    return output;
}

#include "Common.hlsli"

struct VSToPS
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

VSToPS main(VertexShaderInput input)
{
    VSToPS output;
    output.pos = float4(input.posModel, 1.0f);
    output.texcoord = input.texcoord;
    return output;
}
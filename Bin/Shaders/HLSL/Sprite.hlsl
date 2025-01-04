cbuffer g_camera : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
};

cbuffer g_object : register(b1)
{
    float4x4 worldMatrix;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

PSInput VS(VSInput input)
{
    PSInput output;

    output.position =
    mul(mul(mul(
        float4(input.position, 1.0f),
        worldMatrix),
        viewMatrix),
        projMatrix);
    
    output.texcoord = input.texcoord;

    return output;
}

float4 PS(PSInput input) : SV_Target
{
    return g_texture.Sample(g_sampler, input.texcoord);
}

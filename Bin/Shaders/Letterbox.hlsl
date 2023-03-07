Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

PSInput VS(float4 position : POSITION, float2 texcoord : TEXCOORD)
{
    PSInput result;

    result.position = position;
    result.texcoord = texcoord;

    return result;
}

float4 PS(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.texcoord);
}

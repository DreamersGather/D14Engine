Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct VSInput
{
    float4 position : POSITION;
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

    output.position = input.position;
    output.texcoord = input.texcoord;

    return output;
}

float4 PS(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.texcoord);
}

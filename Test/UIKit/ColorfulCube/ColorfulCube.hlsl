cbuffer g_camera : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
};

cbuffer g_object : register(b1)
{
    float4x4 worldMatrix;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
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
    
    output.color = input.color;

    return output;
}

float4 PS(PSInput input) : SV_Target
{
    return input.color;
}

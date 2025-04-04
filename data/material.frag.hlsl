struct Input
{
    float4 color : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

Texture2D<float4> r_texture : register(t0, space2);
SamplerState r_sampler : register(s0, space2);

float4 main(Input input) : SV_Target0
{
    return r_texture.Sample(r_sampler, input.uv) * input.color;
}

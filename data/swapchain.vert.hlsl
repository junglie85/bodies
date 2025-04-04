struct Output
{
    float2 uv : TEXCOORD0;
    float4 position : SV_Position;
};

Output main(uint vertex_index : SV_VertexID)
{
    Output output;
    output.uv = float2(float((vertex_index << 1) & 2), float(vertex_index & 2));
    output.position = float4((output.uv * float2(2.0f, -2.0f)) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

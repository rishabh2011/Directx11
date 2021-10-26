struct VertexIN
{
    float3 PosL : POSITION;
};

struct VertexOUT
{
    float4 PosH : SV_POSITION;
};

VertexOUT vertexShader(VertexIN vin)
{
    VertexOUT vout;

    vout.PosH = float4(vin.PosL, 1.0);
    return vout;
}

float4 pixelShader(VertexOUT vout) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
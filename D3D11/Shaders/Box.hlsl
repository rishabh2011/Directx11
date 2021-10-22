struct VertexIN
{
    float3 PosL : POSITION;
    float4 color : COLOR;
};

struct VertexOUT
{
    float4 PosH : SV_POSITION;
    float4 color : COLOR;
};

VertexOUT vertexShader(VertexIN vin)
{
    VertexOUT vout;

    vout.PosH = float4(vin.PosL, 1.0);
    vout.color = vin.color;
    return vout;
}

float4 pixelShader(VertexOUT vout) : SV_TARGET
{
    return vout.color;
}
cbuffer cbperobject
{
    float4x4 gWorldViewProj;    
}

struct VertexIN
{
    float3 iPosL : POSITION;
    float4 iColor : COLOR;
};

struct VertexOUT
{
    float4 oPosH : SV_POSITION;
    float4 oColor : COLOR;
};

VertexOUT vertexShader(VertexIN vin)
{
    VertexOUT vout;
    vout.oPosH = mul(float4(vin.iPosL, 1.0), gWorldViewProj);
    vout.oColor = vin.iColor;

    return vout;
}

float4 pixelShader(VertexOUT vout) : SV_TARGET
{
    return vout.oColor;        
}


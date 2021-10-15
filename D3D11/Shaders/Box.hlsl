struct Material
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specColor;
};

//Lighting structures
struct DirectLight
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specColor;
    float3 lightDir;
    float pad;
};

struct PointLight
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specColor;

    float3 position;
    float range;

    float3 att;
    float pad;
};

struct SpotLight
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specColor;

    float3 position;
    float range;

    float3 att;
    float pad;

    float3 lightDir;
    float spotPower;
};

//Lighting functions
float4 calculateDirLight(DirectLight l, Material m, float3 normal, float3 viewDirW, float4 diffTexColor);
float4 calculatePointLight(PointLight l, Material m, float3 normal, float3 viewDirW, float4 diffTexColor);
float4 calculateSpotLight(SpotLight l, Material m, float3 normal, float3 viewDirW, float4 diffTexColor);

cbuffer cbperobject
{
	float4x4 gWorldViewProj;
    float4x4 gWorldInvTranspose;
    float4x4 gWorld;
    float4x4 gTexTransform;
    Material material;
    int useTexture;
    int clipAlpha;
}

cbuffer cbperframe
{
    DirectLight dirLight;
	PointLight pointLight;
	SpotLight spotLight;
    float4 viewPosW;
}

struct VertexIN
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 texCoords : TEXCOORD;
};

struct VertexOUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;    
    float2 texCoords : TEXCOORD;
};

Texture2D diffuseMap : register(t0);
Texture2D alphaMap : register(t1);
SamplerState sam : register(s0);

VertexOUT vertexShader(VertexIN vin)
{
    VertexOUT vout;
    
    vout.PosH = mul(float4(vin.PosL, 1.0), gWorldViewProj);
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.PosW = mul(float4(vin.PosL, 1.0), gWorld).xyz;
    vout.texCoords = mul(float4(vin.texCoords, 0.0, 1.0), gTexTransform).xy;
        
    return vout;
}

float4 pixelShader(VertexOUT vout) : SV_TARGET
{
    float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
    float4 diffTexColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float4 alphaTexColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float4 texColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    [flatten]
    if (useTexture)
    {
        diffTexColor = diffuseMap.Sample(sam, vout.texCoords);        
    }
    
    [flatten]
    if (clipAlpha)
    {
        clip(diffTexColor.a - 0.1f);
    }
    
    texColor = diffTexColor; 
    
    color += calculateDirLight(dirLight, material, normalize(vout.NormalW), vout.PosW, texColor);
    //color += calculatePointLight(pointLight, material, normalize(vout.NormalW), vout.PosW, texColor);
    //color += calculateSpotLight(spotLight, material, normalize(vout.NormalW), vout.PosW, texColor);
    
    color.a = texColor.a * material.diffuseColor.a;
    return color;
}

//--------------------------------------------------------------------------------------------------------
float4 calculateDirLight(DirectLight l, Material m, float3 normal, float3 vertexPosW, float4 diffTexColor)
{
    float3 lightDir = normalize(l.lightDir);
    float3 viewDirW = normalize(viewPosW.xyz - vertexPosW);
    
	//Ambient
    float4 ambient = l.ambientColor * m.ambientColor;
	
	//Diffuse
    float diffuseFactor = max(dot(lightDir, normal), 0);
    float4 diffuse =  diffuseFactor * l.diffuseColor * m.diffuseColor;
	
	//Specular
    float4 specular = { 0.0f, 0.0f, 0.0f, 0.0f };
    [flatten]
    if(diffuseFactor > 0.0f)
    {
        float3 reflectDir = reflect(-lightDir, normal);
        float specFactor = pow(max(dot(reflectDir, viewDirW), 0), m.specColor.w);
        specular = specFactor * l.specColor * m.specColor;
    }
	
    return diffTexColor * (ambient + diffuse) + specular;
}

//---------------------------------------------------------------------------------------------------------
float4 calculatePointLight(PointLight l, Material m, float3 normal, float3 vertexPosW, float4 diffTexColor)
{
    float3 lightDir = l.position - vertexPosW;
    float d = length(lightDir);
    
    if (d > l.range)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    lightDir /= d; //normalize
    float3 viewDirW = normalize(viewPosW.xyz - vertexPosW);
    
	//Ambient
    float4 ambient = l.ambientColor * m.ambientColor;
	
	//Diffuse
    float diffuseFactor = max(dot(lightDir, normal), 0);
    float4 diffuse = diffuseFactor * l.diffuseColor * m.diffuseColor;
	
	//Specular
    float4 specular = { 0.0f, 0.0f, 0.0f, 0.0f };
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 reflectDir = reflect(-lightDir, normal);
        float specFactor = pow(max(dot(reflectDir, viewDirW), 0), m.specColor.w);
        specular = specFactor * l.specColor * m.specColor;
    }
        
    float intensity = 1.0f / dot(l.att, float3(1.0f, d, d * d));
    diffuse *= intensity;
    specular *= intensity;    
	
    return diffTexColor * (ambient + diffuse) + specular;
}

//-------------------------------------------------------------------------------------------------------
float4 calculateSpotLight(SpotLight l, Material m, float3 normal, float3 vertexPosW, float4 diffTexColor)
{
    float3 lightDir = l.position - vertexPosW;
    float d = length(lightDir);
    
    if (d > l.range)
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    lightDir /= d; //normalize
    float3 viewDirW = normalize(viewPosW.xyz - vertexPosW);
    
	//Ambient
    float4 ambient = l.ambientColor * m.ambientColor;
	
	//Diffuse
    float diffuseFactor = max(dot(lightDir, normal), 0);
    float4 diffuse = diffuseFactor * l.diffuseColor * m.diffuseColor;
	
	//Specular
    float4 specular = { 0.0f, 0.0f, 0.0f, 0.0f };
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 reflectDir = reflect(-lightDir, normal);
        float specFactor = pow(max(dot(reflectDir, viewDirW), 0), m.specColor.w);
        specular = specFactor * l.specColor * m.specColor;
    }
        
    float intensity = 1.0f / dot(l.att, float3(1.0f, d, d * d));
    diffuse *= intensity;
    specular *= intensity;
    
    float spotFactor = pow(max(dot(normalize(l.lightDir), -lightDir), 0), l.spotPower);
    return spotFactor * (diffTexColor * (ambient + diffuse) + specular);
}
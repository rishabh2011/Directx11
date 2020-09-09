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
float4 calculateDirLight(DirectLight l, Material m, float3 normal, float3 viewDirW);
float4 calculatePointLight(PointLight l, Material m, float3 normal, float3 viewDirW);
float4 calculateSpotLight(SpotLight l, Material m, float3 normal, float3 viewDirW);

cbuffer cbperobject
{
	float4x4 gWorldViewProj;
    float4x4 gWorldInvTranspose;
    float4x4 gWorld;
    Material material;
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
};

struct VertexOUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;    
};

VertexOUT vertexShader(VertexIN vin)
{
    VertexOUT vout;
    vout.PosH = mul(float4(vin.PosL, 1.0), gWorldViewProj);
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
    vout.PosW = mul(float4(vin.PosL, 1.0), gWorld).xyz;
        
    return vout;
}

float4 pixelShader(VertexOUT vout) : SV_TARGET
{
    float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
    color += calculateDirLight(dirLight, material, normalize(vout.NormalW), vout.PosW);
    //color += calculatePointLight(pointLight, material, normalize(vout.NormalW), vout.PosW);
    //color += calculateSpotLight(spotLight, material, normalize(vout.NormalW), vout.PosW);
    return color;
}

//-----------------------------------------------------------------------------------
float4 calculateDirLight(DirectLight l, Material m, float3 normal, float3 vertexPosW)
{
    float3 lightDir = normalize(l.lightDir);
    float3 viewDirW = normalize(viewPosW.xyz - vertexPosW);
    
	//Ambient
    float4 ambient = l.ambientColor * m.ambientColor;
	
	//Diffuse
    float diffuseFactor = max(dot(lightDir, normal), 0);
    
    [flatten]
    if(diffuseFactor <= 0.0f) { diffuseFactor = 0.4f; }
    else if(diffuseFactor > 0.0f && diffuseFactor <= 0.5f) { diffuseFactor = 0.6f; }
    else { diffuseFactor = 1.0f; }
    
    float4 diffuse =  diffuseFactor * l.diffuseColor * m.diffuseColor;
	
	//Specular
    float4 specular = { 0.0f, 0.0f, 0.0f, 0.0f };
    [flatten]
    if(diffuseFactor > 0.0f)
    {
        float3 reflectDir = reflect(-lightDir, normal);
        
        float specFactor = pow(max(dot(reflectDir, viewDirW), 0), m.specColor.w);
        if (specFactor <= 0.1f && specFactor >= 0.0f) { specFactor = 0.0f; }
        else if (specFactor > 0.1f && specFactor <= 0.8f) { specFactor = 0.5f; }
        else if (specFactor > 0.8f && specFactor <= 1.0f) { specFactor = 0.8f; }
        
        specular = specFactor * l.specColor * m.specColor;
    }
	
    return ambient + diffuse + specular;
}

//------------------------------------------------------------------------------------
float4 calculatePointLight(PointLight l, Material m, float3 normal, float3 vertexPosW)
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
	
    return ambient + diffuse + specular;
}

//----------------------------------------------------------------------------------
float4 calculateSpotLight(SpotLight l, Material m, float3 normal, float3 vertexPosW)
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
	return spotFactor * (ambient + diffuse + specular);
}
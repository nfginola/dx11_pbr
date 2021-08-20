struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
    float3 worldPos : WORLDPOS;
};

struct SB_PointLight
{
    float4 position;
    float4 color;
};


Texture2D albedoTex : register(t0);
Texture2D metallicAndRoughnessTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D aoTex : register(t3);

StructuredBuffer<SB_PointLight> pointLightList : register(t7);

SamplerState mainSampler : register(s0);

cbuffer CB_PerFrame : register(b0)      // To retrive the camera position
{
    matrix view;         
    matrix projection;
    float4 cameraPosition;
    
    // Temp
    float3 g_color;
    float g_metallic;
    float g_roughness;
    float g_ao;
}


// Hardcoded PBR stuff below
const static float PI = 3.1415f;
//const static float3 albedoInput = float3(1.f, 0.f, 0.f);
//const static float metallicInput = 0.5f;
//const static float roughnessInput = 0.5f;
//const static float aoInput = 0.f;


float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 fresnelSchlick(float cosTheta, float3 F0);






float4 PSMain(PS_IN input) : SV_TARGET
{
    //float3 albedoInput = g_color;
    //float metallicInput = g_metallic;
    //float roughnessInput = g_roughness;
    //float aoInput = g_ao;
    
    float3 albedoInput = albedoTex.Sample(mainSampler, input.uv).xyz;
    float metallicInput = metallicAndRoughnessTex.Sample(mainSampler, input.uv).b;
    float roughnessInput = metallicAndRoughnessTex.Sample(mainSampler, input.uv).g;
    float aoInput = aoTex.Sample(mainSampler, input.uv).r;
    
    //return float4(albedo, 1.f);
    
    // Grab relevant data
    float3 normal = normalize(input.normal);
    float3 worldPos = input.worldPos;
    //float3 camPos = -view._14_24_34_44.xyz;
    float3 camPos = cameraPosition.xyz;
    
    
    //return float4(normal, 1.f);
    
    //return float4(camPos, 1.f);
    
  
    float3 N = normalize(normal);
    float3 V = normalize(camPos - worldPos);

    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedoInput, metallicInput);
	           
    // reflectance equation
    float3 Lo = float3(0.f, 0.f, 0.f);
    for (int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        float3 L = normalize(pointLightList[i].position.xyz - worldPos);
        float3 H = normalize(V + L);
        float distance = length(pointLightList[i].position.xyz - worldPos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = pointLightList[i].color.xyz * attenuation;
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughnessInput);
        float G = GeometrySmith(N, V, L, roughnessInput);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        
        //return float4(G, G, G, 1.f);
        //return float4(F, 1.f);
        //return float4(NDF, NDF, NDF, 1.f);
        
        
        float3 kS = F;
        float3 kD = float3(1.f, 1.f, 1.f) - kS;
        kD *= 1.0 - metallicInput;
        
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        float3 specular = numerator / max(denominator, 0.001);
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedoInput / PI + specular) * radiance * NdotL;
        
        //return float4(Lo, 1.f);
        
    }
  
    float3 ambient = float3(0.03f, 0.03f, 0.03f) * albedoInput * aoInput;
    float3 color = ambient + Lo;
	
    // No need for tonemapping and gamma correction, we do that on the subsequent fullscreen quad pass
    //color = color / (color + vec3(1.0));
    //color = pow(color, float3(1.0 / 2.2));
   
    return float4(color, 1.0);
}




// Old PS
//float4 PSMain(PS_IN input) : SV_TARGET
//{
//    //float3 color = mainTex.Sample(mainSampler, input.uv).xyz;
//    //float3 color = diffuseTex.SampleLevel(mainSampler, input.uv, mipLevel).xyz; // Note that the sampler must an unlocked maxLOD
//    //float3 color = normalize(input.normal);
//    float3 color = diffuseTex.Sample(mainSampler, input.uv).xyz;
    
    
 
    
    
    
    
//    return float4(color, 1.f);
//}




float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
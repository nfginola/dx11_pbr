
struct PS_IN
{
    float4 pos : SV_Position;
    float3 wsPos : POSITION;
};

TextureCube skyboxTexture : register(t0);
Texture2D skyboxHDR : register(t1);
SamplerState skyboxSampler : register(s0);

static const float PI = 3.141592;
static const float TwoPI = 2 * PI;

// From LearnOpenGL
static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan(v.z / v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float4 main(PS_IN input) : SV_TARGET
{
    // HDR
    // Nadrin (equirect to cubemap)   https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/equirect2cube.hlsl
    float3 v = normalize(input.wsPos);
    float phi = atan2(v.z, v.x);
    float theta = acos(v.y);
    return float4(skyboxHDR.Sample(skyboxSampler, float2(phi / TwoPI, theta / PI)).xyz, 1.f);
    
    // if no hdr
    return skyboxTexture.Sample(skyboxSampler, input.wsPos);
}
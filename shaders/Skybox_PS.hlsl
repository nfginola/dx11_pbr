
struct PS_IN
{
    float4 pos : SV_Position;
    float3 wsPos : POSITION;
};

TextureCube skyboxTexture : register(t0);
SamplerState skyboxSampler : register(s0);

float4 main(PS_IN input) : SV_TARGET
{
    return skyboxTexture.Sample(skyboxSampler, input.wsPos);
}
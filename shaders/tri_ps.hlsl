struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

Texture2D diffuseTex : register(t0);
Texture2D specularTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D opacityTex : register(t3);

SamplerState mainSampler : register(s0);




float4 PSMain(PS_IN input) : SV_TARGET
{
    //float3 color = mainTex.Sample(mainSampler, input.uv).xyz;
    //float3 color = diffuseTex.SampleLevel(mainSampler, input.uv, mipLevel).xyz; // Note that the sampler must an unlocked maxLOD
    //float3 color = normalize(input.normal);
    float3 color = diffuseTex.Sample(mainSampler, input.uv).xyz;
        
    return float4(color, 1.f);
}
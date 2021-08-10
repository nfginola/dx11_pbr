struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

Texture2D mainTex : register(t0);
SamplerState mainSampler : register(s0);

float4 PSMain(PS_IN input) : SV_TARGET
{
    return float4(mainTex.Sample(mainSampler, input.uv).xyz, 1.f);
	return float4(input.uv, 0.0f, 1.0f);
}
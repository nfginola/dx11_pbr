struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

Texture2D mainTex : register(t0);
SamplerState mainSampler : register(s0);

static const float GAMMA = 2.2f;

float4 PSMain(PS_IN input) : SV_TARGET
{
    float3 color = mainTex.Sample(mainSampler, input.uv).xyz;
    //float3 color = mainTex.SampleLevel(mainSampler, input.uv, 3.f).xyz;		// Note that the sampler must an unlocked maxLOD
		
    return float4(pow(color, float3(1.f / GAMMA, 1.f / GAMMA, 1.f / GAMMA)), 1.f);
	//return float4(input.uv, 0.0f, 1.0f);
}
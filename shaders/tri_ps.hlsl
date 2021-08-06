struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

float4 PSMain(PS_IN input) : SV_TARGET
{
	return float4(input.uv, 0.0f, 1.0f);
}
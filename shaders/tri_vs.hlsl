struct VS_INPUT
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;

};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

VS_OUT VSMain(VS_INPUT input)
{
	VS_OUT output = (VS_OUT)0;

	output.pos = float4(input.pos, 1.f);
	output.uv = input.uv;
	output.normal = input.normal;
	return output;
}
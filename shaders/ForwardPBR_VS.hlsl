struct VS_INPUT
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
	
    float4 wm_row0 : INSTANCE_WM_ROW0;
    float4 wm_row1 : INSTANCE_WM_ROW1;
    float4 wm_row2 : INSTANCE_WM_ROW2;
    float4 wm_row3 : INSTANCE_WM_ROW3;

};

struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
    float3 worldPos : WORLDPOS;
    
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

cbuffer CB_PerFrame : register(b0)
{
    matrix view;
    matrix projection;
    float3 cameraPosition;
    
}

cbuffer CB_PerObject : register(b1)
{
    matrix model;
}

VS_OUT main(VS_INPUT input)
{
	VS_OUT output = (VS_OUT)0;
	
    matrix worldMat = matrix(input.wm_row0, input.wm_row1, input.wm_row2, input.wm_row3);
    worldMat = transpose(worldMat);

	//output.pos = float4(input.pos, 1.f);
	//output.uv = input.uv;
	//output.normal = input.normal;
	
    output.worldPos = mul(worldMat, float4(input.pos, 1.f)).xyz;
    output.pos = mul(projection, mul(view, float4(output.worldPos, 1.f)));
    //output.pos = mul(projection, mul(view, mul(float4(input.pos, 1.f), worldMat)));
    //output.normal = input.normal;
    output.normal = normalize(mul(worldMat, float4(input.normal, 0.f)).xyz);
    output.tangent = normalize(mul(worldMat, float4(input.tangent, 0.f)).xyz);
    output.bitangent = normalize(mul(worldMat, float4(input.bitangent, 0.f)).xyz);
    output.uv = input.uv;
    
    
	
	return output;
}
struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

// CW
static const float3 positions[] =
{
    // Left tri 
    { -1.f, 1.f, 0.f },
    { 1.f, -1.f, 0.f },
    { -1.f, -1.f, 0.f },
    
    // Right tri
    { -1.f, 1.f, 0.f },
    { 1.f, 1.f, 0.f },
    { 1.f, -1.f, 0.f }
};

static const float2 uvs[] =
{
    // Left tri 
    { 0.f, 0.f },
    { 1.f, 1.f },
    { 0.f, 1.f },
    
    // Right tri
    { 0.f, 0.f },
    { 1.f, 0.f },
    { 1.f, 1.f }
};


VS_OUT VSMain(uint vID : SV_VertexID)
{
    VS_OUT output = (VS_OUT) 0;
    
    output.position = float4(positions[vID], 1.f);
    output.uv = uvs[vID];
    
	return output;
}
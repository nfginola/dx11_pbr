static const float4 CUBE[36] =
{
float4(0.5f, -0.5f, -0.5f, 0.5f),
float4(-0.5f, -0.5f, -0.5f, 0.5f),
float4(-0.5f, 0.5f, -0.5f, 0.5f),

float4(-0.5f, 0.5f, -0.5f, 0.5f),
float4(0.5f, 0.5f, -0.5f, 0.5f),
float4(0.5f, -0.5f, -0.5f, 0.5f),

float4(-0.5f, -0.5f, 0.5f, 0.5f),
float4(0.5f, -0.5f, 0.5f, 0.5f),
float4(0.5f, 0.5f, 0.5f, 0.5f),

float4(0.5f, 0.5f, 0.5f, 0.5f),
float4(-0.5f, 0.5f, 0.5f, 0.5f),
float4(-0.5f, -0.5f, 0.5f, 0.5f),

float4(-0.5f, -0.5f, -0.5f, 0.5f),
float4(-0.5f, -0.5f, 0.5f, 0.5f),
float4(-0.5f, 0.5f, 0.5f, 0.5f),
    
float4(-0.5f, 0.5f, 0.5f, 0.5f),
float4(-0.5f, 0.5f, -0.5f, 0.5f),
float4(-0.5f, -0.5f, -0.5f, 0.5f),

float4(0.5f, -0.5f, 0.5f, 0.5f),
float4(0.5f, -0.5f, -0.5f, 0.5f),
float4(0.5f, 0.5f, -0.5f, 0.5f),

float4(0.5f, 0.5f, -0.5f, 0.5f),
float4(0.5f, 0.5f, 0.5f, 0.5f),
float4(0.5f, -0.5f, 0.5f, 0.5f),

float4(0.5f, 0.5f, -0.5f, 0.5f),
float4(-0.5f, 0.5f, -0.5f, 0.5f),
float4(-0.5f, 0.5f, 0.5f, 0.5f),

float4(-0.5f, 0.5f, 0.5f, 0.5f),
float4(0.5f, 0.5f, 0.5f, 0.5f),
float4(0.5f, 0.5f, -0.5f, 0.5f),

float4(0.5f, -0.5f, 0.5f, 0.5f),
float4(-0.5f, -0.5f, 0.5f, 0.5f),
float4(-0.5f, -0.5f, -0.5f, 0.5f),
    
float4(-0.5f, -0.5f, -0.5f, 0.5f),
float4(0.5f, -0.5f, -0.5f, 0.5f),
float4(0.5f, -0.5f, 0.5f, 0.5f)
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float3 wsPos : POSITION;
};

cbuffer CamViewMat : register(b0)
{
    Matrix viewProjMat;
}


VS_OUT main(uint vID : SV_VertexID)
{
    VS_OUT output = (VS_OUT) 0;
    
    output.wsPos = CUBE[vID].xyz; // Sample vector
    output.pos = mul(viewProjMat, float4(CUBE[vID].xyz, 0.f)); // Consider rotation but not translation
    output.pos.z = output.pos.w;
   
    return output;
}
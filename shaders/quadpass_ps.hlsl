struct PS_IN
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D renderTexture : register(t0);     // HDR texture
SamplerState mainSampler : register(s0);

static float GAMMA = 2.2f;
static float EXPOSURE = 0.1f;

float4 PSMain(PS_IN input) : SV_TARGET
{
    float3 hdrColor = renderTexture.Sample(mainSampler, input.uv);
    float3 ldrColor;
    
    // Reinhard tonemapping (HDR to LDR mapping)
    ldrColor = hdrColor / (hdrColor + float3(1.f, 1.f, 1.f));
    
    // Compare tonemapped (right) vs non-tonemapped (left)
    //if (input.uv.x >= 0.5f)
    //    ldrColor = hdrColor / (hdrColor + float3(1.f, 1.f, 1.f));
    //else
    //    ldrColor = hdrColor;
    
    // Exposure based
    //float3 ldrColor = float3(1.f, 1.f, 1.f) - exp(-hdrColor * EXPOSURE);
    
    // Dont do any tonemapping
    //ldrColor = hdrColor;
    
    // Gamma correction
    ldrColor = pow(ldrColor, float3(1.f / GAMMA, 1.f / GAMMA, 1.f / GAMMA));
    
    return float4(ldrColor, 1.f);
}
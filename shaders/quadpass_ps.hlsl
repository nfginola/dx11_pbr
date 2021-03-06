struct PS_IN
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D renderTexture : register(t0);     // HDR texture
SamplerState mainSampler : register(s0);

static float GAMMA = 2.2f;
static float EXPOSURE = 0.1f;


static const float3 aces_input_matrix[] =
{
    float3(0.59719f, 0.35458f, 0.04823f),
    float3(0.07600f, 0.90834f, 0.01566f),
    float3(0.02840f, 0.13383f, 0.83777f)
};

static const float3 aces_output_matrix[] =
{
    float3(1.60475f, -0.53108f, -0.07367f),
    float3(-0.10208f, 1.10813f, -0.00605f),
    float3(-0.00327f, -0.07276f, 1.07602f)
};

float3 mul(float3 v)
{
    float x = aces_input_matrix[0][0] * v[0] + aces_input_matrix[0][1] * v[1] + aces_input_matrix[0][2] * v[2];
    float y = aces_input_matrix[1][0] * v[1] + aces_input_matrix[1][1] * v[1] + aces_input_matrix[1][2] * v[2];
    float z = aces_input_matrix[2][0] * v[1] + aces_input_matrix[2][1] * v[1] + aces_input_matrix[2][2] * v[2];
    return float3(x, y, z);
}

float3 mul2(float3 v)
{
    float x = aces_output_matrix[0][0] * v[0] + aces_output_matrix[0][1] * v[1] + aces_output_matrix[0][2] * v[2];
    float y = aces_output_matrix[1][0] * v[1] + aces_output_matrix[1][1] * v[1] + aces_output_matrix[1][2] * v[2];
    float z = aces_output_matrix[2][0] * v[1] + aces_output_matrix[2][1] * v[1] + aces_output_matrix[2][2] * v[2];
    return float3(x, y, z);
}

float3 rtt_and_odt_fit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 aces_fitted(float3 v)
{
    v = mul(v);
    v = rtt_and_odt_fit(v);
    return mul2(v);
}

float3 reinhard(float3 v)
{
    return v / (v + float3(1.f, 1.f, 1.f));
}

// https://64.github.io/tonemapping/

float luminance(float3 color)
{
    return dot(color, float3(0.2126f, 0.7152f, 0.0722f));
}

float3 reinhard_jodie(float3 v)
{
    float l = luminance(v);
    float3 tv = v / (1.0f + v);
    return lerp(v / (1.0f + l), tv, tv);
}


float4 main(PS_IN input) : SV_TARGET
{
    float3 hdrColor = renderTexture.Sample(mainSampler, input.uv).xyz;
    float3 ldrColor;
    
    //return float4(hdrColor, 1.f);
    
    // Reinhard tonemapping (HDR to LDR mapping)
    //ldrColor = hdrColor / (hdrColor + float3(1.f, 1.f, 1.f));
    
    // Compare tonemapped (right) vs non-tonemapped (left)
    //if (input.uv.x >= 0.75f)
    //    ldrColor = reinhard(hdrColor);
    //else if (input.uv.x >= 0.50f)
    //    ldrColor = aces_fitted(hdrColor);
    //else if (input.uv.x >= 0.25f)
    //    ldrColor = reinhard_jodie(hdrColor);
    //else
    //    ldrColor = hdrColor;
    
    //ldrColor = reinhard_jodie(hdrColor);
   
    ldrColor = reinhard_jodie(hdrColor);
    
    //ldrColor = aces_fitted(hdrColor);
    
    // Exposure based
    //float3 ldrColor = float3(1.f, 1.f, 1.f) - exp(-hdrColor * EXPOSURE);
    
    // Dont do any tonemapping
    //ldrColor = hdrColor;
    
    // Gamma correction
    //ldrColor = pow(ldrColor, float3(1.f / GAMMA, 1.f / GAMMA, 1.f / GAMMA));
    ldrColor = pow(ldrColor, (1.f / GAMMA).xxx);

    
    return float4(ldrColor, 1.f);
}
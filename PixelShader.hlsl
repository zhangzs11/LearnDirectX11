cbuffer LightBuffer : register(b1)
{
    float4 lightPosition;
    float4 lightColor;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float3 lightDir = normalize(lightPosition.xyz - input.worldPos.xyz);
    float diffuseIntensity = max(0.1f, dot(input.normal, lightDir));
    
    float4 textureColor = shaderTexture.Sample(SampleType, input.uv);
    
    return textureColor * diffuseIntensity;
    //return textureColor;
    //return float4(1.0f, 0.0f, 0.0f, 1.0f);


}
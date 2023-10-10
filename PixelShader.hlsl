cbuffer LightBuffer
{
    float4 lightPosition;
    float4 lightColor;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : TEXCOORD0;
};
float4 main(PixelInputType input) : SV_TARGET
{
    float3 lightDir = normalize(lightPosition.xyz - input.worldPos.xyz);
    float diffuseIntensity = max(0.1f, dot(input.normal, lightDir));
    
    return float4(0.0f, 1.0f, 1.0f, 1.0f) * diffuseIntensity;
}
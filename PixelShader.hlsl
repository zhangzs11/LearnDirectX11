cbuffer LightBuffer : register(b1)
{
    float4 lightPosition;
    float4 lightColor;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

Texture2D normalMap : register(t1);
SamplerState NormalMapSample : register(s1);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : TEXCOORD0;
    float2 uv : TEXCOORD1;
    matrix tbn : TEXCOORD2; //3*3 matrix stored in a 3*4 format
};

float4 main(PixelInputType input) : SV_TARGET
{
    //Sample the normal map
    float3 normalFromMap = normalMap.Sample(NormalMapSample, input.uv).xyz;
    //Transform the normal from [0,1] range to [-1,1] range
    normalFromMap = 2.0f * normalFromMap - 1.0f;
    //Use the TBN matrix to transform the sampled normal to world space
    float3 worldNormal = normalize(mul(normalFromMap, (float3x3) input.tbn));
    
    float3 lightDir = normalize(lightPosition.xyz - input.worldPos.xyz);
    float diffuseIntensity = max(0.1f, dot(worldNormal, lightDir));
    
    float4 textureColor = shaderTexture.Sample(SampleType, input.uv);
    
    return textureColor * diffuseIntensity;
    //return textureColor;
    //return float4(1.0f, 0.0f, 0.0f, 1.0f);


}
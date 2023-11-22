cbuffer LightBuffer : register(b1)
{
    float4 lightPosition;
    float4 lightColor;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

Texture2D normalMap : register(t1);
SamplerState NormalMapSample : register(s1);

TextureCube shadowMapCube : register(t2);
SamplerState ShadowMapSample : register(s2);

struct MainScenePixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : TEXCOORD0;
    float2 uv : TEXCOORD1;
    matrix tbn : TEXCOORD3;
    float3 toLight : TEXCOORD2;
};
float4 main(MainScenePixelInputType input) : SV_TARGET
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
    
    float shadowDepth = shadowMapCube.Sample(ShadowMapSample, -input.toLight).r;
    // The negative sign (-input.toLight) is to fetch the depth in the direction of the light from the point in the scene.
    float near = 0.1f;
    float far = 100.0f;
    float depthView = (near * far) / (far - shadowDepth * (far - near));
    
    float currentDepth = length(input.worldPos.xyz - lightPosition.xyz);
    // Calculate distance from the current fragment to the light source

    float shadow = currentDepth > depthView ? 0.5 : 1.0;
    
    //return textureColor * diffuseIntensity * shadow;
    return textureColor * shadow;
    //return float4(1, 0, 0, 1)*shadow;
    //float shadowDepth = shadowMapCube.Sample(ShadowMapSample, -input.toLight).r;
    //return float4(shadowDepth, shadowDepth, shadowDepth, 1);
    //return float4(currentDepth/3, currentDepth/3, currentDepth/3, 1.0f);
    //return float4(shadow, shadow, shadow, 1.0f);
    //return float4(depthView/10, 0, 0, 1.0f);
}
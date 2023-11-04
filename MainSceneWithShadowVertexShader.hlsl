cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
cbuffer LightBuffer : register(b1)
{
    float4 lightPosition;
    float4 lightColor;
};
struct VertexInputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 worldPos : TEXCOORD0;
    float2 uv : TEXCOORD1;
    matrix tbn : TEXCOORD3;
    float3 toLight : TEXCOORD2;
};
PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.normal = input.normal;
    output.worldPos = input.position;
    output.uv = input.texCoord;
    
    //Compute TBN matrix
    float3 T = normalize(mul(input.tangent, (float3x3) worldMatrix));
    float3 B = normalize(mul(input.bitangent, (float3x3) worldMatrix));
    float3 N = normalize(mul(input.normal, (float3x3) worldMatrix));
    
    output.tbn[0] = float4(T, 0.0f);
    output.tbn[1] = float4(B, 0.0f);
    output.tbn[2] = float4(N, 0.0f);
    
    output.toLight = lightPosition.xyz - output.worldPos.xyz;
    
    
    return output;
}

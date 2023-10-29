struct PixelInputType
{
    float4 position : SV_POSITION;
    float depth : DEPTH;
};

float4 main(PixelInputType input) : SV_TARGET
{
    return float4(input.depth, input.depth, input.depth, 1.0f);
}

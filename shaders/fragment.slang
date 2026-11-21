struct FragmentInput {
    float4 position : SV_Position;
    float3 color : COLOR0;
};

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target {
    return float4(input.color, 1.0);
}
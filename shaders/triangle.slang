struct VertexOutput {
    float4 position : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VertexOutput vertexMain(uint vertexIndex : SV_VulkanVertexID) {
    float2 positions[3] = {
        float2( 0.0, -0.5),
        float2(-0.5,  0.5),
        float2( 0.5,  0.5),
    };

    VertexOutput output;

    float3 colors[3] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    }; 

    output.position = float4(positions[vertexIndex], 0.0, 1.0);
    output.color = colors[vertexIndex];

    return output;
}
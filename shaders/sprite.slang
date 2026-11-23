// sprite.slang

struct vIn {
    [[vk::location(0)]]
    float2 position;

    [[vk::location(1)]]
    float3 color;

    [[vk::location(2)]]
    float2 texCoord;
};

struct vOut {
    float4 position : SV_Position;

    [[vk::location(0)]]
    float3 color;

    [[vk::location(1)]]
    float2 texCoord;
}
[shader("vertex")]
vOut vertexMain(vIn input) {
    vOut output;
    float2 pos = input.position;

    output.position = float4(pos.x, pos.y, 0, 1.0);
    output.color = input.color;
    output.texCoord = input.texCoord;
    return output;
};

[[vk::binding(0, 0)]]
Sampler2D spriteTexture;

[shader("fragment")]
float4 fragmentMain(vOut input) : SV_Target {
    float4 texColor = spriteTexture.Sample(input.texCoord);

    return texColor;
//    return float4(1.0, 0.0, 0.0, 1.0);
}

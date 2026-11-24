// sprite.slang

struct vIn {
    [[vk::location(0)]]
    float2 position;

    [[vk::location(1)]]
    float2 texCoord;

    [[vk::location(2)]]
    float3 instancePosition;

    [[vk::location(3)]]
    float2 instanceScale;

    [[vk::location(4)]]
    float instanceRotation;

    [[vk::location(5)]]
    float4 instanceTint;
};

struct vOut {
    float4 position : SV_Position;

    [[vk::location(0)]]
    float2 texCoord : TEXCOORD0;

    [[vk::location(1)]]
    float4 tint : COLOR0;
};

[[vk::binding(0, 0)]]
Sampler2D spriteTexture;

float2 rotate2D(float2 position, float angle) {
    float s = sin(angle);
    float c = cos(angle);

    return float2(
        c * position.x - s * position.y,
        s * position.x + c * position.y
    );
}

[shader("vertex")]
vOut vertexMain(vIn input) {
    vOut output;

    float2 scaledPosition = input.position.xy
        * input.instanceScale;

    float2 rotatedPosition = rotate2D(
        scaledPosition,
        input.instanceRotation
    );

    float3 worldPosition = float3(
        rotatedPosition + input.instancePosition.xy,
        input.instancePosition.z
    );

    output.position = float4(worldPosition, 1.0);
    output.texCoord = input.texCoord;
    output.tint = input.instanceTint;

    return output;
};


[shader("fragment")]
float4 fragmentMain(vOut input) : SV_Target {
    float4 texColor = spriteTexture.Sample(input.texCoord);

    return texColor * input.tint;
//    return float4(1.0, 0.0, 0.0, 1.0);
}

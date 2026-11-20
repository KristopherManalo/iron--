// shader.slang


struct UniformBufferObject {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
}

struct vIn {
    [[vk::location(0)]]
    float3 position;
    [[vk::location(1)]]
    float2 uv;
    [[vk::location(2)]]
    float2 spritePosition;
    [[vk::location(3)]]
    float2 spriteScale;
    [[vk::location(4)]]
    int32_t spriteIndex;
}

struct vOut {
    float4 position : SV_Position;
    [[vk::location(0)]]
    float2 texCoord;
}

struct Sprite {
    float2 uvMin;
    float2 uvMax;
}

[[vk::binding(0)]]
ConstantBuffer<UniformBufferObject> ubo;
[[vk::binding(1)]]
Texture2D textureMap;
[[vk::binding(2)]]
SamplerState sampler;
[[vk::binding(3)]]
StructuredBuffer<Sprite> sprites;

[shader("vertex")]
vOut vertexMain(vIn input) {
    vOut output;
    float3 pos = float3(input.position.xy, input.position.z);
    //pos += 0.5;
    pos.xy *= input.spriteScale;
    pos.xy += input.spritePosition.xy;
    output.position = mul(
        ubo.proj,
        mul(
            ubo.view,
            mul(
                ubo.model,
                float4(pos, 1.0)
            )
        )
    );
    /*
    output.position = mul(
        ubo.proj,
        mul(ubo.view,
            mul(ubo.model, float4(pos, 0.0, 1.0)
        ))
    );
    */
    //output.spriteIndex = input.spriteIndex;
	int column = input.spriteIndex % 2;
	int row = input.spriteIndex / 2;
    float2 offset = float2(column, row) * spriteSize;
    Sprite sprite = sprites[input.spriteIndex];
    // output.texCoord = offset + input.uv * spriteSize;
    output.texCoord = lerp(sprite.uvMin, sprite.uvMax, input.uv);
    //output.texCoord = input.uv;
    /*
    Sprite sprite = sprites[0];
    output.texCoord = sprite.uvMin;
    */
    return output;
}

static float2 spriteSize = float2(1.0 / 2.0, 1.0 / 2.0);
struct SpriteSheet {
    uint columns;
    uint rows;
};


[shader("fragment")]
float4 fragmentMain(vOut input) : SV_Target {
    float4 color = textureMap.Sample(sampler, input.texCoord);
    //return float4(input.texCoord, 0, 1);
    /*
    if (color.a < 0.1) {
        discard;
    }
    */

    return color;
}


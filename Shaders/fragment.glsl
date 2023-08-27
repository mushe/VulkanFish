#version 450

layout(binding = 1) uniform sampler2D inSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(inSampler, inTexCoord);
    if(outColor.a < 0.01) discard;
}

#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) flat in uint TextureIndex;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler[32];

void main() {	
    outColor =  fragColor * texture(texSampler[TextureIndex], TexCoord);
}
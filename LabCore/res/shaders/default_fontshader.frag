#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) flat in uint matIndex;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler[32];

struct UVCoords {
    //bottom left
    vec2 uv0;

    //top right
    vec2 uv1;
};

layout(set = 1, binding = 0) uniform UVCoordsBuffer {
	UVCoords uvcoords[96];
} uvcoordsbuffer;	

void main() {	

    UVCoords uv = uvcoordsbuffer.uvcoords[matIndex];

    vec2 mixedcoord = mix(uv.uv0, uv.uv1, TexCoord);

    outColor = vec4(fragColor.xyz, texture(texSampler[1], mixedcoord).r);
}
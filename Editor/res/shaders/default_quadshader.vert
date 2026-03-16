#version 450

//vertexbuffer attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

//instancebuffer attributes
layout(location = 2) in vec4 inRotScaleMat;
layout(location = 3) in vec3 instanceTranslation;
layout(location = 4) in vec4 instanceColor;
layout(location = 5) in uint inTextureIndex;

//outgoing attributes
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 TexCoord;
layout(location = 2) out uint TextureIndex;

layout(set = 1, binding = 0) uniform Scene {
	mat4 projection;
	mat4 view;
} scene;

void main() {

	mat2 rotscalematrix = mat2(inRotScaleMat);
    gl_Position = scene.projection * scene.view * (vec4(rotscalematrix * inPosition.xy, inPosition.z, 1.0) + vec4(instanceTranslation, 0.0));

    fragColor = instanceColor;
	TexCoord = inTexCoord;
	TextureIndex = inTextureIndex;
}
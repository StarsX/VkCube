//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 wpos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

layout (location = 0) out vec4 uFragColor[2];

//layout (set = 0, binding = 1) uniform sampler2D tex;
layout (set = 0, binding = 1) uniform sampler samp;
layout (set = 0, binding = 2) uniform texture2D tex;

void main()
{
	//uFragColor[0] = texture(tex, texcoord);
	uFragColor[0] = texture(sampler2D(tex, samp), texcoord);
	uFragColor[1] = vec4(normalize(normal) * 0.5 + 0.5, 1.0);
}

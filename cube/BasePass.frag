//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 wpos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

layout (location = 0) out vec4 fragColor[2];

layout (set = 1, binding = 0) uniform sampler samp;
layout (set = 2, binding = 0) uniform texture2D tex;

void main()
{
	fragColor[0] = texture(sampler2D(tex, samp), texcoord);
	fragColor[1] = vec4(normalize(normal) * 0.5 + 0.5, 1.0);
}

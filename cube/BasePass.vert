//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 pos;
layout (location = 1) in vec3 nrm;
layout (location = 2) in vec4 tex;

layout (location = 0) out vec3 wpos;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec2 texcoord;

layout(std140, binding = 0) uniform buf
{
	mat4 MVP;
	mat4 Model;
	mat3 Normal;
} ubuf;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	wpos = (ubuf.Model * pos).xyz;
	normal = normalize(ubuf.Normal * nrm);
	texcoord = tex.xy;

	gl_Position = ubuf.MVP * pos;
}

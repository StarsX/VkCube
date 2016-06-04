//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec2 texcoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	texcoord = 2.0 * vec2(gl_VertexIndex >> 1, gl_VertexIndex & 1);
	
	gl_Position = vec4(texcoord * 2.0 - 1.0, 0.5, 1.0);
}

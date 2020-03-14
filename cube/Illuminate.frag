//--------------------------------------------------------------------------------------
// By S. XU Tianchen
//--------------------------------------------------------------------------------------

#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 texcoord;

layout (location = 0) out vec4 uFragColor;

layout (set = 0, binding = 0) uniform buf
{
	vec3 g_lightPos;
	vec3 g_eyePos;
};

layout (set = 1, binding = 0) uniform sampler samp;
layout (set = 2, binding = 0) uniform texture2D txDiffuse;
layout (set = 2, binding = 1) uniform texture2D txNormal;

void main()
{
	const vec3 upDir = vec3(0.0, 1.0, 0.0);
	const vec4 light = vec4(5.0);
	const vec4 ambient = vec4(1.2);

	vec4 diffuse = texture(sampler2D(txDiffuse, samp), texcoord);
	vec4 norm = texture(sampler2D(txNormal, samp), texcoord);
	norm.xyz = norm.xyz * 2.0 - 1.0;

	//vec3 lightDir = normalize(g_lightPos - wpos);
	vec3 lightDir = normalize(g_lightPos);

	float lightAmt = clamp(dot(norm.xyz, lightDir), 0.0, 1.0);
	float ambientAmt = clamp(dot(norm.xyz, upDir) * 0.5 + 0.5, 0.0, 1.0);
	vec4 lightColor = light * lightAmt + ambient * ambientAmt;

	//vec3 viewDir = normalize(g_eyePos - wpos);
	vec3 viewDir = normalize(g_eyePos);
	vec3 halfAngle = normalize(lightDir + viewDir);
	float specAmt = clamp(dot(norm.xyz, halfAngle), 0.0, 1.0);
	vec4 spec = pow(specAmt, 32.0) * vec4(1.0);

	uFragColor = lightColor * diffuse + spec;
	uFragColor.w = diffuse.w;
	
	// Simple tone mapping
	uFragColor.xyz /= uFragColor.xyz + 1.0;
	uFragColor.xyz *= uFragColor.xyz;
}

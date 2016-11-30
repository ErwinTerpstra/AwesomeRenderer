#version 330

layout(location = 0) out vec4 outColor;

uniform sampler2D diffuseMap;

in vec3 normal;
in vec2 texcoord;

void main() 
{
	//outColor = texture(diffuseMap, texcoord);
	outColor = vec4(normal, 1.0);
}
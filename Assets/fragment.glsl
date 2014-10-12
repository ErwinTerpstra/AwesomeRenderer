#version 330

layout(location = 0) out vec4 outColor;

uniform sampler2D diffuseMap;

varying vec3 normal;
varying vec2 texcoord;

void main() 
{
	outColor = texture(diffuseMap, texcoord);
}
#version 330

layout(location = 0) out vec4 outColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

uniform vec4 diffuseColor;
uniform vec4 specularColor;

in vec3 normal;
in vec2 texcoord;

void main() 
{
	vec4 diffuse = texture(diffuseMap, texcoord);// * diffuseColor;
	outColor = diffuse;
	
	//outColor = vec4(texcoord.x, texcoord.y, 0, 1);
	outColor = vec4(normal, 1);
}
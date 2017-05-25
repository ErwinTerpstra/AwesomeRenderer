#version 330

in vec3 inPosition;
in vec2 inTexcoord;

out vec2 texcoord;

void main()
{
	gl_Position = vec4(inPosition, 1.0);
	texcoord = inTexcoord;
}

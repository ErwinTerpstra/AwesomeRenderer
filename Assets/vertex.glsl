#version 330

in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexcoord;

uniform mat4 modelMtx;
uniform mat4 viewMtx;
uniform mat4 projMtx;

out vec3 normal;
out vec2 texcoord;

void main()
{
	mat4 mvp = projMtx * viewMtx * modelMtx;
    
    gl_Position = mvp * vec4(inPosition, 1.0);
    normal = inNormal;
    texcoord = inTexcoord;
}

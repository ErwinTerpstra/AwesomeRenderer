#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;

uniform mat4 modelMtx;
uniform mat4 viewMtx;
uniform mat4 projMtx;

varying vec3 normal;
varying vec2 texcoord;

void main()
{
	mat4 mvp = projMtx * viewMtx * modelMtx;
    
    gl_Position = mvp * vec4(inPosition, 1.0);
    normal = inNormal;
    texcoord = inTexcoord;
}
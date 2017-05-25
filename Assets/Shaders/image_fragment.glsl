#version 330

#define GAMMA 2.2

layout(location = 0) out vec4 outColor;

uniform sampler2D image;
uniform float gamma;

in vec2 texcoord;

vec3 Tonemap(vec3 x)
{
	// Very basic reinhard tonemapping
	//return x / (x + 1);
	
	// Approximation of Aces filming tonemapping curve:
	// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	const float a = 2.51f;
	const float b = 0.03f;
	const float c = 2.43f;
	const float d = 0.59f;
	const float e = 0.14f;
	
	return clamp((x*(a*x + b)) / (x*(c*x + d) + e), 0.0, 1.0);
}

void main()
{
	vec4 color = texture(image, texcoord);
	color.rgb = Tonemap(color.rgb);

	vec4 result;
	result.rgb = pow(color.rgb, vec3(1.0 / GAMMA));
	result.a = color.a;

	outColor = result;
}
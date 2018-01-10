#define GAMMA 2.2

layout(location = 0) out vec4 outColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

uniform vec4 diffuseColor;
uniform vec4 specularColor;

uniform vec3 cameraPosition;

in vec2 texcoord;

in vec4 worldPos;

in mat3 tbnMtx;

const vec3 lightAmbient = vec3(0.1, 0.1, 0.1);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 lightDirection = vec3(0, -0.894427, -0.447214);
const float lightIntensity = 1.0;

void main() 
{

#ifdef USE_NORMAL_MAP
	// Normal mapping
	vec3 normal = texture(normalMap, texcoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(tbnMtx * normal);
#else
	vec3 normal = tbnMtx[2];
#endif

	vec3 radiance = vec3(0, 0, 0);

	vec3 light = lightAmbient + (lightColor * lightIntensity);

	// Diffuse
	vec4 diffuse = texture(diffuseMap, texcoord);
	diffuse.rgb = pow(diffuse.rgb, vec3(GAMMA));
	diffuse *= diffuseColor;

	float NdotL = max(dot(-lightDirection, normal), 0.0);
	radiance += diffuse.rgb * (lightAmbient + (lightColor * lightIntensity) * NdotL);

	// Specular
	vec4 specular = texture(specularMap, texcoord);
	specular.rgb = pow(specular.rgb, vec3(GAMMA));
	specular *= specularColor;

	vec3 v = normalize(cameraPosition - (worldPos.xyz / worldPos.w));
	vec3 h = normalize(-lightDirection + v);
	float NdotH = max(dot(h, normal), 0.0);

	radiance += specular.rgb * (lightColor * lightIntensity) *  pow(NdotH, specularColor.a);
	
#ifndef OUTPUT_LINEAR
	// Gamma correct
	radiance = pow(radiance, vec3(1.0 / GAMMA));
#endif

	outColor = vec4(radiance, diffuse.a);

	//outColor = vec4(specular.rgb, 1.0);
	
	//outColor = vec4(texcoord.x, texcoord.y, 0, 1);
	//outColor = vec4(0.5 + normal * 0.5, 1);
	//outColor = vec4(0.5 + tbnMtx[0] * 0.5, 1);
}
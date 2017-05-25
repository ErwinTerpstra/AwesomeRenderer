in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexcoord;

in vec3 inTangent;
in vec3 inBitangent;

uniform mat4 modelMtx;
uniform mat4 viewMtx;
uniform mat4 projMtx;

out vec2 texcoord;

out vec4 worldPos;

out mat3 tbnMtx;

void main()
{
	worldPos = modelMtx * vec4(inPosition, 1.0);
	gl_Position = projMtx * viewMtx * worldPos;

    vec3 t = normalize(mat3(modelMtx) * inTangent);
    vec3 b = normalize(mat3(modelMtx) * inBitangent);
    vec3 n = normalize(mat3(modelMtx) * inNormal);

    tbnMtx = mat3(t, b, n);

    texcoord = inTexcoord;
}

#ifndef _PHONG_SHADER_H_
#define _PHONG_SHADER_H_

#define MAX_LIGHTS 8

namespace AwesomeRenderer
{

	class PhongShader : public SoftwareShader
	{

	public:

		enum LightType
		{
			POINT,
			SPOT,
			DIRECTIONAL
		};

		struct Light
		{
			Vector3 position, direction;

			Color color;

			float angle;
			float intensity;

			float constantAttenuation, lineairAttenuation, quadricAttenuation;

			LightType type;

			bool enabled;

			Light() :
				position(0.0f, 0.0f, 0.0f), direction(0.0f, 1.0f, 0.0f), color(Color::WHITE), angle(0.0f), intensity(1.0f),
				constantAttenuation(1.0f), lineairAttenuation(0.0f), quadricAttenuation(0.0f),
				type(LightType::POINT), enabled(false)
			{

			}
		};

		struct LightData
		{
			Light lights[MAX_LIGHTS];

			Color ambient;

			int numPixelLights;
		};

		LightData lightData;

	public:
		PhongShader();
		
		virtual void ProcessVertex(const VertexInfo& in, VertexToPixel& out) const;
		virtual void ProcessPixel(const VertexToPixel& in, PixelInfo& out) const;

	};

}

#endif
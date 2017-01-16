#ifndef _LIGHT_DATA_H_
#define _LIGHT_DATA_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{
	class Renderable;

	class LightData
	{
	public:
		static const uint32_t MAX_LIGHTS = 8;

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

			float angle, angleExponent;
			float intensity;

			float constantAttenuation, lineairAttenuation, quadricAttenuation;

			LightType type;

			bool enabled;

			Light() :
				position(0.0f, 0.0f, 0.0f), direction(0.0f, 1.0f, 0.0f), color(Color::WHITE),
				angle(0.0f), angleExponent(1.0f), intensity(1.0f),
				constantAttenuation(1.0f), lineairAttenuation(0.0f), quadricAttenuation(0.0f),
				type(LightType::POINT), enabled(false)
			{

			}
		};

		Light lights[MAX_LIGHTS];

		std::vector<Renderable*> areaLights;

		Color ambient;

		int numPixelLights;

	public:
		LightData();
	};

}

#endif
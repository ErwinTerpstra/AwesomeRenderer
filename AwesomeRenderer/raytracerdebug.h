#ifndef _RAYTRACER_DEBUG_H_
#define _RAYTRACER_DEBUG_H_

#include "unlitshader.h"

namespace AwesomeRenderer
{
	class InputManager;
	class TextMesh;
	class Context;

	namespace RayTracing
	{
		class RayTracer;
		class SurfaceIntegrator;

		class RayTracerDebug
		{

		private:
			static const uint32_t INTEGRATOR_COUNT = 3;

			Context& context;
			RayTracer& rayTracer;
			InputManager& inputManager;

			char* textBuffer;

			UnlitShader unlitShader;
			TextMesh* debugText;

			// Integrator switching
			SurfaceIntegrator* integrators[INTEGRATOR_COUNT];
			uint32_t currentIntegrator;

		public:
			RayTracerDebug(Context& context, RayTracer& rayTracer);
			~RayTracerDebug();

			void Setup();
			void Update();

		private:
			void SetupDebugDisplay();
			void UpdateDebugDisplay();
		};
	}
}

#endif

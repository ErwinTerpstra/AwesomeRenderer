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
			static const std::string RENDER_ROOT;
			static const uint32_t INTEGRATOR_COUNT = 3;
			static const uint32_t TEXT_BUFFER_SIZE = 1024;
			static const float UPDATE_INTERVAL;

			enum ExportMode
			{
				DISABLED,
				ONCE,
				CONTINUOUS,

				EXPORT_MODE_COUNT
			};

			Context& context;
			RayTracer& rayTracer;
			InputManager& inputManager;

			char* textBuffer;

			UnlitShader unlitShader;
			TextMesh* debugText;

			// Integrator switching
			SurfaceIntegrator* integrators[INTEGRATOR_COUNT];
			uint32_t currentIntegrator;

			ExportMode exportMode;

			float timeSinceUpdate;

		public:
			RayTracerDebug(Context& context, RayTracer& rayTracer);
			~RayTracerDebug();

			void Setup();
			void Update(float dt);

			void Export();

		private:
			void SetupDebugDisplay();
			void UpdateDebugDisplay();

			std::string FormatTime(float time);
		};
	}
}

#endif

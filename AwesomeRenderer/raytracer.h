#ifndef _RAY_TRACER_H_
#define _RAY_TRACER_H_

#include "awesomerenderer.h"
#include "renderer.h"
#include "timer.h"

namespace AwesomeRenderer
{
	class Ray;
	struct RaycastHit;

	class Window;
	class Random;

	class PhongMaterial;
	class PbrMaterial;

	namespace RayTracing
	{
		class RayTracer : public Renderer
		{

		private:
			static const float MAX_FRAME_TIME;

			struct ShadingInfo
			{
				Color color;
			};

			Timer timer;
			Timer frameTimer;

			Random& random;

			uint32_t pixelIdx;
			std::vector<Point2> pixelList;

		public:

			uint32_t maxDepth;
			uint32_t sampleCount;

		public:

			RayTracer();

			void Initialize();
			void Render();
			void Present(Window& window);
			void Cleanup();
			void ResetFrame();

			void SetRenderContext(const RenderContext* context);

			float GetProgress() const { return pixelIdx / (float)pixelList.size(); }
		private:

			void PreRender();
			void PostRender();

			void Render(const Point2& pixel);
			void CalculateShading(const Ray& ray, ShadingInfo& shadingInfo, int depth = 0);

			void CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const PhongMaterial& material, ShadingInfo& shadingInfo, int depth);
			void CalculateShading(const Ray& ray, const RaycastHit& hitInfo, const PbrMaterial& material, ShadingInfo& shadingInfo, int depth);

			bool RayCast(const Ray& ray, RaycastHit& nearestHit, float maxDistance = FLT_MAX);
			void Trace(const Ray& ray, const Point2& screenPosition);

			float Fresnel(const Vector3& v, const Vector3& normal, float ior);

			Vector3 GenerateSampleVector(const Vector3& v, const Vector3& n, float roughness, float& pdf);
			void ImportanceSampleGGX(const Vector2& r, float alpha, float& phi, float& theta);
			float PDFGGX(float phi, float theta, float alpha);

			Vector3 DiffuseLambert(const Vector3& albedo);
			Vector3 SpecularCookTorrance(const Vector3& v, const Vector3& n, const Vector3& l, const Vector3& F0, float roughness, Vector3& ks);

			float RoughnessToShininess(float a);

			float DistributionBlinn(const Vector3& n, const Vector3& h, float e);
			float DistributionGGX(const Vector3& n, const Vector3& h, float alpha);

			float GeometryImplicit(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h);
			float GeometryCookTorrance(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h);
			float GeometrySmith(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a);
			float GeometryGGX(const Vector3& v, const Vector3& l, const Vector3& n, const Vector3& h, float a);

			float G1Schlick(const Vector3& v, const Vector3& n, float a);
			float G1GGX(const Vector3& v, const Vector3& n, const Vector3& h, float a);

			Vector3 FresnelSchlick(float cosT, Vector3 F0);

		};

	}

}

#endif
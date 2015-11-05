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

	class PhongMaterial;
	class PbrMaterial;

	class RayTracer : public Renderer
	{

	private:
		static const float MAX_FRAME_TIME;
		static const int MAX_DEPTH;

		struct ShadingInfo
		{
			Color color;
		};

		Timer timer;
		Timer frameTimer;

		uint32_t pixelIdx;
		std::vector<Point2> pixelList;

	public:

		RayTracer();

		void Initialize();
		void Render();
		void Present(Window& window);
		void Cleanup();

		void SetRenderContext(const RenderContext* context);
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

		Vector3 SpecularCookTorrance(const Vector3& radiance, const Vector3& v, const Vector3& n, const Vector3& l, const Vector3& F0, float roughness, Vector3& ks);
		float chiGGX(float v);
		float DistributionGGX(Vector3 n, Vector3 h, float alpha);
		float GeometryGGX(Vector3 v, Vector3 n, Vector3 h, float alpha);
		Vector3 FresnelSchlick(float cosT, Vector3 F0);

	};

}

#endif
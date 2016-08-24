#ifndef _SETUP_H_
#define _SETUP_H_

namespace AwesomeRenderer
{
	class LightData;
	class RenderContext;
	class ObjLoader;
	class TextureFactory;
	class Camera;

	void SetupScene(RenderContext& mainContext, RenderContext& hudContext, ObjLoader& objLoader, TextureFactory& textureFactory);
	void SetupLighting(LightData& lightData);

	void SetupCornellBox(RenderContext& context, Camera& camera);
	void SetupSpheres(RenderContext& context, Camera& camera);
}

#endif
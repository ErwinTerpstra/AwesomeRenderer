#ifndef _CONTEXT_H_
#define _CONTEXT_H_

namespace AwesomeRenderer
{
	class LightData;
	class RenderContext;
	class ObjLoader;
	class TextureFactory;
	class Camera;
	class Window;

	class Context
	{

	public:
		Window* window;

		RenderContext* mainContext;
		RenderContext* hudContext;

		Camera* mainCamera;
		Camera* hudCamera;

		ObjLoader* objLoader;
		TextureFactory* textureFactory;


	};

}

#endif
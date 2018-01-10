#ifndef _DEBUG_DISPLAY_H_
#define _DEBUG_DISPLAY_H_

namespace AwesomeRenderer
{
	class Context;
	class RenderContext;
	class InputManager;

	class DebugDisplay
	{

	private:
		Context& context;
		RenderContext& renderContext;
		InputManager& input;

	public:
		DebugDisplay(Context& context, RenderContext& renderContext);

		void Update(float dt);
		void Render();

	};

}

#endif
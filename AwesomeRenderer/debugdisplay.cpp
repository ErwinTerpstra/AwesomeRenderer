#include "debugdisplay.h"
#include "context.h"
#include "rendercontext.h"
#include "rendertarget.h"
#include "texture.h"
#include "window.h"

#include "inputmanager.h"

#include "imgui/imgui.h"

using namespace AwesomeRenderer;

DebugDisplay::DebugDisplay(Context& context, RenderContext& renderContext) : 
	context(context), renderContext(renderContext),
	input(InputManager::Instance())
{

}

void DebugDisplay::Update(float dt)
{
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	int w = renderContext.renderTarget->frameBuffer->width;
	int h = renderContext.renderTarget->frameBuffer->height;
	int ww = context.window->width;
	int wh = context.window->height;

	io.DisplaySize = ImVec2((float)w, (float)h);
	io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)ww / w) : 0, h > 0 ? ((float)wh / h) : 0);

	// Setup time step
	io.DeltaTime = dt;

	// Setup inputs
	// (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
	if (context.window->HasFocus())
	{
		assert(!io.WantMoveMouse);

		Point2 mousePosition = input.GetMousePosition();
		io.MousePos = ImVec2((float) mousePosition[0], (float)mousePosition[1]);
	}
	else
	{
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
	}

	io.MouseDown[0] = input.GetKey(InputManager::LEFT_MOUSE_BUTTON);

	//io.MouseWheel = Current frame mousewheel delta;

	// Hide OS mouse cursor if ImGui is drawing it (io.MouseDrawCursor)

	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	ImGui::NewFrame();

	static bool demoWindowOpen = true;
	ImGui::ShowDemoWindow(&demoWindowOpen);
}

void DebugDisplay::Render()
{

}
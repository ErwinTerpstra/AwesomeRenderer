#include "awesomerenderer.h"

using namespace AwesomeRenderer;


Renderer::Renderer() : shader(), cullMode(CULL_BACK)
{

}

void Renderer::SortTriangle(Shader::VertexToPixel** a, Shader::VertexToPixel** b, Shader::VertexToPixel** c)
{
	if ((*a)->screenPosition[1] > (*b)->screenPosition[1]) Swap<Shader::VertexToPixel>(a, b);
	if ((*b)->screenPosition[1] > (*c)->screenPosition[1]) Swap<Shader::VertexToPixel>(b, c);
	if ((*a)->screenPosition[1] > (*b)->screenPosition[1]) Swap<Shader::VertexToPixel>(a, b);
}

template <typename T>
void Renderer::Swap(T** a, T** b)
{
	T* tmp = *a;
	*a = *b;
	*b = tmp;
}

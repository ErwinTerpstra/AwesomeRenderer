#include "renderable.h"

using namespace AwesomeRenderer;

Renderable::Renderable()
{

}

Renderable::~Renderable()
{

}

const Primitive& Renderable::GetShape() const
{
	return *primitive;
}
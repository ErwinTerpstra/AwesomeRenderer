#include "awesomerenderer.h"

using namespace AwesomeRenderer;

Node::Node() : model(NULL), transform(NULL)
{

}

const Shape& Node::GetShape() const { return model->bounds; }
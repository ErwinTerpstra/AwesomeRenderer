#include "awesomerenderer.h"
#include "node.h"

using namespace AwesomeRenderer;

Node::Node()
{
	for (int id = 0; id < Component::LAST_ID; ++id)
	{
		components[id] = NULL;
	}
}
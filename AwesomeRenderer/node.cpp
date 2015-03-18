#include "awesomerenderer.h"
#include "node.h"

using namespace AwesomeRenderer;

Node::Node()
{

}

template <class T>
void Node::AddComponent(Component* component, bool replace)
{
	if (!replace && HasComponent<T>())
		return;

	components[T::id] = component;
}

template <class T>
bool Node::RemoveComponent()
{
	bool present = HasComponent<T>();

	components[T:id] = NULL;
}

template <class T>
bool Node::HasComponent()
{
	return components[T::id] != NULL;
}

template <class T>
T* Node::GetComponent()
{
	return components[T::id];
}
#ifndef _NODE_H_
#define _NODE_H_

#include "component.h"

namespace AwesomeRenderer
{

	class Node
	{

	private:
		Component* components[Component::LAST_ID];

	public:
		Node();

		template <class T>
		void AddComponent(Component* component, bool replace = true);

		template <class T>
		bool RemoveComponent();

		template <class T>
		bool HasComponent();

		template <class T>
		T* GetComponent();

	};

}

#endif
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
		void AddComponent(T* component, bool replace = true)
		{
			if (!replace && HasComponent<T>())
				return;

			components[T::id] = component;
		}

		template <class T>
		bool RemoveComponent()
		{
			bool present = HasComponent<T>();

			components[T:id] = NULL;
		}

		template <class T>
		bool HasComponent()
		{
			return components[T::id] != NULL;
		}

		template <class T>
		T* GetComponent()
		{
			return static_cast<T*>(components[T::id]);
		}

		template <class T>
		const T* GetComponent() const
		{
			return static_cast<const T*>(components[T::id]);
		}

	};

}

#endif
#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class InputManager
	{

	public:
		static const uint32_t MAX_KEYS = 512;

		enum Keys
		{
			LEFT_MOUSE_BUTTON = 256
		};

	private:
		bool keys[MAX_KEYS];
		bool prevKeys[MAX_KEYS];
		
		Point2 mousePosition;

		static InputManager* instance;

	public:
		void Update();

		bool GetKey(uint8_t key) const;
		bool GetKeyDown(uint8_t key) const;
		bool GetKeyUp(uint8_t key) const;

		void KeyUp(uint8_t key);
		void KeyDown(uint8_t key);

		void SetMousePosition(const Point2& position);
		Point2 GetMousePosition() const;

		static InputManager& Instance();

	private:
		InputManager();

	};

}

#endif
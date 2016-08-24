#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

#include "awesomerenderer.h"

namespace AwesomeRenderer
{

	class InputManager
	{

	public:
		static const uint32_t MAX_KEYS = 256;

	private:
		bool keys[MAX_KEYS];
		bool prevKeys[MAX_KEYS];

		static InputManager* instance;

	public:
		void Update();

		bool GetKey(uint8_t key);
		bool GetKeyDown(uint8_t key);
		bool GetKeyUp(uint8_t key);

		void KeyUp(uint8_t key);
		void KeyDown(uint8_t key);

		static InputManager& Instance();

	private:
		InputManager();

	};

}

#endif
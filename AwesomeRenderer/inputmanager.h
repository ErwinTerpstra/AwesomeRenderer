#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

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

		bool GetKey(char key);
		bool GetKeyDown(char key);
		bool GetKeyUp(char key);

		void KeyUp(char key);
		void KeyDown(char key);

		static InputManager& Instance();

	private:
		InputManager();

	};

}

#endif
#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

#define MAX_KEYS 256

namespace AwesomeRenderer
{

	class InputManager
	{

	public:

	private:
		bool keys[MAX_KEYS];

		static InputManager* instance;

	public:

		bool GetKey(char key);

		void KeyUp(char key);
		void KeyDown(char key);

		static InputManager& Instance();

	private:
		InputManager();

	};

}

#endif
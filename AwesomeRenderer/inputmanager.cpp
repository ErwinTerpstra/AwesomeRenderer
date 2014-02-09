#include "awesomerenderer.h"

using namespace AwesomeRenderer;

InputManager* InputManager::instance = NULL;

InputManager::InputManager()
{
	for (unsigned int i = 0; i < MAX_KEYS; ++i)
		keys[i] = false;
}

bool InputManager::GetKey(char key)
{
	return keys[key];
}

void InputManager::KeyDown(char key)
{
	keys[key] = true;
}

void InputManager::KeyUp(char key)
{
	keys[key] = false;
}

InputManager& InputManager::Instance()	
{
	if (instance == NULL)
		instance = new InputManager();

	return *instance;
}
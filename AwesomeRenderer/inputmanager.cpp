#include "awesomerenderer.h"
#include "inputmanager.h"

using namespace AwesomeRenderer;

InputManager* InputManager::instance = NULL;

InputManager::InputManager()
{
	for (uint32_t i = 0; i < MAX_KEYS; ++i)
	{
		keys[i] = false;
		prevKeys[i] = false;
	}
}

void InputManager::Update()
{
	for (uint32_t i = 0; i < MAX_KEYS; ++i)
		prevKeys[i] = keys[i];
}

bool InputManager::GetKey(char key)
{
	return keys[key];
}

bool InputManager::GetKeyDown(char key)
{
	return keys[key] && !prevKeys[key];
}

bool InputManager::GetKeyUp(char key)
{
	return !keys[key] && prevKeys[key];
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
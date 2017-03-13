#include "InputManager.h"

CInputManager::CInputManager() {}

CInputManager::CInputManager(const CInputManager &Other) {}

CInputManager::~CInputManager() {}

bool CInputManager::Init() {
	for (int i = 0; i < 256; i++)
		IsKeyPressed[i] = false;

	return true;
}

bool CInputManager::IsKeyDown(unsigned int KeyCode) {
	return IsKeyPressed[KeyCode];
}

void CInputManager::KeyDown(unsigned int KeyCode) {
	IsKeyPressed[KeyCode] = true;
}

void CInputManager::KeyUp(unsigned int KeyCode) {
	IsKeyPressed[KeyCode] = false;
}
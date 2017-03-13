#pragma once

class CInputManager {
public:
	CInputManager();
	CInputManager(const CInputManager &);
	~CInputManager();

	bool Init();
	bool IsKeyDown(unsigned int);
	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

private:
	bool IsKeyPressed[256];
};
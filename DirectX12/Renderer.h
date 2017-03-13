#pragma once
#include <Windows.h>

class CRenderer {
public:
	bool Init(int, int, HWND);
	bool Frame();
};
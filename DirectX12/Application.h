#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include "InputManager.h"
#include "Renderer.h"

class CApplication {
public:
	CApplication();
	CApplication(const CApplication&);
	~CApplication();

	bool Init();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitWindows(int&, int&);
	void ShutdownWindows();

	LPCWSTR ApplicationName;
	HINSTANCE hInstance;
	HWND hWnd;

	CInputManager* InputManager;
	CRenderer* Renderer;

	bool Fullscreen = true;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static CApplication* ApplicationHandle = nullptr;

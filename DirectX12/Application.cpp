#include "Application.h"

CApplication::CApplication() : InputManager(nullptr), Renderer(nullptr) {}

CApplication::CApplication(const CApplication &Other) {}

CApplication::~CApplication() {
	if (InputManager) {
		delete InputManager;
		InputManager = nullptr;
	}

	if (Renderer) {
		delete Renderer;
		Renderer = nullptr;
	}
}

bool CApplication::Init() {
	int ScreenWidth(0), ScreenHeight(0);
	bool bResult;

	InitWindows(ScreenHeight, ScreenWidth);

	InputManager = new CInputManager();
	if (!InputManager)
		return false;

	InputManager->Init();

	Renderer = new CRenderer();

	if (!Renderer)
		return false;

	bResult = Renderer->Init(ScreenWidth, ScreenHeight, hWnd);

	if (!bResult)
		return false;

	return true;
}

void CApplication::Shutdown() {
	ShutdownWindows();
}

void CApplication::Run() {
	MSG Msg;
	bool bDone, bResult;

	ZeroMemory(&Msg, sizeof(MSG));

	bDone = false;
	while (!bDone) {
		if (!PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

		if (Msg.message == WM_QUIT) {
			bDone = true;
		}
		else {
			bResult = Frame();
			if (!bResult)
				bDone = true;
		}
	}
}

bool CApplication::Frame() {

	if (InputManager->IsKeyDown(VK_ESCAPE))
		return false;

	return Renderer->Frame();
}

LRESULT CALLBACK CApplication::MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_KEYDOWN:
			InputManager->KeyDown((unsigned int)wParam);
			return 0;

		case WM_KEYUP:
			InputManager->KeyUp((unsigned int)wParam);
			return 0;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void CApplication::InitWindows(int &ScreenWidth, int &ScreenHeight) {
	WNDCLASSEX WindowClass;
	DEVMODE ScreenSettings;
	int PosX, PosY;

	ApplicationHandle = this;

	hInstance = GetModuleHandle(NULL);

	ApplicationName = L"Engine";

	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = WndProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = hInstance;
	WindowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	WindowClass.hIconSm = WindowClass.hIcon;
	WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = ApplicationName;
	WindowClass.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&WindowClass);

	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (Fullscreen) {
		memset(&ScreenSettings, 0, sizeof(ScreenSettings));
		ScreenSettings.dmSize = sizeof(ScreenSettings);
		ScreenSettings.dmPelsHeight = (unsigned long)ScreenHeight;
		ScreenSettings.dmPelsWidth = (unsigned long)ScreenWidth;
		ScreenSettings.dmBitsPerPel = 32;
		ScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&ScreenSettings, CDS_FULLSCREEN);

		PosX = PosY = 0;
	}
	else {
		ScreenWidth = 1024;
		ScreenHeight = 768;

		PosX = (GetSystemMetrics(SM_CXSCREEN) - ScreenWidth) / 2;
		PosY = (GetSystemMetrics(SM_CYSCREEN) - ScreenHeight) / 2;
	}

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, ApplicationName, ApplicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		PosX, PosY, ScreenWidth, ScreenHeight, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	ShowCursor(false);
}

void CApplication::ShutdownWindows() {
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (Fullscreen) {
		ChangeDisplaySettings(nullptr, 0);
	}

	// Remove the window.
	DestroyWindow(hWnd);
	hWnd = 0;

	// Remove the application instance.
	UnregisterClass(ApplicationName, hInstance);
	hInstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = nullptr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) {
	switch (uMessage) {
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}

		case WM_CLOSE: {
			PostQuitMessage(0);
			return 0;
		}

		default: {
			return ApplicationHandle->MessageHandler(hWnd, uMessage, wParam, lParam);
		}
	}
}

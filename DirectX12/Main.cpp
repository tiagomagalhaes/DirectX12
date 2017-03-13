#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	CApplication *Application;
	bool bResult;

	Application = new CApplication;
	
	if (!Application)
		return 0;

	bResult = Application->Init();
	
	if (bResult)
		Application->Run();

	Application->Shutdown();
	delete Application;

	return 0;
}
#include <Windows.h>
#include "Renderer.h"
#include "Window.h"
#include "Timer.h"
#include <sstream>
#include <string>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    Window window;
	window.create(800, 600);
    
	Timer timer;

    Renderer renderer;
    renderer.Initialize(window);

    while (true)
    {
		/*std::ostringstream oss;
		oss << timer.time() << std::endl;
		std::string str = oss.str();
		std::wstring wstr(str.begin(), str.end());
		OutputDebugString(wstr.c_str());*/

		//return true if the message is WM_QUIT
		if (window.processMessages()) break;
		renderer.cleanup();
		float currentTime = timer.time();	
		renderer.updateConstantBuffer("buffername1", "time", &currentTime, sizeof(float));

		renderer.Render();
		renderer.present();
    }

    return 0;
}
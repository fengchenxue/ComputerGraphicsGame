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

	std::vector<StaticVertex> vertices;
	std::vector<unsigned int> indices;

	drawASphere(vertices, indices);

	renderer.Initialize(window, vertices, indices);

    while (true)
    {
		/*std::ostringstream oss;
		oss << timer.time() << std::endl;
		std::string str = oss.str();
		std::wstring wstr(str.begin(), str.end());
		OutputDebugString(wstr.c_str());*/

		//return true if the message is WM_QUIT
		if (window.processMessages()) break;
		renderer.cleanFrame();

		//update the world matrix
		DirectX::XMFLOAT4X4 planeWorld(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		renderer.updateConstantBuffer(true, "staticMeshBuffer", "W", &planeWorld, sizeof(planeWorld));

		//update VP
		DirectX::XMVECTOR eye = DirectX::XMVectorSet(10*cosf(timer.time()), 5.0f, 10*sinf(timer.time()), 0.0f);
		//DirectX::XMVECTOR eye = DirectX::XMVectorSet(11.f, 5.0f, 11.f, 0.0f);
		DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixLookAtLH(eye, at, up));
		//DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, at, up);
		//DirectX::XMFLOAT4X4 viewMatrixF;
		//DirectX::XMStoreFloat4x4(&viewMatrixF, viewMatrix);


        constexpr float fov = DirectX::XMConvertToRadians(45.0f);
		float aspectRatio = static_cast<float>(window.width) / static_cast<float>(window.height);
		float nearZ = 0.1f;
		float farZ = 1000.0f;
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ));
		//DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);
		DirectX::XMMATRIX VP = projectionMatrix * viewMatrix;
		//VP = DirectX::XMMatrixTranspose(VP);
		DirectX::XMFLOAT4X4 VPF;
		DirectX::XMStoreFloat4x4(&VPF, VP);

		renderer.updateConstantBuffer(true, "staticMeshBuffer", "VP", &VPF, sizeof(VPF));

		renderer.Render();
		renderer.present();
    }
	renderer.cleanup();
    return 0;
}
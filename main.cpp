#include <Windows.h>
#include "Renderer.h"
#include "Window.h"
#include "Timer.h"
#include <sstream>
#include <string>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

void ComputeTangent(DirectX::XMFLOAT3 normal, DirectX::XMFLOAT3& tangent) {
	DirectX::XMVECTOR Temporaryt;
	if (fabs(normal.x) > 0.9999f && fabs(normal.y) < 0.0001f && fabs(normal.z) < 0.0001f) {
		Temporaryt = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	}
	else {
		Temporaryt = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	}
	DirectX::XMVECTOR n = DirectX::XMLoadFloat3(&normal);
	DirectX::XMVECTOR t = DirectX::XMVector3Cross(n, Temporaryt);
	t = DirectX::XMVector3Normalize(t);
	DirectX::XMStoreFloat3(&tangent, t);
}

StaticVertex addVertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT2 uvCoords) {
	StaticVertex vertex;
	vertex.position = position;
	vertex.normal = normal;	
	DirectX::XMFLOAT3 tangent;
	ComputeTangent(normal, tangent);
	vertex.tangent = tangent;
	vertex.uvCoords = uvCoords;
	return vertex;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    Window window;
	window.create(800, 600);
    
	Timer timer;

	Renderer renderer;

	std::vector<StaticVertex> vertices;
	vertices.push_back(addVertex(DirectX::XMFLOAT3(-15.0f, 0.0f, -15.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  DirectX::XMFLOAT2(0.0f, 0.0f)));
	vertices.push_back(addVertex(DirectX::XMFLOAT3(15.0f, 0.0f, -15.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)));
	vertices.push_back(addVertex(DirectX::XMFLOAT3(-15.0f, 0.0f, 15.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  DirectX::XMFLOAT2(0.0f, 1.0f)));
	vertices.push_back(addVertex(DirectX::XMFLOAT3(15.0f, 0.0f, 15.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
	std::vector<unsigned int> indices = { 0, 1, 2, 2, 1, 3 };

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
		DirectX::XMVECTOR eye = DirectX::XMVectorSet(40*cosf(timer.time()), 20.0f, 40*sinf(timer.time()), 0.0f);
		//DirectX::XMVECTOR eye = DirectX::XMVectorSet(11.f, 5.0f, 11.f, 0.0f);
		DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixLookAtLH(eye, at, up));
		//DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, at, up);
		//DirectX::XMFLOAT4X4 viewMatrixF;
		//DirectX::XMStoreFloat4x4(&viewMatrixF, viewMatrix);


        constexpr float fov = DirectX::XMConvertToRadians(45.0f);
		float aspectRatio = 16.0f / 9.0f;
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
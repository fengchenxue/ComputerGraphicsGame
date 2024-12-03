#include <Windows.h>
#include "Renderer.h"
#include "Window.h"
#include "Timer.h"
#include "GEMLoader.h"
#include <sstream>
#include <string>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
void GEMStaticVertexToStaticVertex(GEMLoader::GEMStaticVertex & gemVertices, StaticVertex & vertices)
{
	vertices.position = { gemVertices.position.x,gemVertices.position.y,gemVertices.position.z };
	vertices.normal = { gemVertices.normal.x,gemVertices.normal.y,gemVertices.normal.z };
	vertices.tangent = { gemVertices.tangent.x,gemVertices.tangent.y,gemVertices.tangent.z };
	vertices.uvCoords = { gemVertices.u,gemVertices.v };

}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	Window window;
	window.create(800, 600);
	Timer timer;
	Renderer renderer;
	std::vector<Mesh> meshes;
	std::vector<StaticVertex> vertices;
	std::vector<unsigned int> indices;
	
	//load meshes
	GEMLoader::GEMModelLoader loader;
	std::vector<GEMLoader::GEMMesh> gemmeshes; 
	loader.load("Res/acacia_003.gem", gemmeshes);
	for (auto& gemmesh : gemmeshes)
	{
		//store all of the mesh info in one index buffer and one vertex buffer, 
		//so we need to store the offset of the mesh in the buffer
		Mesh mesh;
		mesh.indeicesOffsets = indices.size();
		mesh.verticesOffsets = vertices.size();
		meshes.push_back(mesh);

		StaticVertex v;
		for (auto& vertex : gemmesh.verticesStatic)
		{
			GEMStaticVertexToStaticVertex(vertex, v);
			vertices.push_back(v);
		}
		
		for (auto& index : gemmesh.indices)
		{
			indices.push_back(index+mesh.verticesOffsets);
		}
	}

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
		DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f));
		//DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(timer.time()));
		DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f));

		DirectX::XMMATRIX planeWorld = translationMatrix * scaleMatrix;
		DirectX::XMFLOAT4X4 planeWorldF;
		DirectX::XMStoreFloat4x4(&planeWorldF, planeWorld);
		renderer.updateConstantBuffer(true, "staticMeshBuffer", "W", &planeWorldF, sizeof(planeWorldF));

		//update V
		DirectX::XMVECTOR eye = DirectX::XMVectorSet(10*cosf(timer.time()), 10.0f, 10*sinf(timer.time()), 0.0f);
		//DirectX::XMVECTOR eye = DirectX::XMVectorSet(11.f, 5.0f, 11.f, 0.0f);
		DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixLookAtLH(eye, at, up));
	
		//update P
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

		/*planeWorldF = DirectX::XMMatrixTranspose( DirectX::XMMatrixTranslation(-5.0f, 0.0f, 0.0f));
		DirectX::XMStoreFloat4x4(&planeWorld, planeWorldF);
		renderer.updateConstantBuffer(true, "staticMeshBuffer", "W", &planeWorld, sizeof(planeWorld));
		renderer.Render();*/

		renderer.present();
    }
	renderer.cleanup();
    return 0;
}
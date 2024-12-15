#include <Windows.h>
#include "Renderer.h"
#include "Window.h"
#include "Timer.h"
#include "Object.h"
#include <sstream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	//initialize the window, timer, renderer, meshManager, loader, gemmeshes, gemanimation
	Window window;
	window.create(1920, 1080);
	Timer timer;
	Renderer renderer;
	MeshManager meshManager;
	ObjectManager objectManager;
	Map map;
	Player player;
	window.player = &player;

	//load from file
	std::string filename = "Input.txt";
	meshManager.loadlevel(filename,objectManager,map);
	
	//set initial position
	map.CheckVerticalCollision_Player(player);
	for (auto& npc : objectManager.npcs)
	{
		map.CheckVerticalCollision_Object(npc);
	}

	//initialize renderer
	renderer.Initialize(window, meshManager);

	//initialize texture
	for (auto& pair : meshManager.objects)
	{
		if (pair.first == "Terrain")
		{
			renderer.loadTexture(pair.second.textureFile, "tex0");
		}
		else if (pair.first == "NPC")
		{
			renderer.loadTexture(pair.second.textureFile, "tex1");
		}
		else
		{
			renderer.loadTexture(pair.second.textureFile, "tex2");
		}
	}

	float dt;
    while (true)
    {
		//return true if the message is WM_QUIT
		if (window.processMessages()) break;
		renderer.cleanFrame();
		//update dt
		dt = timer.dt();

		//update player
		player.move(window, dt,map,objectManager);
		map.CheckVerticalCollision_Player(player);

		int i = 0;
		//update bones and check collision with player
		for (auto& npc : objectManager.npcs)
		{
			if (npc.isAlive && npc.checkCollisionWithPlayer(player)) {
				npc.isAlive = false;
				npc.animationInstance.update("death", 0.0f);
			}
			else {
				npc.animationInstance.update(npc.animationInstance.sequenceName, dt);
			}
			meshManager.updateBonesVector(npc.animationInstance.BonesTransforms,i);
			i++;
		}
		renderer.updataBonesBuffer(meshManager.bonesVector);

		//update V
		DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&player.position);
		//DirectX::XMVECTOR at = eye + DirectX::XMLoadFloat3(&player.forward);
		//DirectX::XMVECTOR to = DirectX::XMLoadFloat3(&player.forward);
		//DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookToLH (eye, player.forward, player.up);
		DirectX::XMMATRIX skyboxViewMatrix = viewMatrix;
		viewMatrix = DirectX::XMMatrixTranspose(viewMatrix);

		//update P
        constexpr float fov = DirectX::XMConvertToRadians(45.0f);
		float aspectRatio = static_cast<float>(window.width) / static_cast<float>(window.height);
		float nearZ = 0.1f;
		float farZ = 2000.0f;
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ));
		DirectX::XMMATRIX VP = projectionMatrix * viewMatrix;

		DirectX::XMFLOAT4X4 VPF;
		DirectX::XMStoreFloat4x4(&VPF, VP);
		renderer.updateConstantBuffer(true, "cbVS_D", "VP", &VPF, sizeof(VPF));
		renderer.updateConstantBuffer(true, "cbVS_S", "VP", &VPF, sizeof(VPF));

		//update skybox VP
		skyboxViewMatrix.r[3] = { 0,0,0,1 };
		skyboxViewMatrix = DirectX::XMMatrixTranspose(skyboxViewMatrix);
		DirectX::XMMATRIX skyboxVP = projectionMatrix * skyboxViewMatrix;
		DirectX::XMFLOAT4X4 skyboxVPF;
		DirectX::XMStoreFloat4x4(&skyboxVPF, skyboxVP);
		renderer.updateSkyboxConstantBuffer(skyboxVPF);


		renderer.Render(meshManager);

		renderer.present();

    }
    return 0;
}
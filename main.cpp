#include <Windows.h>
#include "Renderer.h"
#include "Window.h"
#include "Timer.h"
#include "GEMLoader.h"
#include "Object.h"
#include <sstream>
#include <string>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
Vertex_Static GEMStaticVertexToStaticVertex(const GEMLoader::GEMStaticVertex& gemVertices) {
	return {
		{ gemVertices.position.x, gemVertices.position.y, gemVertices.position.z },
		{ gemVertices.normal.x, gemVertices.normal.y, gemVertices.normal.z },
		{ gemVertices.tangent.x, gemVertices.tangent.y, gemVertices.tangent.z },
		{ gemVertices.u, gemVertices.v }
	};
}

void loadGEMMesh(std::vector<GEMLoader::GEMMesh>& gemmeshes, MeshManager&meshManager,Mesh &mesh)
{	
	for (auto& gemmesh : gemmeshes)
	{
		//store all of the mesh info in one index buffer and one vertex buffer, 
		//so we need to store the offset of the mesh in the buffer
		if (gemmesh.isAnimated()) {
			mesh.indeicesOffsets = meshManager.indices_Dynamic.size();
			mesh.verticesOffsets = meshManager.vertices_Dynamic.size();
			Vertex_Dynamic v;
			for (auto& vertex : gemmesh.verticesAnimated)
			{
				v.position = { vertex.position.x,vertex.position.y,vertex.position.z };
				v.normal = { vertex.normal.x,vertex.normal.y,vertex.normal.z };
				v.tangent = { vertex.tangent.x,vertex.tangent.y,vertex.tangent.z };
				v.uvCoords = { vertex.u,vertex.v };
				for (int i = 0; i < 4; i++)
				{
					v.bonesIDs[i] = vertex.bonesIDs[i];
					v.boneWeights[i] = vertex.boneWeights[i];
				}
				meshManager.vertices_Dynamic.push_back(v);
			}
			for (auto& index : gemmesh.indices)
			{
				meshManager.indices_Dynamic.push_back(index + static_cast<unsigned int>(mesh.verticesOffsets));
			}
		}
		else
		{
			//store all of the mesh info in one index buffer and one vertex buffer, 
			//so we need to store the offset of the mesh in the buffer
			mesh.indeicesOffsets = meshManager.indices_Static.size();
			mesh.verticesOffsets = meshManager.vertices_Static.size();
			Vertex_Static v;
			for (auto& vertex : gemmesh.verticesStatic)
			{
				v=GEMStaticVertexToStaticVertex(vertex);
				meshManager.vertices_Static.push_back(v);
			}
			for (auto& index : gemmesh.indices)
			{
				meshManager.indices_Static.push_back(index + static_cast<unsigned int>(mesh.verticesOffsets));
			}
		}
	}
}
void loadSkeleton(GEMLoader::GEMAnimation& gemanimation, Animation& animation) {
	for (auto& bone : gemanimation.bones)
	{
		Bone b;
		b.name = bone.name;
		b.bindingOffset = DirectX::XMMATRIX(bone.offset.m[0], bone.offset.m[1], bone.offset.m[2], bone.offset.m[3],
			bone.offset.m[4], bone.offset.m[5], bone.offset.m[6], bone.offset.m[7],
			bone.offset.m[8], bone.offset.m[9], bone.offset.m[10], bone.offset.m[11],
			bone.offset.m[12], bone.offset.m[13], bone.offset.m[14], bone.offset.m[15]);
		b.parentIndex = bone.parentIndex;
		animation.skeleton.bones.push_back(b);
	}
	animation.skeleton.globalInverse = DirectX::XMMATRIX(gemanimation.globalInverse.m[0], gemanimation.globalInverse.m[1], gemanimation.globalInverse.m[2], gemanimation.globalInverse.m[3],
		gemanimation.globalInverse.m[4], gemanimation.globalInverse.m[5], gemanimation.globalInverse.m[6], gemanimation.globalInverse.m[7],
		gemanimation.globalInverse.m[8], gemanimation.globalInverse.m[9], gemanimation.globalInverse.m[10], gemanimation.globalInverse.m[11],
		gemanimation.globalInverse.m[12], gemanimation.globalInverse.m[13], gemanimation.globalInverse.m[14], gemanimation.globalInverse.m[15]);
	
}

void loadAnimation(GEMLoader::GEMAnimation& gemanimation, Animation& animation) {
	for (auto& sequence : gemanimation.animations)
	{
		AnimationSequence seq;
		seq.ticksPerSecond = sequence.ticksPerSecond;
		//store all of the frames in the sequence
		for (auto& frame : sequence.frames)
		{
			//store all of the bone transformation in the frame
			AnimationFrame fr;
			for (int i = 0; i < frame.positions.size(); i++)
			{
				DirectX::XMFLOAT3 position = { frame.positions[i].x,frame.positions[i].y,frame.positions[i].z };
				DirectX::XMFLOAT4 rotation = { frame.rotations[i].q[0],frame.rotations[i].q[1],frame.rotations[i].q[2],frame.rotations[i].q[3] };
				DirectX::XMFLOAT3 scale = { frame.scales[i].x,frame.scales[i].y,frame.scales[i].z };
				fr.position.push_back(position);
				fr.quaternion.push_back(rotation);
				fr.scale.push_back(scale);
			}
			seq.frames.push_back(fr);
		}
		animation.sequences[sequence.name] = seq;
	}
}
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	//initialize the window, timer, renderer, meshManager, loader, gemmeshes, gemanimation
	Window window;
	window.create(800, 600);
	Timer timer;
	Renderer renderer;
	MeshManager meshManager;
	GEMLoader::GEMModelLoader loader;
	std::vector<GEMLoader::GEMMesh> gemmeshes;
	GEMLoader::GEMAnimation gemanimation;

	//load the mesh and animation for NPC
	NPC npc;
	loader.load("Res/TRex.gem", gemmeshes, gemanimation);
	//load meshes
	loadGEMMesh(gemmeshes,meshManager, npc.mesh);
	//load skeleton
	loadSkeleton(gemanimation, npc.animation);
	//load sequences
	loadAnimation(gemanimation, npc.animation);

	

	//update W
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	//DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(timer.time()));
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	DirectX::XMMATRIX planeWorld = translationMatrix * scaleMatrix;
	InstanceData_Dynamic instance;
	DirectX::XMStoreFloat4x4(&instance.W, planeWorld);
	meshManager.instances_Dynamic.push_back(instance);
	

	renderer.Initialize(window, meshManager);
	float dt;
    while (true)
    {
		//return true if the message is WM_QUIT
		if (window.processMessages()) break;
		renderer.cleanFrame();
		//update dt
		dt = timer.dt();

		//update bones
		npc.animationInstance.update("walk", dt);
		meshManager.updateBonesVector(npc.animationInstance.BonesTransforms);
		renderer.updateInstanceBuffer(meshManager.instances_Static, meshManager.instances_Dynamic,meshManager.bonesVector);

		//update V
		DirectX::XMVECTOR eye = DirectX::XMVectorSet(10*cosf(timer.time()), 10.0f, 10*sinf(timer.time()), 0.0f);
		DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixLookAtLH(eye, at, up));
	
		//update P
        constexpr float fov = DirectX::XMConvertToRadians(45.0f);
		float aspectRatio = static_cast<float>(window.width) / static_cast<float>(window.height);
		float nearZ = 0.1f;
		float farZ = 1000.0f;
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ));
		DirectX::XMMATRIX VP = projectionMatrix * viewMatrix;

		DirectX::XMFLOAT4X4 VPF;
		DirectX::XMStoreFloat4x4(&VPF, VP);
		renderer.updateConstantBuffer(true, "cbVS_D", "VP", &VPF, sizeof(VPF));
		renderer.updateConstantBuffer(true, "cbVS_S", "VP", &VPF, sizeof(VPF));

		renderer.Render();

		renderer.present();
    }
	renderer.cleanup();
    return 0;
}
#pragma once
#include <DirectXMath.h> 
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
struct Vertex_Static
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uvCoords;
};
struct InstanceData_General
{
	DirectX::XMFLOAT4X4 W;
	int MaterialIndex;
};
struct Vertex_Dynamic
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uvCoords;
	unsigned int bonesIDs[4];
	float boneWeights[4];
};


void ComputeTangent(DirectX::XMFLOAT3 normal, DirectX::XMFLOAT3& tangent);
Vertex_Static addVertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT2 uvCoords);
void drawACube(std::vector<Vertex_Static>& vertices, std::vector<unsigned int>& indices);
void drawASphere(std::vector<Vertex_Static>& vertices, std::vector<unsigned int>& indices);
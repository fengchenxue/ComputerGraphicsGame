#pragma once

#include <DirectXMath.h> 
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
struct StaticVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uvCoords;
};

class Mesh {
public:

	//std::vector<StaticVertex> vertices;
	//std::vector<unsigned int> indices;
	int indeicesOffsets = 0;
	int verticesOffsets = 0;
};


void ComputeTangent(DirectX::XMFLOAT3 normal, DirectX::XMFLOAT3& tangent);

StaticVertex addVertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT2 uvCoords);

void drawACube(std::vector<StaticVertex>& vertices, std::vector<unsigned int>& indices);

void drawASphere(std::vector<StaticVertex>& vertices, std::vector<unsigned int>& indices);
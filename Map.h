#pragma once
#include "Vertex.h"
#include "Object.h"
#include <vector>
//struct Vertex_Static;
class Object;
class Map
{
private:
	float GetHeight(float x, float z);
	DirectX::XMFLOAT3 GetTerrainNormal(float x, float z);
public:
	int width=0;
	int height=0;
	int channel=0;
	//float scale = 100.0f;

	std::vector<float> heightMap;
	std::vector<DirectX::XMFLOAT3> normals;

	void LoadHeightMap(std::string filename, std::vector<Vertex_Static>& vertices, std::vector<unsigned int>& indices);

	void CheckVerticalCollision_Player(Object& object);
	void CheckVerticalCollision_Object(Object& object);

	bool CanArrive(Object& object,float x,float y);
};
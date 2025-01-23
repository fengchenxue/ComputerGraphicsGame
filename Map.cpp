#include "Map.h"
#include"stb_image.h"
#include<algorithm>
float Map::GetHeight(float x, float z)
{
	int x0 = static_cast<int>(x);
	int z0 = static_cast<int>(z);
	int x1 = x0 + 1;
	int z1 = z0 + 1;

	if (x0 < 0 || x1 >= width || z0 < 0 || z1 >= height)
	{
		return 0.0f;
	}
	float h00 = heightMap[z0 * width + x0];
	float h01 = heightMap[z0 * width + x1];
	float h10 = heightMap[z1 * width + x0];
	float h11 = heightMap[z1 * width + x1];

	float dx = x - static_cast<float>(x0);
	float dz = z - static_cast<float>(z0);

	float h0 = h00 + (h01 - h00) * dx;
	float h1 = h10 + (h11 - h10) * dx;
	float h = h0 + (h1 - h0) * dz;
	return h;
}
DirectX::XMFLOAT3 Map::GetTerrainNormal(float x, float z)
{
	int x0 = static_cast<int>(x);
	int z0 = static_cast<int>(z);
	int x1 = x0 + 1;
	int z1 = z0 + 1;

	if (x0 < 0 || x1 >= width || z0 < 0 || z1 >= height)
	{
		return { 0.0f,1.0f,0.0f };
	}
	DirectX::XMFLOAT3 v00 = normals[z0 * width + x0];
	DirectX::XMFLOAT3 v01 = normals[z0 * width + x1];
	DirectX::XMFLOAT3 v10 = normals[z1 * width + x0];
	DirectX::XMFLOAT3 v11 = normals[z1 * width + x1];

	float dx = x - static_cast<float>(x0);
	float dz = z - static_cast<float>(z0);

	DirectX::XMFLOAT3 v0 = {
		v00.x + (v01.x - v00.x) * dx,
		v00.y + (v01.y - v00.y) * dx,
		v00.z + (v01.z - v00.z) * dx };
	DirectX::XMFLOAT3 v1 = {
		v10.x + (v11.x - v10.x) * dx,
		v10.y + (v11.y - v10.y) * dx,
		v10.z + (v11.z - v10.z) * dx };

	DirectX::XMFLOAT3 v = {
		v0.x + (v1.x - v0.x) * dz,
		v0.y + (v1.y - v0.y) * dz,
		v0.z + (v1.z - v0.z) * dz };

	float l = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	v.x /= l;
	v.y /= l;
	v.z /= l;
	return v;
}
void Map::LoadHeightMap(std::string filename, std::vector<Vertex_Static>& vertices, std::vector<unsigned int>& indices)
{
	size_t vertexOffset = vertices.size();
	size_t indexOffset = indices.size();
	//load the height map
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channel, 0);
	for (int i = 0; i < width * height; i++)
	{
		//set the height of the terrain
		heightMap.push_back((float)data[i]);
	}
	stbi_image_free(data);

	//create the vertices
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float yf = static_cast<float>(y);
			float xf = static_cast<float>(x);

			vertices.push_back(addVertex(DirectX::XMFLOAT3(xf, heightMap[y * width + x], yf), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(xf / static_cast<float>(width - 1), yf / static_cast<float>(height - 1))));
		}
	}

	//create the indices
	for (int y = 0; y < height - 1; y++)
	{
		for (int x = 0; x < width - 1; x++)
		{
			indices.push_back(  y * width + x);
			indices.push_back( (y + 1) * width + x);
			indices.push_back(  y * width + x + 1);

			indices.push_back(  y * width + x + 1);
			indices.push_back( (y + 1) * width + x);
			indices.push_back( (y + 1) * width + x + 1);
		}
	}

	//calculate the normals
	for (int i = vertexOffset; i < vertices.size(); i++) {
		vertices[i].normal = { 0.0f,0.0f,0.0f };
	}
	for (int i = indexOffset; i < indices.size(); i += 3)
	{
		DirectX::XMFLOAT3 v0 = vertices[indices[i]+ vertexOffset].position;
		DirectX::XMFLOAT3 v1 = vertices[indices[i + 1]+ vertexOffset].position;
		DirectX::XMFLOAT3 v2 = vertices[indices[i + 2]+ vertexOffset].position;

		DirectX::XMFLOAT3 edge1 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
		DirectX::XMFLOAT3 edge2 = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
		DirectX::XMFLOAT3 normal = {
			edge1.y * edge2.z - edge1.z * edge2.y,
			edge1.z * edge2.x - edge1.x * edge2.z,
			edge1.x * edge2.y - edge1.y * edge2.x };

		vertices[indices[i]].normal.x += normal.x;
		vertices[indices[i]].normal.y += normal.y;
		vertices[indices[i]].normal.z += normal.z;
		vertices[indices[i + 1]].normal.x += normal.x;
		vertices[indices[i + 1]].normal.y += normal.y;
		vertices[indices[i + 1]].normal.z += normal.z;
		vertices[indices[i + 2]].normal.x += normal.x;
		vertices[indices[i + 2]].normal.y += normal.y;
		vertices[indices[i + 2]].normal.z += normal.z;
	}
	//normalize the normals
	for (int i = vertexOffset; i < vertices.size(); i++) {
		DirectX::XMVECTOR n = DirectX::XMLoadFloat3(&vertices[i].normal);
		n = DirectX::XMVector3Normalize(n);
		DirectX::XMStoreFloat3(&vertices[i].normal, n);
		normals.push_back(vertices[i].normal);
	}
}

void Map::CheckVerticalCollision_Player(Object& object)
{
	object.position.x = std::max(std::min(object.position.x, static_cast<float>(width)), 0.0f);
	object.position.z = std::max(std::min(object.position.z, static_cast<float>(height)), 0.0f);

	//get the collision bound
	DirectX::XMFLOAT3 minBound, maxBound;
	object.getBound(minBound, maxBound);

	float maxH = -FLT_MAX;

	for (float x = minBound.x; x <= maxBound.x; x = x + 1.0f) {
		for (float z = minBound.z; z <= maxBound.z; z = z + 1.0f) {
			float h = GetHeight(x, z);
			if (h > maxH) {
				maxH = h;
			}
		}
	}
	object.position.y = maxH + object.collisionHalfY;
}

void Map::CheckVerticalCollision_Object(Object& object)
{
	//check the collision with the terrain
	DirectX::XMFLOAT3 minBound, maxBound;
	object.getBound(minBound, maxBound);
	float maxH = -FLT_MAX;

	for (float x = minBound.x; x <= maxBound.x; x = x + 1.0f) {
		for (float z = minBound.z; z <= maxBound.z; z = z + 1.0f) {
			float h = GetHeight(x, z);
			if (h > maxH) {
				maxH = h;
			}
		}
	}
	object.position.y = maxH;
}

bool Map::CanArrive(Object& object, float x, float z)
{
	if (x < 0 || x >= width || z < 0 || z >= height)
	{
		return false;
	}
	DirectX::XMFLOAT3 normal = GetTerrainNormal(x, z);
	if (normal.y< 0.7f && GetHeight(x, z)>GetHeight(object.position.x, object.position.z))
	{
		return false;
	}
	return true;

}

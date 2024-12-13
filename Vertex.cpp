#include "Vertex.h"

void ComputeTangent(DirectX::XMFLOAT3 normal, DirectX::XMFLOAT3& tangent)
{
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

Vertex_Static addVertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT2 uvCoords)
{
	Vertex_Static vertex;
	vertex.position = position;
	vertex.normal = normal;
	DirectX::XMFLOAT3 tangent;
	ComputeTangent(normal, tangent);
	vertex.tangent = tangent;
	vertex.uvCoords = uvCoords;
	return vertex;
}

void drawACube(std::vector<Vertex_Static>& vertices, std::vector<unsigned int>& indices)
{
	DirectX::XMFLOAT3 p0 = { -1.0f, -1.0f, -1.0f };
	DirectX::XMFLOAT3 p1 = { 1.0f, -1.0f, -1.0f };
	DirectX::XMFLOAT3 p2 = { 1.0f, 1.0f, -1.0f };
	DirectX::XMFLOAT3 p3 = { -1.0f, 1.0f, -1.0f };
	DirectX::XMFLOAT3 p4 = { -1.0f, -1.0f, 1.0f };
	DirectX::XMFLOAT3 p5 = { 1.0f, -1.0f, 1.0f };
	DirectX::XMFLOAT3 p6 = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 p7 = { -1.0f, 1.0f, 1.0f };

	vertices.push_back(addVertex(p0, DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f)));
	vertices.push_back(addVertex(p1, DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
	vertices.push_back(addVertex(p2, DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 0.0f)));
	vertices.push_back(addVertex(p3, DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 0.0f)));

	vertices.push_back(addVertex(p5, DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f)));
	vertices.push_back(addVertex(p4, DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
	vertices.push_back(addVertex(p7, DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 0.0f)));
	vertices.push_back(addVertex(p6, DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f)));

	vertices.push_back(addVertex(p4, DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f)));
	vertices.push_back(addVertex(p0, DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
	vertices.push_back(addVertex(p3, DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)));
	vertices.push_back(addVertex(p7, DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)));

	vertices.push_back(addVertex(p1, DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f)));
	vertices.push_back(addVertex(p5, DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
	vertices.push_back(addVertex(p6, DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)));
	vertices.push_back(addVertex(p2, DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)));

	vertices.push_back(addVertex(p3, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f)));
	vertices.push_back(addVertex(p2, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
	vertices.push_back(addVertex(p6, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)));
	vertices.push_back(addVertex(p7, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)));

	vertices.push_back(addVertex(p4, DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f)));
	vertices.push_back(addVertex(p5, DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
	vertices.push_back(addVertex(p1, DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)));
	vertices.push_back(addVertex(p0, DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)));
	indices =
	{ 0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10,8, 10,11,
		12,13,14,12,14,15,
		16,17,18,16,18,19,
		20,21,22,20,22,23
	};
}

void drawASphere(std::vector<Vertex_Static>& vertices, std::vector<unsigned int>& indices)
{
	int rings = 100;
	int segments = 100;
	float radius = 1.0f;
	for (int lat = 0; lat <= rings; lat++) {
		float theta = static_cast<float>(lat * M_PI / rings);
		float sinTheta = sinf(theta);
		float cosTheta = cosf(theta);
		for (int lon = 0; lon <= segments; lon++) {
			float phi = static_cast<float>(lon * 2 * M_PI / segments);
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			DirectX::XMFLOAT3 position = { radius * sinTheta * cosPhi, cosTheta * radius, sinPhi * sinTheta * radius };
			DirectX::XMFLOAT3 normal = { sinTheta * cosPhi, cosTheta, sinTheta * sinPhi };

			DirectX::XMFLOAT2 uvCoords = { 1.0f - (float)lon / segments, 1.0f - (float)lat / rings };
			vertices.push_back(addVertex(position, normal, uvCoords));

			int current = lat * (segments + 1) + lon;
			int next = current + segments + 1;

			indices.push_back(current);
			indices.push_back(next);
			indices.push_back(current + 1);

			indices.push_back(current + 1);
			indices.push_back(next);
			indices.push_back(next + 1);
		}
	}
}



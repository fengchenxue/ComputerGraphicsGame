#include "Vertex.h"



DirectX::XMFLOAT3 AnimationSequence::interpolateFloat3(DirectX::XMFLOAT3& p0, DirectX::XMFLOAT3& p1, float interpolationFact)
{
	DirectX::XMVECTOR v0 = DirectX::XMLoadFloat3(&p0);
	DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&p1);
	DirectX::XMVECTOR result = DirectX::XMVectorLerp(v0, v1, interpolationFact);
	DirectX::XMFLOAT3 resultF;
	DirectX::XMStoreFloat3(&resultF, result);
	return resultF;

}

DirectX::XMFLOAT4 AnimationSequence::interpolateQuaternion(DirectX::XMFLOAT4& q0, DirectX::XMFLOAT4& q1, float interpolationFact)
{
	DirectX::XMVECTOR v0 = DirectX::XMLoadFloat4(&q0);
	DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4(&q1);
	DirectX::XMVECTOR result = DirectX::XMQuaternionSlerp(v0, v1, interpolationFact);
	DirectX::XMFLOAT4 resultF;
	DirectX::XMStoreFloat4(&resultF, result);
	return resultF;
}

float AnimationSequence::getDuration()
{
	return static_cast<float>(frames.size()) / ticksPerSecond;
}

void AnimationSequence::calcFrame(float time, int& frame, float& interpolationFact)
{
	interpolationFact = time * ticksPerSecond;
	frame = static_cast<int>(interpolationFact);
	frame = std::max(0,std::min(frame, static_cast<int>(frames.size()) - 1));
	interpolationFact = interpolationFact - static_cast<float>(frame);
	interpolationFact = std::min(interpolationFact, 1.0f);
}

int AnimationSequence::getNextFrame(int frame)
{
	return (frame + 1) % frames.size();
}

DirectX::XMMATRIX AnimationSequence::interpolateBonesToGlobal(std::vector<DirectX::XMFLOAT4X4>& boneTransforms, int baseFrame, float interpolationFact, Skeleton& skeleton, int boneIndex)
{
	AnimationFrame frame1 = frames[baseFrame];
	AnimationFrame frame2 = frames[getNextFrame(baseFrame)];

	DirectX::XMFLOAT3 p=interpolateFloat3(frame1.position[boneIndex], frame2.position[boneIndex], interpolationFact);
	DirectX::XMFLOAT4 q = interpolateQuaternion(frame1.quaternion[boneIndex], frame2.quaternion[boneIndex], interpolationFact);
	DirectX::XMFLOAT3 s = interpolateFloat3(frame1.scale[boneIndex], frame2.scale[boneIndex], interpolationFact);
	
	DirectX::XMMATRIX m = DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(s.x, s.y, s.z));
	m = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(q.x, q.y, q.z, q.w))) * m;
	m = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(p.x, p.y, p.z)) * m;

	if (skeleton.bones[boneIndex].parentIndex > -1) {
		DirectX::XMMATRIX parentMatrix=DirectX::XMLoadFloat4x4 (&boneTransforms[skeleton.bones[boneIndex].parentIndex]);
		m = parentMatrix * m;
	}
	return m;
}


void Animation::calcFrame(std::string sequenceName, float time, int& frame, float& interpolationFact)
{
	sequences[sequenceName].calcFrame(time, frame, interpolationFact);
}

DirectX::XMMATRIX Animation::interpolateBonesToGlobal(std::string name, std::vector<DirectX::XMFLOAT4X4>& boneTransforms, int baseFrame, float interpolationFact, int boneIndex)
{
	return sequences[name].interpolateBonesToGlobal(boneTransforms, baseFrame, interpolationFact, skeleton, boneIndex);
}

void Animation::calcFinalTransformations(std::vector<DirectX::XMFLOAT4X4>& transform)
{
	for (int i = 0; i < skeleton.bones.size(); i++)
	{
		DirectX::XMMATRIX finalTransformation = DirectX::XMLoadFloat4x4(&transform[i]);
		finalTransformation = skeleton.globalInverse* skeleton.bones[i].bindingOffset * finalTransformation;
		DirectX::XMStoreFloat4x4(&transform[i], finalTransformation);
	}
}

void MeshManager::updateBonesVector(std::vector<DirectX::XMFLOAT4X4>& BonesTransforms)
{
	for (int i = 0; i < BonesTransforms.size(); i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				bonesVector[i * 16 + j * 4 + k] = BonesTransforms[i].m[j][k];
			}
		}
	}
}

bool AnimationInstance::animationFinished()
{
	if (time > animation->sequences[sequenceName].getDuration()) return true;
	return false;
}

void AnimationInstance::update(std::string name, float deltaTime)
{
	if (name == sequenceName)
		time += deltaTime;
	else
	{
		sequenceName = name;
		time = 0.0f;
	}
	//if only play once, change here
	if (animationFinished()) time = 0.0f;
	int frame = 0;
	float interpolationFact = 0.0f;
	animation->calcFrame(sequenceName, time, frame, interpolationFact);
	DirectX::XMMATRIX bt;
	for (int i = 0; i < animation->skeleton.bones.size(); i++)
	{
		bt = animation->interpolateBonesToGlobal(sequenceName, BonesTransforms, frame, interpolationFact, i);
		DirectX::XMStoreFloat4x4(&BonesTransforms[i], bt);
	}
	animation->calcFinalTransformations(BonesTransforms);
}


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



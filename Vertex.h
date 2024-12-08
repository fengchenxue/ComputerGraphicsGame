#pragma once

#include <DirectXMath.h> 
#include <vector>
#include <string>
#include<map>
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
struct InstanceData_Static
{
	DirectX::XMFLOAT4X4 W;
};
struct  Vertex_Dynamic
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uvCoords;
	unsigned int bonesIDs[4];
	float boneWeights[4];
};
struct InstanceData_Dynamic
{
	DirectX::XMFLOAT4X4 W;
};
struct Bone
{
	int parentIndex = -1;
	std::string name;
	DirectX::XMMATRIX bindingOffset = DirectX::XMMatrixIdentity();
};
struct Skeleton
{
	std::vector<Bone> bones;
	DirectX::XMMATRIX globalInverse = DirectX::XMMatrixIdentity();

};

struct Mesh {
public:
	size_t indeicesOffsets = 0;
	size_t verticesOffsets = 0;
};

class MeshManager {
public:
	std::vector<Vertex_Static> vertices_Static;
	std::vector<unsigned int> indices_Static;
	std::vector<InstanceData_Static> instances_Static;

	std::vector<Vertex_Dynamic> vertices_Dynamic;
	std::vector<unsigned int> indices_Dynamic;
	std::vector<InstanceData_Dynamic> instances_Dynamic;
	std::vector<float> bonesVector = std::vector<float>(256 * 16 * 10);

	void updateBonesVector(std::vector<DirectX::XMFLOAT4X4> & BonesTransforms);
};

struct AnimationFrame {
	//std::vector<DirectX::XMFLOAT4X4> boneTransforms;
	std::vector<DirectX::XMFLOAT3> position;
	std::vector<DirectX::XMFLOAT4> quaternion;
	std::vector<DirectX::XMFLOAT3> scale;


};

class AnimationSequence {
private:
	DirectX::XMFLOAT3 interpolateFloat3(DirectX::XMFLOAT3& p0, DirectX::XMFLOAT3& p1, float interpolationFact);
	DirectX::XMFLOAT4 interpolateQuaternion(DirectX::XMFLOAT4& q0, DirectX::XMFLOAT4& q1, float interpolationFact);
public:
	std::vector<AnimationFrame> frames;
	float ticksPerSecond = 0.f;

	float getDuration();

	void calcFrame(float time, int& frame, float& interpolationFact);

	int getNextFrame(int frame);

	DirectX::XMMATRIX interpolateBonesToGlobal(std::vector<DirectX::XMFLOAT4X4>& boneTransforms, int baseFrame, float interpolationFact, Skeleton& skeleton, int boneIndex);
};
class Animation {
public:
	std::map<std::string, AnimationSequence> sequences;
	Skeleton skeleton;

	void calcFrame(std::string sequenceName, float time, int& frame, float& interpolationFact);
	DirectX::XMMATRIX interpolateBonesToGlobal(std::string name, std::vector<DirectX::XMFLOAT4X4>& boneTransforms, int baseFrame, float interpolationFact, int boneIndex);
	void calcFinalTransformations(std::vector<DirectX::XMFLOAT4X4>& transform);
};

class AnimationInstance {
public:
	Animation* animation = nullptr;
	float time = 0;
	std::string sequenceName;
	std::vector<DirectX::XMFLOAT4X4> BonesTransforms;

	AnimationInstance() :BonesTransforms(256) {}
	//void resetAnimationTime();
	bool animationFinished();
	void update(std::string name, float deltaTime);
};
void ComputeTangent(DirectX::XMFLOAT3 normal, DirectX::XMFLOAT3& tangent);
Vertex_Static addVertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT2 uvCoords);
void drawACube(std::vector<Vertex_Static>& vertices, std::vector<unsigned int>& indices);
void drawASphere(std::vector<Vertex_Static>& vertices, std::vector<unsigned int>& indices);
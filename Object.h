#pragma once
#include <map>
#include"Window.h"
#include"Map.h"
#include"vertex.h"
#include"GEMLoader.h"

class Map;
class Window;
class ObjectManager;

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

	int getNextFrame(int frame, bool death);

	DirectX::XMMATRIX interpolateBonesToGlobal(std::vector<DirectX::XMFLOAT4X4>& boneTransforms, int baseFrame, float interpolationFact, Skeleton& skeleton, int boneIndex, bool death);
};
class Animation {
public:
	std::map<std::string, AnimationSequence> sequences;
	Skeleton skeleton;

	void calcFrame(std::string sequenceName, float time, int& frame, float& interpolationFact);
	DirectX::XMMATRIX interpolateBonesToGlobal(std::string name, std::vector<DirectX::XMFLOAT4X4>& boneTransforms, int baseFrame, float interpolationFact, int boneIndex,bool death);
	void calcFinalTransformations(std::vector<DirectX::XMFLOAT4X4>& transform);
};

class AnimationInstance {
public:
	Animation* animation = nullptr;
	float time = 0;
	bool deathAnimationFinished = false;
	std::string sequenceName;
	std::vector<DirectX::XMFLOAT4X4> BonesTransforms;

	AnimationInstance() :BonesTransforms(256) {}
	//void resetAnimationTime();
	bool animationFinished();
	void update(std::string name, float deltaTime);
};
struct MaterialProperty {
	std::string name;
	std::string filePath;
};
struct Material {
	std::vector<MaterialProperty> properties;
	MaterialProperty find(std::string name)
	{
		for (int i = 0; i < properties.size(); i++)
		{
			if (properties[i].name == name)
			{
				return properties[i];
			}
		}
		return MaterialProperty();
	}

};
struct MeshDescriptor {
	bool isDynamic = false;
	int vertexOffset = 0;
	int vertexCount = 0;

	int indexOffset = 0;
	int indexCount = 0;

	int instanceOffset = 0;
	int instanceCount = 0;

	std::string textureFile;
};
class Object {
public:
	bool isAlive = true;
	DirectX::XMFLOAT3 position = { 0,0,0 };
	DirectX::XMFLOAT3 rotation = { 0,0,0 };
	DirectX::XMFLOAT3 scale = { 1,1,1 };

	float collisionHalfX = 5.0f;
	float collisionHalfZ = 5.0f;
	float collisionHalfY = 10.0f;

	void getBound(DirectX::XMFLOAT3& minBound, DirectX::XMFLOAT3& maxBound);
	
	void setScaledCollision(float x, float y, float z);

	bool checkCollisionWithPlayer(Object & object);

	bool preCheckCollisionWithPlayer(float x, float z,Object& object);
};
class Character : public Object {
public:
	AnimationInstance animationInstance;

	float speed = 100.0f;
};

class Player : public Character {
public:
	float pitch = 0.0f;
	float yaw = 0.0f;

	DirectX::XMVECTOR forward = { 0,0,1,0 };
	DirectX::XMVECTOR right = { 1,0,0,0 };
	DirectX::XMVECTOR up = { 0,1,0,0 };

	Animation animation;
	Skeleton skeleton;

	Player();

	void updateCamera(float dx, float dy);

	void move(Window & window, float deltaTime,Map&map, ObjectManager& objectManager);
};

class NPC : public Character {
public:
	static Animation animation;
	static Skeleton skeleton;
	NPC();
};

class ObjectManager {
public:
	std::vector<NPC> npcs;
	std::vector<Object> objects;
};

class MeshManager {
private:

	Vertex_Static GEMStaticVertexToStaticVertex(const GEMLoader::GEMStaticVertex& gemVertices);

	void loadGEMMeshAndAnimation(std::vector<GEMLoader::GEMMesh>& gemmeshes, GEMLoader::GEMAnimation& GEManimation, Animation& animation, std::string objectName);
	void loadSkeleton(GEMLoader::GEMAnimation& gemanimation, Animation& animation);
	void loadAnimation(GEMLoader::GEMAnimation& gemanimation, Animation& animation);

	void calculateW(float p1, float p2, float p3, float r1, float r2, float r3, float s1, float s2, float s3, InstanceData_General& instance);
public:
	//type name{Terrain,NPC,Static}
	std::map<std::string, MeshDescriptor> objects;

	std::vector<Vertex_Sky> vertices_Skybox = {
		{DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f)},
		{DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f)},
		{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f)},
		{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f)},

		{DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)},
		{DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f)},
		{DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f)}
	};
	std::vector<unsigned int> indices_Skybox = {
		//front
		0,1,2,0,2,3,
		//back
		4,6,5,4,7,6,
		//left
		4,5,1,4,1,0,
		//right
		3,2,6,3,6,7,
		//top
		4,0,3,4,3,7,
		//bottom
		1,5,6,1,6,2 
	};

	std::vector<Vertex_Static> vertices_Static;
	std::vector<unsigned int> indices_Static;

	std::vector<Vertex_Dynamic> vertices_Dynamic;
	std::vector<unsigned int> indices_Dynamic;

	std::vector<float> bonesVector = std::vector<float>(256 * 16 * 10);

	std::vector<InstanceData_General> instances;
	std::vector<Material> materials;

	void updateBonesVector(std::vector<DirectX::XMFLOAT4X4>& BonesTransforms, int index);

	void loadlevel(std::string& filename, ObjectManager& npcManager, Map& map);


};
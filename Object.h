#pragma once
#include "Vertex.h"

class Object {
public:
	Mesh mesh;
	DirectX::XMFLOAT3 position = { 0,0,0 };
	DirectX::XMFLOAT3 rotation = { 0,0,0 };
	DirectX::XMFLOAT3 scale = { 1,1,1 };
	DirectX::XMVECTOR quaternion = { 0,0,0,1 };

};
class Character : public Object {
public:
	AnimationInstance animationInstance;
};

class Player : public Character {
public:
	Animation animation;
	Skeleton skeleton;
	Player();
};

class NPC : public Character {
public:
	static Animation animation;
	static Skeleton skeleton;
	NPC();
};


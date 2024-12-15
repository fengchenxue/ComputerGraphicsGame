#include "Object.h"
#include<algorithm>
Animation NPC::animation;
Skeleton NPC::skeleton;

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
	frame = std::max(0, std::min(frame, static_cast<int>(frames.size()) - 1));
	interpolationFact = interpolationFact - static_cast<float>(frame);
	interpolationFact = std::min(interpolationFact, 1.0f);
}

int AnimationSequence::getNextFrame(int frame, bool death)
{
	if (death) 
	{ 
		return std::min(frame + 1, static_cast<int>(frames.size()-1));
	}
	return (frame + 1) % frames.size();
}

DirectX::XMMATRIX AnimationSequence::interpolateBonesToGlobal(std::vector<DirectX::XMFLOAT4X4>& boneTransforms, int baseFrame, float interpolationFact, Skeleton& skeleton, int boneIndex, bool death)
{
	AnimationFrame frame1 = frames[baseFrame];
	AnimationFrame frame2 = frames[getNextFrame(baseFrame,death)];

	DirectX::XMFLOAT3 p = interpolateFloat3(frame1.position[boneIndex], frame2.position[boneIndex], interpolationFact);
	DirectX::XMFLOAT4 q = interpolateQuaternion(frame1.quaternion[boneIndex], frame2.quaternion[boneIndex], interpolationFact);
	DirectX::XMFLOAT3 s = interpolateFloat3(frame1.scale[boneIndex], frame2.scale[boneIndex], interpolationFact);

	DirectX::XMMATRIX m = DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(s.x, s.y, s.z));
	m = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(q.x, q.y, q.z, q.w))) * m;
	m = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(p.x, p.y, p.z)) * m;

	if (skeleton.bones[boneIndex].parentIndex > -1) {
		DirectX::XMMATRIX parentMatrix = DirectX::XMLoadFloat4x4(&boneTransforms[skeleton.bones[boneIndex].parentIndex]);
		m = parentMatrix * m;
	}
	return m;
}


void Animation::calcFrame(std::string sequenceName, float time, int& frame, float& interpolationFact)
{
	sequences[sequenceName].calcFrame(time, frame, interpolationFact);
}

DirectX::XMMATRIX Animation::interpolateBonesToGlobal(std::string name, std::vector<DirectX::XMFLOAT4X4>& boneTransforms, int baseFrame, float interpolationFact, int boneIndex,bool death)
{
	return sequences[name].interpolateBonesToGlobal(boneTransforms, baseFrame, interpolationFact, skeleton, boneIndex,death);
}

void Animation::calcFinalTransformations(std::vector<DirectX::XMFLOAT4X4>& transform)
{
	for (int i = 0; i < skeleton.bones.size(); i++)
	{
		DirectX::XMMATRIX finalTransformation = DirectX::XMLoadFloat4x4(&transform[i]);
		//finalTransformation = skeleton.globalInverse* skeleton.bones[i].bindingOffset * finalTransformation;
		finalTransformation = finalTransformation * skeleton.bones[i].bindingOffset * skeleton.globalInverse;
		DirectX::XMStoreFloat4x4(&transform[i], finalTransformation);
	}
}
bool AnimationInstance::animationFinished()
{
	if (time > animation->sequences[sequenceName].getDuration()) return true;
	return false;
}

void AnimationInstance::update(std::string name, float deltaTime)
{
	if (deathAnimationFinished) {
		return;
	}
	if (name == sequenceName)
		time += deltaTime;
	else
	{
		sequenceName = name;
		time = 0.0f;
	}

	if (animationFinished()) {
		if (sequenceName == "death") {
			deathAnimationFinished = true;
			return;
		}
		else {
			time = 0.0f;

		}
	}

	int frame = 0;
	float interpolationFact = 0.0f;
	animation->calcFrame(sequenceName, time, frame, interpolationFact);
	DirectX::XMMATRIX bt;
	for (int i = 0; i < animation->skeleton.bones.size(); i++)
	{
		bt = animation->interpolateBonesToGlobal(sequenceName, BonesTransforms, frame, interpolationFact, i,sequenceName=="death");
		DirectX::XMStoreFloat4x4(&BonesTransforms[i], bt);
	}
	animation->calcFinalTransformations(BonesTransforms);
}

Player::Player()
{
	animationInstance.animation = &animation;
	updateCamera(0, 0);
	position = { 100,0,200 };
}

void Player::updateCamera(float dx, float dy)
{
	pitch += dy;
	yaw += dx;
	if (pitch > DirectX::XM_PIDIV2) pitch = DirectX::XM_PIDIV2;
	if (pitch < -DirectX::XM_PIDIV2) pitch = -DirectX::XM_PIDIV2;

	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0);
	forward = DirectX::XMVector3TransformNormal({ 0,0,1 }, rotationMatrix);
	right = DirectX::XMVector3TransformNormal({ 1,0,0 }, rotationMatrix);
	up = DirectX::XMVector3TransformNormal({ 0,1,0 }, rotationMatrix);
}

void Player::move(Window& window, float deltaTime, Map& map,ObjectManager& objectManager)
{
	float f = 0;
	float r = 0;
	if (window.keys['W']) f = f + 1;
	if (window.keys['S']) f = f - 1;
	if (window.keys['A']) r = r - 1;
	if (window.keys['D']) r = r + 1;

	DirectX::XMVECTOR moveDir =
		DirectX::XMVECTOR{ forward.m128_f32[0] * f + right.m128_f32[0] * r, 0 ,forward.m128_f32[2] * f + right.m128_f32[2] * r, 0 };
	moveDir = DirectX::XMVector3Normalize(moveDir);

	moveDir = DirectX::XMVectorScale(moveDir, speed * deltaTime);
	DirectX::XMFLOAT3 p;
	DirectX::XMStoreFloat3(&p, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position), moveDir));
	if (map.CanArrive(*this, p.x, p.z))
	{
		for (auto & object : objectManager.objects)
		{
			if (object.preCheckCollisionWithPlayer(p.x, p.z, *this))
			{
				return;
			}
		}
		position = p;
	}
}

NPC::NPC()
{
	animationInstance.animation = &animation;
}

void Object::getBound(DirectX::XMFLOAT3& minBound, DirectX::XMFLOAT3& maxBound)
{
	minBound = position;
	minBound.x -= collisionHalfX;
	minBound.y -= collisionHalfY;
	minBound.z -= collisionHalfZ;

	maxBound = position;
	maxBound.x += collisionHalfX;
	maxBound.y += collisionHalfY;
	maxBound.z += collisionHalfZ;
}

void Object::setScaledCollision(float x, float y, float z)
{
	collisionHalfX = x * scale.x;
	collisionHalfY = y * scale.y;
	collisionHalfZ = z * scale.z;
}

bool Object::checkCollisionWithPlayer(Object& object)
{
	float PDx = std::min(position.x + collisionHalfX, object.position.x + object.collisionHalfX) - std::max(position.x - collisionHalfX, object.position.x - object.collisionHalfX);
	float PDy = std::min(position.y + 2*collisionHalfY, object.position.y + object.collisionHalfY) - std::max(position.y , object.position.y - object.collisionHalfY);
	float PDz = std::min(position.z + collisionHalfZ, object.position.z + object.collisionHalfZ) - std::max(position.z - collisionHalfZ, object.position.z - object.collisionHalfZ);
	if (PDx > 0 && PDy > 0 && PDz > 0)
	{
		return true;
	}
	return false;
}

bool Object::preCheckCollisionWithPlayer(float x, float z, Object& object)
{
	float PDx = std::min(position.x + collisionHalfX, x + object.collisionHalfX) - std::max(position.x - collisionHalfX, x - object.collisionHalfX);
	float PDz = std::min(position.z + collisionHalfZ, z + object.collisionHalfZ) - std::max(position.z - collisionHalfZ, z - object.collisionHalfZ);
	if (PDx > 0  && PDz > 0)
	{
		return true;
	}
	return false;
}

Vertex_Static MeshManager::GEMStaticVertexToStaticVertex(const GEMLoader::GEMStaticVertex& gemVertices)
{
	return {
		{ gemVertices.position.x, gemVertices.position.y, gemVertices.position.z },
		{ gemVertices.normal.x, gemVertices.normal.y, gemVertices.normal.z },
		{ gemVertices.tangent.x, gemVertices.tangent.y, gemVertices.tangent.z },
		{ gemVertices.u, gemVertices.v }
	};
}

void MeshManager::loadGEMMeshAndAnimation(std::vector<GEMLoader::GEMMesh>& gemmeshes, GEMLoader::GEMAnimation& GEManimation, Animation& animation, std::string objectName)
{
	if (gemmeshes.size() > 1) {
		std::runtime_error("The object has more than one mesh");
	}
	for (auto& gemmesh : gemmeshes)
	{
		MeshDescriptor md;
		if (gemmesh.isAnimated()) {
			md.isDynamic = true;
			//load the vertices
			md.vertexOffset = vertices_Dynamic.size();
			md.vertexCount = gemmesh.verticesAnimated.size();
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
				vertices_Dynamic.push_back(v);
			}
			//load the indices
			md.indexOffset = indices_Dynamic.size();
			md.indexCount = gemmesh.indices.size();
			for (auto& index : gemmesh.indices)
			{
				indices_Dynamic.push_back(index);
			}
			//load the materials
			Material material;
			for (auto& prop : gemmesh.material.properties) {

				material.properties.push_back({ prop.name, prop.value });
			}
			materials.push_back(material);
			md.textureFile = material.find("diffuse").filePath;
			md.normalMapFile = material.find("normals").filePath;
			//load Skeleton
			loadSkeleton(GEManimation, animation);
			//load Animation
			loadAnimation(GEManimation, animation);
		}
		else
		{
			md.isDynamic = false;
			//load the vertices
			md.vertexOffset = vertices_Static.size();
			md.vertexCount = gemmesh.verticesStatic.size();
			Vertex_Static v;
			for (auto& vertex : gemmesh.verticesStatic)
			{
				v = GEMStaticVertexToStaticVertex(vertex);
				vertices_Static.push_back(v);
			}
			//load the indices
			md.indexOffset = indices_Static.size();
			md.indexCount = gemmesh.indices.size();
			for (auto& index : gemmesh.indices)
			{
				indices_Static.push_back(index);
			}
			//load the materials
			Material material;
			for (auto& prop : gemmesh.material.properties) {

				material.properties.push_back({ prop.name, prop.value });
			}
			materials.push_back(material);
			md.textureFile = material.find("diffuse").filePath;
			md.normalMapFile = material.find("normals").filePath;
		}
		objects[objectName] = md;
	}

}

void MeshManager::loadSkeleton(GEMLoader::GEMAnimation& gemanimation, Animation& animation)
{
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

void MeshManager::loadAnimation(GEMLoader::GEMAnimation& gemanimation, Animation& animation)
{
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
void MeshManager::calculateW(float p1, float p2, float p3, float r1, float r2, float r3, float s1, float s2, float s3, InstanceData_General& instance)
{
	//update W
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(s1, s2, s3));
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationRollPitchYaw(r1, r2, r3));
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(p1, p2, p3));
	DirectX::XMMATRIX planeWorld = translationMatrix * rotationMatrix * scaleMatrix;

	DirectX::XMStoreFloat4x4(&instance.W, planeWorld);
}
void MeshManager::updateBonesVector(std::vector<DirectX::XMFLOAT4X4>& BonesTransforms,int index)
{
	for (int i = 0; i < BonesTransforms.size(); i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				bonesVector[4096*index+i * 16 + j * 4 + k] = BonesTransforms[i].m[j][k];
			}
		}
	}
}

void MeshManager::loadlevel(std::string& filename, ObjectManager &objectManager, Map& map)
{
	int materialIndex = 0;
	GEMLoader::GEMModelLoader loader;
	std::vector<GEMLoader::GEMMesh> gemmeshes;
	GEMLoader::GEMAnimation gemanimation;

	std::ifstream file(filename);
	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		//split the line into tokens
		std::stringstream ss(line);
		std::string segment;
		std::vector<std::string> tokens;
		while (std::getline(ss, segment, ','))
		{
			tokens.push_back(segment);
		}

		//check its type
		if (tokens[0] == "Terrain") {
			MeshDescriptor md;
			md.isDynamic = false;
			md.vertexOffset = vertices_Static.size();
			md.indexOffset = indices_Static.size();
			map.LoadHeightMap(tokens[1], vertices_Static, indices_Static);
			md.vertexCount = vertices_Static.size() - md.vertexOffset;
			md.indexCount = indices_Static.size() - md.indexOffset;
			md.textureFile = tokens[2];

			//update instance
			md.instanceOffset = instances.size();
			md.instanceCount = 1;
			objects["Terrain"] = md;
			InstanceData_General instance;
			calculateW(std::stof(tokens[3]), std::stof(tokens[4]), std::stof(tokens[5]), std::stof(tokens[6]), std::stof(tokens[7]), std::stof(tokens[8]), std::stof(tokens[9]), std::stof(tokens[10]), std::stof(tokens[11]), instance);
			instance.MaterialIndex = materialIndex++;
			instances.push_back(instance);
		}
		else if (tokens[0] == "NPC")
		{
			loader.load(tokens[1], gemmeshes, gemanimation);
			loadGEMMeshAndAnimation(gemmeshes, gemanimation, NPC::animation, "NPC");
			objects["NPC"].instanceOffset = instances.size();
			for (int i = 2; i < tokens.size(); i=i+10)
			{
				//create NPC
				NPC npc;
				npc.position = { std::stof(tokens[i]), std::stof(tokens[i + 1]), std::stof(tokens[i + 2]) };
				npc.rotation = { std::stof(tokens[i + 3]), std::stof(tokens[i + 4]), std::stof(tokens[i + 5]) };
				npc.scale = { std::stof(tokens[i + 6]), std::stof(tokens[i + 7]), std::stof(tokens[i + 8]) };
				npc.setScaledCollision(1.0f, 5.0f, 5.0f);
				map.CheckVerticalCollision_Object(npc);

				//update instance
				objects["NPC"].instanceCount++;
				InstanceData_General instance;
				calculateW(npc.position.x, npc.position.y, npc.position.z, npc.rotation.x, npc.rotation.y, npc.rotation.z, npc.scale.x, npc.scale.y, npc.scale.z, instance);
				instance.MaterialIndex = materialIndex;
				instances.push_back(instance);

				npc.animationInstance.update(tokens[i + 9], 0.0f);
				objectManager.npcs.push_back(npc);
			}
			materialIndex++;
		}
		else if (tokens[0] == "Static")
		{
			Animation animation;

			loader.load(tokens[1], gemmeshes);
			loadGEMMeshAndAnimation(gemmeshes, gemanimation, animation, "Static");
			objects["Static"].instanceOffset = instances.size();
			for (int i = 2; i < tokens.size(); i=i+9)
			{
				//create Static object
				Object s;
				s.position = { std::stof(tokens[i]), std::stof(tokens[i + 1]), std::stof(tokens[i + 2]) };
				s.rotation = { std::stof(tokens[i + 3]), std::stof(tokens[i + 4]), std::stof(tokens[i + 5]) };
				s.scale = { std::stof(tokens[i + 6]), std::stof(tokens[i + 7]), std::stof(tokens[i + 8]) };
				s.setScaledCollision(25.0f, 45.0f, 25.0f);
				map.CheckVerticalCollision_Object(s);

				//update instance
				objects["Static"].instanceCount++;
				InstanceData_General instance;
				calculateW(s.position.x, s.position.y, s.position.z, s.rotation.x, s.rotation.y, s.rotation.z, s.scale.x, s.scale.y, s.scale.z, instance);
				instance.MaterialIndex = materialIndex;
				instances.push_back(instance);

				objectManager.objects.push_back(s);
			}
			materialIndex++;

		}
	}
}


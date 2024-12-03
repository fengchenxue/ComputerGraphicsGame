//#pragma once
//
//#include<fbxsdk.h>
//#include<vector>
//
//
//#pragma comment(lib,"libfbxsdk.lib")
//#pragma comment(lib,"libxml2.lib")
//#pragma comment(lib,"zlib.lib")
//
//struct MeshData {
//	std::vector<FbxVector4> vertices;
//	std::vector<FbxVector4> normals;
//	std::vector<FbxVector2> uvCoords;
//	std::vector<unsigned int> indices;
//};
//
//class FBXLoader {
//private:
//	FbxManager* manager;
//	FbxScene* scene;
//	MeshData meshData;
//
//	void processNode(FbxNode* node);
//	void processMesh(FbxMesh* mesh);
//public:
//	FBXLoader();
//	~FBXLoader();
//	void loadMesh(char* filename);
//};
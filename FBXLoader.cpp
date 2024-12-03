//#include "FBXLoader.h"
//void FBXLoader::processNode(FbxNode* node)
//{
//	if (!node) return;
//	FbxMesh* mesh = node->GetMesh();
//	//extract mesh data
//	if (mesh) processMesh(mesh);
//	//recursively traverse all children
//	for (int i = 0; i < node->GetChildCount(); i++)
//	{
//		processNode(node->GetChild(i));
//	}
//}
//void FBXLoader::processMesh(FbxMesh* mesh)
//{
//	//extract vertex data
//	int controlPointsCount = mesh->GetControlPointsCount();
//	FbxVector4* controlPoints = mesh->GetControlPoints();
//	for (int i = 0; i < controlPointsCount; i++)
//	{
//		meshData.vertices.push_back(controlPoints[i]);
//	}
//
//	//extract index data
//	int polygonCount = mesh->GetPolygonCount();
//	for (int i = 0; i < polygonCount; i++)
//	{
//		//
//		int polygonSize = mesh->GetPolygonSize(i);
//		for (int j = 0; j < polygonSize; j++)
//		{
//			meshData.indices.push_back(mesh->GetPolygonVertex(i, j));
//		}
//	}
//}
//FBXLoader::FBXLoader()
//{
//	//create the manager to manage memory
//	manager = FbxManager::Create();
//	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
//	manager->SetIOSettings(ios);
//	//create the scene to hold the imported data
//	scene = FbxScene::Create(manager, "myScene");
//
//	ios->Destroy();
//}
//
//FBXLoader::~FBXLoader()
//{
//	if (manager) manager->Destroy();
//	if (scene) scene->Destroy();
//}
//
//void FBXLoader::loadMesh(char* filename)
//{
//	//create the importer to import the data
//	FbxImporter* importer = FbxImporter::Create(manager, "");
//	importer->Initialize(filename, -1, manager->GetIOSettings());
//	//import the data to the scene
//	importer->Import(scene);
//	importer->Destroy();
//	//recursively traverse all nodes to extract mesh data
//	processNode(scene->GetRootNode());
//}
//

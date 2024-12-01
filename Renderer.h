#pragma once
#include "Window.h"
#include "Vertex.h"
#include<d3d11.h>
#include<wrl.h>
#include<d3d11shader.h>
#include<map>
#include<vector>

class Renderer {
private:
	
	struct ConstantBufferOffsetInfo
	{
		unsigned int offset;
		unsigned int size;
	};
	
	struct ConstantBufferManager_single
	{
		bool dirty = false;
		std::vector<unsigned char> temporaryBufferData;
		std::string name;
		std::map<std::string, ConstantBufferOffsetInfo>  constantBufferVariableInfoMap;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
	};
	//used to store reflection constant buffer data
	std::vector<ConstantBufferManager_single> VSconstantBufferManager_collection;
	std::vector<ConstantBufferManager_single> PSconstantBufferManager_collection;

	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backbufferRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthbuffer;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	int indicesSize=0;
	UINT stride=0;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	void InitializeDeviceAndContext(Window& window);
	void InitializeRenderTarget(Window&window);
	void InitializeShadersAndConstantBuffer();
	void initializeIndexAndVertexBuffer(void *vertics, int vertexSizeInBytes,int numVertics,unsigned int *indices, int numIndics);
	void initializeIndexAndVertexBuffer(std::vector<StaticVertex> vertices,std::vector<unsigned int> indices );
	void BindIndexAndVertexBuffer();

	void updateConstantBufferManager();

public:
	void Initialize(Window& window, std::vector<StaticVertex> vertices, std::vector<unsigned int> indices);

	//call if you want to change variable in constant buffer
	void updateConstantBuffer(bool VSBuffer,std::string bufferName, std::string variableName, void* data, size_t dataSize);

	//call every frame
	void Render();

	//call every frame
	void cleanFrame();

	//call at the end of the game
	void cleanup();

	//call every frame
	void present();


};
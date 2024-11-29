#pragma once
#include "Window.h"
#include<d3d11.h>
#include<wrl.h>
#include<DirectXMath.h>
#include<d3d11shader.h>
#include<map>
#include<vector>

class Renderer {
private:
	
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

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
	std::vector<ConstantBufferManager_single> constantBufferManager_collection;

	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backbufferRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthbuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
	//std::map<std::string, Microsoft::WRL::ComPtr<ID3D11Buffer>> constantBufferMap;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	void CreateVertexBuffer();
	void CreateShaders();
	
public:
	void Initialize(Window& window);
	//call if you want to change variable in constant buffer
	void updateConstantBuffer(std::string bufferName, std::string variableName, void* data, size_t dataSize);
	//call every frame
	void Render();
	//call every frame
	void cleanFrame();
	//call at the end of the game
	void cleanup();
	//call every frame
	void present();


};
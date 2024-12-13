#pragma once
#include "Window.h"
#include "Vertex.h"
#include <d3d11.h>
#include <wrl.h>
#include <d3d11shader.h>
#include <map>
#include "stb_image.h"

class Renderer {
private:
	
	struct ConstantBufferOffsetInfo
	{
		unsigned int offset;
		unsigned int size;
	};
	
	struct ConstantBufferManager
	{
		bool dirty = false;
		std::vector<unsigned char> temporaryBufferData;
		std::string name;
		std::map<std::string, ConstantBufferOffsetInfo>  constantBufferVariableInfoMap;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
	};
	//used to store reflection constant buffer data
	std::vector<ConstantBufferManager> VSconstantBufferManager_collection;
	std::vector<ConstantBufferManager> PSconstantBufferManager_collection;

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

	//two shaders, one for static objects and the other for dynamic objects
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_Static;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_Static;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_Static;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_Static;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_Dynamic;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_Dynamic;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_Dynamic;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_Dynamic;
	//used in transferring bone data to GPU
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bonesSRV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> bonesTexture;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_General;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> VSstructuredSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> VSstructuredBuffer;

	//used in transferring texture data to GPU
	std::map<std::string, int> textureBindPoints;
	struct TextureData {
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	};
	std::map<std::string, TextureData> textureMap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

	void InitializeDeviceAndContext(Window& window);
	void InitializeRenderTarget(Window&window);
	void InitializeShadersAndConstantBuffer();
	void InitializeConstantBuffer(Microsoft::WRL::ComPtr<ID3DBlob> vsBlob_S, Microsoft::WRL::ComPtr<ID3DBlob> vsBlob_D, Microsoft::WRL::ComPtr<ID3DBlob> psBlob);
	void initializeIndexAndVertexBuffer(std::vector<Vertex_Static> &vertices_Static,std::vector<unsigned int>& indices_Static, std::vector<Vertex_Dynamic> &vertices_dynamic, std::vector<unsigned int> &indices_dynamic);
	void InitializeStructuredBuffer();
	void InitializeState();
	void InitializeSampler();

	void SwitchShader(bool _static);
	void updateConstantBufferManager();

public:
	void Initialize(Window& window, MeshManager& meshmanager);

	//call if you want to change variable in constant buffer
	void updateConstantBuffer(bool VSBuffer,std::string bufferName, std::string variableName, void* data, size_t dataSize);

	void updateInstanceBuffer(MeshManager &meshmanager, int mode);

	void updataBonesBuffer(std::vector<float>& bonesVector);
	//call every frame
	void Render(MeshManager & meshManager);

	//call every frame
	void cleanFrame();

	//call at the end of the game
	void cleanup();

	//call every frame
	void present();

	void loadTexture(std::string filename, std::string textureShaderName);

};
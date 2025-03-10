#include "Renderer.h"
#include <d3dcompiler.h>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

void Renderer::InitializeDeviceAndContext(Window& window)
{
	//find the best adapter
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapterf;
	std::vector<IDXGIAdapter1*> adapters;
	Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)factory.GetAddressOf());
	for (int i = 0; factory->EnumAdapters1(i, adapterf.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(adapterf.Get());
	}
	size_t maxVideoMemory = 0;
	for (auto local_adapter : adapters)
	{
		DXGI_ADAPTER_DESC1 desc;
		local_adapter->GetDesc1(&desc);
		if (desc.DedicatedVideoMemory > maxVideoMemory)
		{
			maxVideoMemory = desc.DedicatedVideoMemory;
			adapter = local_adapter;
		}
	}

	//Create configuration for SwapChain
	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount = 1;
	scd.BufferDesc.Width = window.width;
	scd.BufferDesc.Height = window.height;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = window.hwnd;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = window.fullscreen ? FALSE : TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Create Device, SwapChain and DeviceContext according to DXGI_SWAP_CHAIN_DESC
	D3D11CreateDeviceAndSwapChain(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, D3D11_CREATE_DEVICE_DEBUG,
		NULL, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, NULL, &context);

	swapChain->SetFullscreenState(window.fullscreen, NULL);
}

void Renderer::InitializeRenderTarget(Window& window)
{
	//get the back buffer and its render target view
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());
	HRESULT hr = device->CreateRenderTargetView(backbuffer.Get(), NULL, backbufferRenderTargetView.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create render target view", L"Error", MB_OK);
		exit(0);
	}
	//create configuration for depth buffer
	D3D11_TEXTURE2D_DESC dbd = {};
	dbd.Width = window.width;
	dbd.Height = window.height;
	dbd.MipLevels = 1;
	dbd.ArraySize = 1;
	dbd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dbd.SampleDesc.Count = 1;
	dbd.SampleDesc.Quality = 0;
	dbd.Usage = D3D11_USAGE_DEFAULT;
	dbd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dbd.CPUAccessFlags = 0;
	dbd.MiscFlags = 0;

	//create depth buffer and its depth stencil view
	device->CreateTexture2D(&dbd, NULL, depthbuffer.GetAddressOf());
	device->CreateDepthStencilView(depthbuffer.Get(), NULL, depthStencilView.GetAddressOf());

	//send command to GPU for rendering
	context->OMSetRenderTargets(1, backbufferRenderTargetView.GetAddressOf(), depthStencilView.Get());
	if (!backbufferRenderTargetView || !depthStencilView)
	{
		MessageBox(NULL, L"Failed to set render target view", L"Error", MB_OK);
		exit(0);
	}
	//create configuration for Viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(window.width);
	viewport.Height = static_cast<float>(window.height);
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	//apply the configuration to the context
	context->RSSetViewports(1, &viewport);

}

void Renderer::InitializeShadersAndConstantBuffer()
{
	//-----------static shaders----------------//
	//compile vertex shader and store it in vsBlob,then create vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob_S;
	HRESULT hr = D3DCompileFromFile(L"VertexShader_Static.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, vsBlob_S.GetAddressOf(), nullptr);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to compile VertexShader_Static.hlsl", L"Shader Error", MB_OK);
	}
	device->CreateVertexShader(vsBlob_S->GetBufferPointer(), vsBlob_S->GetBufferSize(), nullptr, &vertexShader_Static);

	//create input layout
	D3D11_INPUT_ELEMENT_DESC layout_S[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	device->CreateInputLayout(layout_S, _countof(layout_S), vsBlob_S->GetBufferPointer(), vsBlob_S->GetBufferSize(), &inputLayout_Static);

	//compile pixel shader and store it in psBlob,then create pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	hr=D3DCompileFromFile(L"PixelShader_Texture.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, psBlob.GetAddressOf(), nullptr);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to compile PixelShader_Static.hlsl", L"Shader Error", MB_OK);
	}
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader_General);

	//-----------dynamic shaders----------------//
	//compile vertex shader and store it in vsBlob,then create vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob_D;
	hr=D3DCompileFromFile(L"VertexShader_Dynamic.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, vsBlob_D.GetAddressOf(), nullptr);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to compile VertexShader_Dynamic.hlsl", L"Shader Error", MB_OK);
	}
	device->CreateVertexShader(vsBlob_D->GetBufferPointer(), vsBlob_D->GetBufferSize(), nullptr, &vertexShader_Dynamic);

	//create input layout
	D3D11_INPUT_ELEMENT_DESC layout_D[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"BONEIDS",0,DXGI_FORMAT_R32G32B32A32_UINT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"BONEWEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	device->CreateInputLayout(layout_D, _countof(layout_D), vsBlob_D->GetBufferPointer(), vsBlob_D->GetBufferSize(), &inputLayout_Dynamic);

	//create constant buffer
	InitializeConstantBuffer(vsBlob_S, vsBlob_D, psBlob);
}

void Renderer::InitializeConstantBuffer(Microsoft::WRL::ComPtr<ID3DBlob> vsBlob_S, Microsoft::WRL::ComPtr<ID3DBlob> vsBlob_D, Microsoft::WRL::ComPtr<ID3DBlob> psBlob)
{
	//--------- VS_S constant buffer creation----------------//
	//create shader reflection from the vertex shader
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> VSreflection;
	D3DReflect(vsBlob_S->GetBufferPointer(), vsBlob_S->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)VSreflection.GetAddressOf());
	//get the shader description 
	D3D11_SHADER_DESC sdc;
	VSreflection->GetDesc(&sdc);
	for (UINT i = 0; i < sdc.ConstantBuffers; i++) {
		//get the constant buffer
		ID3D11ShaderReflectionConstantBuffer* reflectionConstantBuffer = VSreflection->GetConstantBufferByIndex(i);
		//get the constant buffer description
		D3D11_SHADER_BUFFER_DESC sbd;
		reflectionConstantBuffer->GetDesc(&sbd);

		//create a constant buffer manager for each constant buffer
		ConstantBufferManager constantBufferManager;
		constantBufferManager.name = sbd.Name;

		//Store total size of the constant buffer
		UINT totalSize = 0;

		//iterate through all the variables in the constant buffer
		for (UINT j = 0; j < sbd.Variables; j++) {
			ID3D11ShaderReflectionVariable* variable = reflectionConstantBuffer->GetVariableByIndex(j);

			D3D11_SHADER_VARIABLE_DESC svd;
			variable->GetDesc(&svd);

			ConstantBufferOffsetInfo bufferVariable = {};
			bufferVariable.offset = svd.StartOffset;
			bufferVariable.size = svd.Size;

			constantBufferManager.constantBufferVariableInfoMap.insert({ svd.Name,bufferVariable });

			totalSize = std::max(totalSize, svd.StartOffset + svd.Size);
		}
		//align the size of the constant buffer to 16 bytes
		UINT allignedSize = (totalSize + 15) & ~15;
		//resize the temporary buffer data to the size of the constant buffer
		constantBufferManager.temporaryBufferData.resize(allignedSize, 0);

		//create constant buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.ByteWidth = allignedSize;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
		device->CreateBuffer(&bd, nullptr, constantBuffer.GetAddressOf());
		constantBufferManager.constantBuffer = constantBuffer;
		//store the constant buffer manager in the vector
		VSconstantBufferManager_collection.push_back(constantBufferManager);

		context->VSSetConstantBuffers(i, 1, constantBuffer.GetAddressOf());
	}
	
	//--------- VS_D constant buffer creation----------------//
	D3DReflect(vsBlob_D->GetBufferPointer(), vsBlob_D->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)VSreflection.GetAddressOf());
	//get the shader description 
	//D3D11_SHADER_DESC sdc;
	size_t offset = VSconstantBufferManager_collection.size();
	VSreflection->GetDesc(&sdc);
	for (UINT i = 0; i < sdc.ConstantBuffers; i++) {
		//get the constant buffer
		ID3D11ShaderReflectionConstantBuffer* reflectionConstantBuffer = VSreflection->GetConstantBufferByIndex(i);
		//get the constant buffer description
		D3D11_SHADER_BUFFER_DESC sbd;
		reflectionConstantBuffer->GetDesc(&sbd);

		//create a constant buffer manager for each constant buffer
		ConstantBufferManager constantBufferManager;
		constantBufferManager.name = sbd.Name;

		//Store total size of the constant buffer
		UINT totalSize = 0;

		//iterate through all the variables in the constant buffer
		for (UINT j = 0; j < sbd.Variables; j++) {
			ID3D11ShaderReflectionVariable* variable = reflectionConstantBuffer->GetVariableByIndex(j);

			D3D11_SHADER_VARIABLE_DESC svd;
			variable->GetDesc(&svd);

			ConstantBufferOffsetInfo bufferVariable = {};
			bufferVariable.offset = svd.StartOffset;
			bufferVariable.size = svd.Size;

			constantBufferManager.constantBufferVariableInfoMap.insert({ svd.Name,bufferVariable });

			totalSize = std::max(totalSize, svd.StartOffset + svd.Size);
		}
		//align the size of the constant buffer to 16 bytes
		UINT allignedSize = (totalSize + 15) & ~15;
		//resize the temporary buffer data to the size of the constant buffer
		constantBufferManager.temporaryBufferData.resize(allignedSize, 0);

		//create constant buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.ByteWidth = allignedSize;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
		device->CreateBuffer(&bd, nullptr, constantBuffer.GetAddressOf());
		constantBufferManager.constantBuffer = constantBuffer;
		//store the constant buffer manager in the vector
		VSconstantBufferManager_collection.push_back(constantBufferManager);

		context->VSSetConstantBuffers(i + offset, 1, constantBuffer.GetAddressOf());
	}
	//---------PS constant buffer creation----------------//
	//create shader reflection from the pixel shader
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> PSreflection;
	D3DReflect(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)PSreflection.GetAddressOf());
	//get the shader description 
	//D3D11_SHADER_DESC sdc;
	PSreflection->GetDesc(&sdc);
	for (UINT i = 0; i < sdc.ConstantBuffers; i++) {
		//get the constant buffer
		ID3D11ShaderReflectionConstantBuffer* reflectionConstantBuffer = PSreflection->GetConstantBufferByIndex(i);
		//get the constant buffer description
		D3D11_SHADER_BUFFER_DESC sbd;
		reflectionConstantBuffer->GetDesc(&sbd);

		//create a constant buffer manager for each constant buffer
		ConstantBufferManager constantBufferManager;
		constantBufferManager.name = sbd.Name;

		//Store total size of the constant buffer
		UINT totalSize = 0;

		//iterate through all the variables in the constant buffer
		for (UINT j = 0; j < sbd.Variables; j++) {
			ID3D11ShaderReflectionVariable* variable = reflectionConstantBuffer->GetVariableByIndex(j);

			D3D11_SHADER_VARIABLE_DESC svd;
			variable->GetDesc(&svd);

			ConstantBufferOffsetInfo bufferVariable = {};
			bufferVariable.offset = svd.StartOffset;
			bufferVariable.size = svd.Size;

			constantBufferManager.constantBufferVariableInfoMap.insert({ svd.Name,bufferVariable });

			totalSize = std::max(totalSize, svd.StartOffset + svd.Size);
		}
		//align the size of the constant buffer to 16 bytes
		UINT allignedSize = (totalSize + 15) & ~15;
		//resize the temporary buffer data to the size of the constant buffer
		constantBufferManager.temporaryBufferData.resize(allignedSize, 0);

		//create constant buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.ByteWidth = allignedSize;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
		device->CreateBuffer(&bd, nullptr, constantBuffer.GetAddressOf());
		constantBufferManager.constantBuffer = constantBuffer;
		//store the constant buffer manager in the vector
		PSconstantBufferManager_collection.push_back(constantBufferManager);

		context->PSSetConstantBuffers(i, 1, constantBuffer.GetAddressOf());
	}
	

	//--------get the shader reflection from the pixel shader, for texture binding points--------//
	for (UINT i = 0; i < sdc.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		PSreflection->GetResourceBindingDesc(i, &bindDesc);
		if (bindDesc.Type == D3D_SIT_TEXTURE)
		{
			textureBindPoints.insert({ bindDesc.Name,bindDesc.BindPoint });
		}
	}
}

void Renderer::initializeIndexAndVertexBuffer(std::vector<Vertex_Static>& vertices_Static, std::vector<unsigned int>& indices_Static, std::vector<Vertex_Dynamic>& vertices_dynamic, std::vector<unsigned int>& indices_dynamic)
{
	//-------initialize the static buffer----------------//
	int vertexSizeInBytes = sizeof(Vertex_Static);
	size_t numVertics = vertices_Static.size();
	UINT numIndics = static_cast<UINT>(indices_Static.size());
	//indicesSize_Static = numIndics;
	//create static index buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * numIndics);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = indices_Static.data();
	device->CreateBuffer(&bd, &initData, &indexBuffer_Static);

	//create static vertex buffer
	bd.ByteWidth = static_cast<UINT>(vertexSizeInBytes * numVertics);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	initData.pSysMem = vertices_Static.data();
	device->CreateBuffer(&bd, &initData, &vertexBuffer_Static);


	//-------initialize the dynamic buffer----------------//
	vertexSizeInBytes = sizeof(Vertex_Dynamic);
	numVertics = vertices_dynamic.size();
	numIndics = static_cast<UINT>(indices_dynamic.size());
	//indicesSize_Dynamic = numIndics;
	//create dynamic index buffer
	bd.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * numIndics);
	initData.pSysMem = indices_dynamic.data();
	HRESULT hr=device->CreateBuffer(&bd, &initData, &indexBuffer_Dynamic);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to compile dynamic buffer.hlsl", L"Shader Error", MB_OK);
	}
	//create dynamic vertex buffer
	bd.ByteWidth = static_cast<UINT>(vertexSizeInBytes * numVertics);
	initData.pSysMem = vertices_dynamic.data();
	hr=device->CreateBuffer(&bd, &initData, &vertexBuffer_Dynamic);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to create dynamic buffer", L"Shader Error", MB_OK);
	}
}

void Renderer::InitializeStructuredBuffer()
{
	//custom buffer size, you can change it
	UINT bufferSize = 20;
	//initialize the instance buffer
	D3D11_BUFFER_DESC id = {};
	id.Usage = D3D11_USAGE_DYNAMIC;
	id.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	id.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	id.ByteWidth = sizeof(InstanceData_General) * bufferSize;
	id.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	id.StructureByteStride = sizeof(InstanceData_General);
	device->CreateBuffer(&id, nullptr, &VSstructuredBuffer);

	//create shader resource view for the instance buffer
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = DXGI_FORMAT_UNKNOWN;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvd.Buffer.FirstElement = 0;
	srvd.Buffer.NumElements = bufferSize;
	device->CreateShaderResourceView(VSstructuredBuffer.Get(), &srvd, VSstructuredSRV.GetAddressOf());

	//bind the instance buffer to the vertex shader
	context->VSSetShaderResources(0, 1, VSstructuredSRV.GetAddressOf());


	//create texture to store the bones data
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = 256 * 4;
	td.Height = 10;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DYNAMIC;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateTexture2D(&td, nullptr, bonesTexture.GetAddressOf());

	//create shader resource view for the bones texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd2 = {};
	srvd2.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvd2.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd2.Texture2D.MostDetailedMip = 0;
	srvd2.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(bonesTexture.Get(), &srvd2, bonesSRV.GetAddressOf());
	
	//bind the bones data to the vertex shader
	context->VSSetShaderResources(2, 1, bonesSRV.GetAddressOf());
}

void Renderer::InitializeState()
{
	//create configuration for rasterizer state
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&rd, rasterizerState.GetAddressOf());
	//apply the configuration to the context
	context->RSSetState(rasterizerState.Get());

	//create configuration for depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = false;
	//create depth stencil state and bind it to output merger stage
	device->CreateDepthStencilState(&dsd, depthStencilState.GetAddressOf());
	context->OMSetDepthStencilState(depthStencilState.Get(), 0);

	////create configuration for blend state
	//D3D11_BLEND_DESC bd = {};
	//device->CreateBlendState(&bd, blendState.GetAddressOf());
	////apply the configuration to the context
	//context->OMSetBlendState(blendState.Get(), 0, 0xffffffff);
}

void Renderer::InitializeSampler()
{
	//create configuration for sampler
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	//create sampler and bind it to pixel shader
	device->CreateSamplerState(&sd, sampler.GetAddressOf());
	context->PSSetSamplers(0, 1, sampler.GetAddressOf());
}

void Renderer::InitializeSkybox(std::vector<Vertex_Sky>& vertices_Sky, std::vector<unsigned int>& indices_Sky)
{
	//create skybox vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex_Sky) * vertices_Sky.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices_Sky.data();
	device->CreateBuffer(&bd, &initData, skyVertexBuffer.GetAddressOf());

	//create skybox index buffer
	bd.ByteWidth = sizeof(unsigned int) * indices_Sky.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	initData.pSysMem = indices_Sky.data();
	device->CreateBuffer(&bd, &initData, skyIndexBuffer.GetAddressOf());

	//load skybox texture
	std::string skyboxTexturePath[6]={
		"Res/px.png",
		"Res/nx.png",
		"Res/py.png",
		"Res/ny.png",
		"Res/pz.png",
		"Res/nz.png"
	};
	unsigned char* skyboxTextureData[6];
	int width, height, channel;
	for (int i = 0; i < 6; i++)
	{
		skyboxTextureData[i] = stbi_load(skyboxTexturePath[i].c_str(), &width, &height, &channel, STBI_rgb_alpha);
	}
	//create skybox texture description
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = width;
	td.Height = height;
	td.MipLevels = 1;
	td.ArraySize = 6;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	td.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	//create subresource data for each face of the skybox texture
	D3D11_SUBRESOURCE_DATA subResourceData[6];
	for (int i = 0; i < 6; i++)
	{
		subResourceData[i].pSysMem = skyboxTextureData[i];
		subResourceData[i].SysMemPitch = width * 4;
		subResourceData[i].SysMemSlicePitch = 0;
	}
	//create skybox texture
	HRESULT hr = device->CreateTexture2D(&td, subResourceData, skyTexture.GetAddressOf());
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to create Shader Resource View for skyTexture", L"Error", MB_OK);
	}

	//create shader resource view for the skybox texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvd.TextureCube.MipLevels = 1;
	srvd.TextureCube.MostDetailedMip = 0;
	device->CreateShaderResourceView(skyTexture.Get(), &srvd, skySRV.GetAddressOf());
	context->PSSetShaderResources(3, 1, skySRV.GetAddressOf());

	for(int i=0;i<6;i++){
		stbi_image_free(skyboxTextureData[i]);
	}


	//compile skybox vertex shader and store it in vsBlob,then create vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
	D3DCompileFromFile(L"VertexShader_Sky.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), nullptr);
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &skyVertexShader);

	//create input layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	device->CreateInputLayout(layout, _countof(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &skyInputLayout);

	//compile skybox pixel shader and store it in psBlob,then create pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	D3DCompileFromFile(L"PixelShader_Sky.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, psBlob.GetAddressOf(), nullptr);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &skyPixelShader);

	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsd, skyDepthStencilState.GetAddressOf());

	//create samplerState
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sd, skySampler.GetAddressOf());
	context->PSSetSamplers(1, 1, skySampler.GetAddressOf());

	//create rasterizer state
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&rd, skyRasterizerState.GetAddressOf());
	//context->RSSetState(skyRasterizerState.Get());

	//create constant buffer for skybox
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&bd, nullptr, skyConstantBuffer.GetAddressOf());
	context->VSSetConstantBuffers(4, 1, skyConstantBuffer.GetAddressOf());

}

void Renderer::InitializeGBuffer(Window& window)
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = window.width;
	texDesc.Height = window.height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	for (int i = 0; i < 3; i++) {
		if (i == 0)
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		else if (i == 1)
			texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		else if (i == 2)
			texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		device->CreateTexture2D(&texDesc, nullptr, gBufferTextures[i].GetAddressOf());
		device->CreateRenderTargetView(gBufferTextures[i].Get(), nullptr, gBufferRTVs[i].GetAddressOf());
		device->CreateShaderResourceView(gBufferTextures[i].Get(), nullptr, gBufferSRVs[i].GetAddressOf());
	}

	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
	D3DCompileFromFile(L"VertexShader_GBuffer.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), nullptr);
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, gBufferVertexShader.GetAddressOf());


	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	D3DCompileFromFile(L"PixelShader_GBuffer.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, psBlob.GetAddressOf(), nullptr);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, gBufferPixelShader.GetAddressOf());

}

void Renderer::InitializeLightingBuffer()
{
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
	D3DCompileFromFile(L"VertexShader_Lighting.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), nullptr);
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &lightingVertexShader);

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	device->CreateInputLayout(layout, _countof(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &lightingInputLayout);

	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	D3DCompileFromFile(L"PixelShader_Lighting.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, psBlob.GetAddressOf(), nullptr);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &lightingPixelShader);

	//create constant buffer for Lighting
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(lightingConstants);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&bd, nullptr, lightingConstantBuffer.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, lightingConstantBuffer.GetAddressOf());

	//create vertex buffer for lighting
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(DirectX::XMFLOAT3) * fullscreenVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData = {};
	vinitData.pSysMem = fullscreenVertices.data();
	device->CreateBuffer(&vbd, &vinitData, lightingVertexBuffer.GetAddressOf());

	//create depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsd, lightingDepthStencilState.GetAddressOf());
}


void Renderer::SwitchShader(int mode)
{
	if (mode == 0) {
		//set the input layout
		context->IASetInputLayout(skyInputLayout.Get());
		//set the shaders
		context->VSSetShader(skyVertexShader.Get(), NULL, 0);
		context->PSSetShader(skyPixelShader.Get(), NULL, 0);
		//set the vertex and index buffer
		UINT stride = sizeof(Vertex_Sky);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, skyVertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(skyIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}
	else if (mode==1) {
		//set the input layout
		context->IASetInputLayout(inputLayout_Static.Get());
		//set the shaders
		context->VSSetShader(vertexShader_Static.Get(), NULL, 0);
		context->PSSetShader(pixelShader_General.Get(), NULL, 0);
		//set the vertex and index buffer
		UINT stride = sizeof(Vertex_Static);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer_Static.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer_Static.Get(), DXGI_FORMAT_R32_UINT, 0);
	}
	else if(mode==2) {
		//set the input layout
		context->IASetInputLayout(inputLayout_Dynamic.Get());
		//set the shaders
		context->VSSetShader(vertexShader_Dynamic.Get(), NULL, 0);
		context->PSSetShader(pixelShader_General.Get(), NULL, 0);
		//set the vertex and index buffer
		UINT stride = sizeof(Vertex_Dynamic);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer_Dynamic.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer_Dynamic.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

}

void Renderer::updateConstantBufferManager()
{
	for (auto& manager : VSconstantBufferManager_collection) {
		if (manager.dirty)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			context->Map(manager.constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, manager.temporaryBufferData.data(), manager.temporaryBufferData.size());
			context->Unmap(manager.constantBuffer.Get(), 0);
			manager.dirty = false;
		}
	}

	for (auto& manager : PSconstantBufferManager_collection)
	{
		if (manager.dirty)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			context->Map(manager.constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, manager.temporaryBufferData.data(), manager.temporaryBufferData.size());
			context->Unmap(manager.constantBuffer.Get(), 0);
			manager.dirty = false;
		}
	}
}

void Renderer::GeometryPass(MeshManager& meshManager)
{
	//set the render target to the GBuffer
	ID3D11RenderTargetView* rtvArr[3] = {
	   gBufferRTVs[0].Get(),
	   gBufferRTVs[1].Get(),
	   gBufferRTVs[2].Get()
	};
	context->OMSetRenderTargets(3, rtvArr, depthStencilView.Get());

	context->IASetInputLayout(inputLayout_Static.Get());
	context->VSSetShader(gBufferVertexShader.Get(), NULL, 0);
	context->PSSetShader(gBufferPixelShader.Get(), NULL, 0);

	UINT stride = sizeof(Vertex_Static);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, vertexBuffer_Static.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indexBuffer_Static.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void Renderer::LightPass()
{
	context->OMSetDepthStencilState(lightingDepthStencilState.Get(), 0);
	context->OMSetRenderTargets(1, backbufferRenderTargetView.GetAddressOf(), depthStencilView.Get());
	context->PSSetShaderResources(5, 3, gBufferSRVs->GetAddressOf());
	//set shader
	context->IASetInputLayout(lightingInputLayout.Get());
	context->VSSetShader(lightingVertexShader.Get(), NULL, 0);
	context->PSSetShader(lightingPixelShader.Get(), NULL, 0);
	//set vertex buffer
	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, lightingVertexBuffer.GetAddressOf(), &stride, &offset);
}

void Renderer::updateConstantBuffer(bool VSBuffer, std::string bufferName, std::string variableName, void* data, size_t dataSize)
{
	if (VSBuffer) {
		for (size_t i = 0; i < VSconstantBufferManager_collection.size(); i++)
		{
			//find the constant buffer manager with the given name
			if (VSconstantBufferManager_collection[i].name == bufferName)
			{
				//find offset and size of the variable in the constant buffer
				ConstantBufferOffsetInfo cboi = VSconstantBufferManager_collection[i].constantBufferVariableInfoMap[variableName];

				//copy the data to the temporary buffer data
				memcpy(VSconstantBufferManager_collection[i].temporaryBufferData.data() + cboi.offset, data, dataSize);

				//set the dirty flag to true
				VSconstantBufferManager_collection[i].dirty = true;
				break;
			}
		}
	}
	else {
		for (size_t i = 0; i < PSconstantBufferManager_collection.size(); i++)
		{
			//find the constant buffer manager with the given name
			if (PSconstantBufferManager_collection[i].name == bufferName)
			{
				//find offset and size of the variable in the constant buffer
				ConstantBufferOffsetInfo cboi = PSconstantBufferManager_collection[i].constantBufferVariableInfoMap[variableName];

				//copy the data to the temporary buffer data
				memcpy(PSconstantBufferManager_collection[i].temporaryBufferData.data() + cboi.offset, data, dataSize);

				//set the dirty flag to true
				PSconstantBufferManager_collection[i].dirty = true;
				break;
			}
		}
	}
}

void Renderer::updateSkyboxConstantBuffer(DirectX::XMFLOAT4X4 &VP)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(skyConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &VP, sizeof(DirectX::XMFLOAT4X4));
	context->Unmap(skyConstantBuffer.Get(), 0);
}

void Renderer::updataLightingConstantBuffer(lightingConstants& lighting)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(lightingConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &lighting, sizeof(lightingConstants));
	context->Unmap(lightingConstantBuffer.Get(), 0);
}

void Renderer::updateInstanceBuffer(MeshManager& meshmanager,int mode)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (mode == 0) {
		context->Map(VSstructuredBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		MeshDescriptor md = meshmanager.objects["Terrain"];
		memcpy(mappedResource.pData, &meshmanager.instances[md.instanceOffset], sizeof(InstanceData_General) * md.instanceCount);
		context->Unmap(VSstructuredBuffer.Get(), 0);
	}
	else if (mode == 1) {
		context->Map(VSstructuredBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		MeshDescriptor md = meshmanager.objects["Static"];
		memcpy(mappedResource.pData, &meshmanager.instances[md.instanceOffset], sizeof(InstanceData_General) * md.instanceCount);
		context->Unmap(VSstructuredBuffer.Get(), 0);
	}
	else {
		context->Map(VSstructuredBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		MeshDescriptor md = meshmanager.objects["NPC"];
		memcpy(mappedResource.pData, &meshmanager.instances[md.instanceOffset], sizeof(InstanceData_General) * md.instanceCount);
		context->Unmap(VSstructuredBuffer.Get(), 0);

	}
}

void Renderer::updataBonesBuffer(std::vector<float>& bonesVector)
{
	if (bonesVector.size())
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(bonesTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, bonesVector.data(), sizeof(float) * bonesVector.size());
		context->Unmap(bonesTexture.Get(), 0);
	}
}


void Renderer::Initialize(Window& window, MeshManager& meshmanager)
{
	InitializeDeviceAndContext(window);
	InitializeRenderTarget(window);
	InitializeShadersAndConstantBuffer();
	InitializeSkybox(meshmanager.vertices_Skybox, meshmanager.indices_Skybox);
	initializeIndexAndVertexBuffer(meshmanager.vertices_Static, meshmanager.indices_Static, meshmanager.vertices_Dynamic, meshmanager.indices_Dynamic);
	InitializeStructuredBuffer();
	InitializeState();
	InitializeSampler();
	InitializeGBuffer(window);
	InitializeLightingBuffer();

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::Render(MeshManager & meshManager)
{
	cleanFrame();
	updateConstantBufferManager();
	
	
	GeometryPass(meshManager);
	updateInstanceBuffer(meshManager, 1);
	MeshDescriptor md = meshManager.objects["Static"];
	context->DrawIndexedInstanced(md.indexCount, md.instanceCount, md.indexOffset, md.vertexOffset, 0);
	LightPass();
	context->Draw(3, 0);


	SwitchShader(0);
	context->OMSetDepthStencilState(skyDepthStencilState.Get(), 0);
	context->DrawIndexed(static_cast<UINT>(meshManager.indices_Skybox.size()), 0,0);
	context->OMSetDepthStencilState(depthStencilState.Get(), 0);

	SwitchShader(1);
	updateInstanceBuffer(meshManager, 0);
	md = meshManager.objects["Terrain"];
	context->DrawIndexedInstanced(md.indexCount, md.instanceCount, md.indexOffset, md.vertexOffset, 0);

	SwitchShader(2);
	updateInstanceBuffer(meshManager, 2);
	md = meshManager.objects["NPC"];
	context->DrawIndexedInstanced(md.indexCount, md.instanceCount, md.indexOffset, md.vertexOffset, 0);
	
}

void Renderer::cleanFrame()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	context->ClearRenderTargetView(backbufferRenderTargetView.Get(), clearColor);
	context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	for (int i = 0; i < 3; i++) {
		context->ClearRenderTargetView(gBufferRTVs[i].Get(), clearColor);
	}
}


void Renderer::present()
{
	swapChain->Present(0, 0);
}

void Renderer::loadTexture(std::string filename, std::string textureShaderName,UINT slot)
{
	
	int width, height, channels;
	unsigned char* texels = stbi_load(filename.c_str(), &width, &height, &channels, 0);
	if (!texels) {
		MessageBox(NULL, L"Failed to load texture", L"Error", MB_OK);
	}

	unsigned char* texelsWithAlpha;
	if (channels == 3) {
		channels = 4;
		texelsWithAlpha = new unsigned char[width * height * channels];
		for (int i = 0; i < width * height; i++) {
			texelsWithAlpha[i * 4 + 0] = texels[i * 3];
			texelsWithAlpha[i * 4 + 1] = texels[i * 3 + 1];
			texelsWithAlpha[i * 4 + 2] = texels[i * 3 + 2];
			texelsWithAlpha[i * 4 + 3] = 255;
		}
	}
	else {
		texelsWithAlpha = texels;
	}

	//initialize texture description
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = width;
	td.Height = height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = (slot < 4) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = texelsWithAlpha;
	initData.SysMemPitch = width * channels;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	HRESULT hr = device->CreateTexture2D(&td, &initData, texture.GetAddressOf());
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to create texture", L"Error", MB_OK);
	}
	//create shader resource view for the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = (slot < 4) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = 1;
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	hr = device->CreateShaderResourceView(texture.Get(), &srvd, srv.GetAddressOf());
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to create Shader Resource View", L"Error", MB_OK);
	}
	
	textureMap[textureShaderName] = { texture,srv };

	context->PSSetShaderResources(slot, 1, srv.GetAddressOf());

	if (texelsWithAlpha != texels) {
		delete[] texelsWithAlpha;
	}
	stbi_image_free(texels);
}

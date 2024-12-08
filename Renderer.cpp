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
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob_S;
	hr=D3DCompileFromFile(L"PixelShader_Static.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, psBlob_S.GetAddressOf(), nullptr);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to compile PixelShader_Static.hlsl", L"Shader Error", MB_OK);
	}
	device->CreatePixelShader(psBlob_S->GetBufferPointer(), psBlob_S->GetBufferSize(), nullptr, &pixelShader_Static);

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

	//compile pixel shader and store it in psBlob,then create pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob_D;
	hr=D3DCompileFromFile(L"PixelShader_Dynamic.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, psBlob_D.GetAddressOf(), nullptr);
	if (FAILED(hr)) {
		MessageBox(NULL, L"Failed to compile PixelShader_Dynamic.hlsl", L"Shader Error", MB_OK);
	}
	device->CreatePixelShader(psBlob_D->GetBufferPointer(), psBlob_D->GetBufferSize(), nullptr, &pixelShader_Dynamic);

	//create constant buffer
	InitializeConstantBuffer(vsBlob_S, psBlob_S);
	InitializeConstantBuffer(vsBlob_D, psBlob_D);
}

void Renderer::InitializeConstantBuffer(Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, Microsoft::WRL::ComPtr<ID3DBlob> psBlob)
{
	//--------- VS constant buffer creation----------------//
	//create shader reflection from the pixel shader
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> VSreflection;
	D3DReflect(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)VSreflection.GetAddressOf());
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

			totalSize = max(totalSize, svd.StartOffset + svd.Size);
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

		context->VSSetConstantBuffers(i + static_cast<UINT>(VSConstantBufferDynamicOffset), 1, constantBuffer.GetAddressOf());
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

			totalSize = max(totalSize, svd.StartOffset + svd.Size);
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

		context->PSSetConstantBuffers(i + static_cast<UINT>(PSConstantBufferDynamicOffset), 1, constantBuffer.GetAddressOf());
	}
	if (VSConstantBufferDynamicOffset == 0 && PSConstantBufferDynamicOffset == 0)
	{
		VSConstantBufferDynamicOffset = VSconstantBufferManager_collection.size();
		PSConstantBufferDynamicOffset = PSconstantBufferManager_collection.size();
	}
}

void Renderer::initializeIndexAndVertexBuffer(std::vector<Vertex_Static>& vertices_Static, std::vector<unsigned int>& indices_Static, std::vector<Vertex_Dynamic>& vertices_dynamic, std::vector<unsigned int>& indices_dynamic)
{
	//-------initialize the static buffer----------------//
	int vertexSizeInBytes = sizeof(Vertex_Static);
	size_t numVertics = vertices_Static.size();
	UINT numIndics = static_cast<UINT>(indices_Static.size());
	indicesSize_Static = numIndics;
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
	indicesSize_Dynamic = numIndics;
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

void Renderer::InitializeInstanceBuffer()
{
	//custom buffer size, you can change it
	UINT bufferSize = 10;
	//initialize the static instance buffer
	D3D11_BUFFER_DESC id = {};
	id.Usage = D3D11_USAGE_DYNAMIC;
	id.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	id.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	id.ByteWidth = sizeof(InstanceData_Static) * bufferSize;
	id.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	id.StructureByteStride = sizeof(InstanceData_Static);
	device->CreateBuffer(&id, nullptr, &instanceBuffer_Static);

	//initialize the dynamic instance buffer
	id.ByteWidth = sizeof(InstanceData_Dynamic) * bufferSize;
	id.StructureByteStride = sizeof(InstanceData_Dynamic);
	device->CreateBuffer(&id, nullptr, &instanceBuffer_Dynamic);

	//create shader resource view for the instance buffer
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = DXGI_FORMAT_UNKNOWN;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvd.Buffer.FirstElement = 0;
	srvd.Buffer.NumElements = bufferSize;
	device->CreateShaderResourceView(instanceBuffer_Static.Get(), &srvd, SRV_Static.GetAddressOf());
	device->CreateShaderResourceView(instanceBuffer_Dynamic.Get(), &srvd, SRV_Dynamic.GetAddressOf());

	//bind the instance buffer to the vertex shader
	context->VSSetShaderResources(0, 1, SRV_Static.GetAddressOf());
	context->VSSetShaderResources(1, 1, SRV_Dynamic.GetAddressOf());

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

void Renderer::SwitchShader(bool _static)
{
	if (_static) {
		//set the input layout
		context->IASetInputLayout(inputLayout_Static.Get());
		//set the shaders
		context->VSSetShader(vertexShader_Static.Get(), NULL, 0);
		context->PSSetShader(pixelShader_Static.Get(), NULL, 0);
		//set the primitive topology
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//set the vertex and index buffer
		UINT stride = sizeof(Vertex_Static);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer_Static.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(indexBuffer_Static.Get(), DXGI_FORMAT_R32_UINT, 0);
	}
	else {
		//set the input layout
		context->IASetInputLayout(inputLayout_Dynamic.Get());
		//set the shaders
		context->VSSetShader(vertexShader_Dynamic.Get(), NULL, 0);
		context->PSSetShader(pixelShader_Dynamic.Get(), NULL, 0);
		//set the primitive topology
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

void Renderer::updateInstanceBuffer(std::vector<InstanceData_Static>& instances_Static, std::vector<InstanceData_Dynamic>& instances_Dynamic, std::vector<float> &bonesVector)
{
	instanceSize_Static = static_cast<UINT>(instances_Static.size());
	instanceSize_Dynamic = static_cast<UINT>(instances_Dynamic.size());
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (instanceSize_Static)
	{
		context->Map(instanceBuffer_Static.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, instances_Static.data(), sizeof(InstanceData_Static) * instanceSize_Static);
		context->Unmap(instanceBuffer_Static.Get(), 0);
	}
	if (instanceSize_Dynamic) 
	{

        context->Map(instanceBuffer_Dynamic.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        memcpy(mappedResource.pData, instances_Dynamic.data(), sizeof(InstanceData_Dynamic) * instanceSize_Dynamic);
        context->Unmap(instanceBuffer_Dynamic.Get(), 0);
	}
	if (bonesVector.size())
	{
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
	initializeIndexAndVertexBuffer(meshmanager.vertices_Static, meshmanager.indices_Static, meshmanager.vertices_Dynamic, meshmanager.indices_Dynamic);
	InitializeInstanceBuffer();
	InitializeState();
}

void Renderer::Render()
{
	updateConstantBufferManager();

	//SwitchShader(true);
	//context->DrawIndexedInstanced(indicesSize_Static, instanceSize_Static, 0, 0, 0);
	SwitchShader(false);
	context->DrawIndexedInstanced(indicesSize_Dynamic, instanceSize_Dynamic, 0, 0, 0);
	
}

void Renderer::cleanFrame()
{
	float clearColor[] = { 0.0f, 0.2f, 0.2f, 1.0f };
	context->ClearRenderTargetView(backbufferRenderTargetView.Get(), clearColor);
	context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::cleanup()
{
	//release all the resources
	vertexBuffer_Static.Reset();
	for (auto& manager : PSconstantBufferManager_collection)
	{
		manager.constantBuffer.Reset();
	}
	PSconstantBufferManager_collection.clear();
	inputLayout_Static.Reset();
	vertexShader_Static.Reset();
	pixelShader_Static.Reset();
	depthStencilView.Reset();
	depthbuffer.Reset();
	backbufferRenderTargetView.Reset();
	backbuffer.Reset();
	swapChain.Reset();
	context.Reset();
	device.Reset();
	adapter.Reset();
}

void Renderer::present()
{
	swapChain->Present(0, 0);
}

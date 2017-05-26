#include "Renderer.h"
#include <d3dx12.h>

bool CRenderer::Init(int ScreenWidth, int ScreenHeight, HWND hWnd) {

#if(_DEBUG)
	// Enable debugging if necessary
	ComPtr<ID3D12Debug> DebugController;
	D3D12GetDebugInterface(__uuidof(ID3D12Debug), &DebugController);
	DebugController->EnableDebugLayer();
#endif // DEBUG

	CreateDevice();
	CreateDescriptorHeaps();
	CreateSwapChain(ScreenWidth, ScreenHeight, hWnd);


	return true;
}

void CRenderer::CreateDevice() {

	// DXGI Factory
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), &DxgiFactory);

	// D3D Device
	D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0,
		__uuidof(ID3D12Device),
		&Device);

	Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), &Fence);

	// Get descriptor sizes
	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	SrvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Command Objects
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};

	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	Device->CreateCommandQueue(&QueueDesc, __uuidof(ID3D12CommandQueue), &CommandQueue);
	Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), &CommandAllocator);
	Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), nullptr, __uuidof(ID3D12CommandList), &CommandList);

	CommandList->Close();
}

void CRenderer::CreateDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc;
	RtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvHeapDesc.NodeMask = 0;
	Device->CreateDescriptorHeap(&RtvHeapDesc, __uuidof(ID3D12DescriptorHeap), &RtvHeap);

	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
	DsvHeapDesc.NumDescriptors = 1;
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.NodeMask = 0;
	Device->CreateDescriptorHeap(&DsvHeapDesc, __uuidof(ID3D12DescriptorHeap), &DsvHeap);
}

void CRenderer::CreateSwapChain(int Width, int Height, HWND hWnd) {
	SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC SD;

	SD.BufferDesc.Width = Width;
	SD.BufferDesc.Height = Height;
	SD.BufferDesc.RefreshRate.Numerator = 60;
	SD.BufferDesc.RefreshRate.Denominator = 1;
	SD.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SD.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SD.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 4 x MSAA
	SD.SampleDesc.Count = 4;
	SD.SampleDesc.Quality = 1;

	SD.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SD.BufferCount = SwapChainBufferCount;
	SD.OutputWindow = hWnd;
	SD.Windowed = true;
	SD.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SD.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DxgiFactory->CreateSwapChain(CommandQueue.Get(), &SD, &SwapChain);

	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(
		RtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < SwapChainBufferCount; i++) {
		SwapChain->GetBuffer(i, __uuidof(ID3D12Resource), &SwapChainBuffers[i]);
		Device->CreateRenderTargetView(SwapChainBuffers[i].Get(), nullptr, RtvHeapHandle);
		RtvHeapHandle.Offset(1, RtvDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC DepthStencilDescriptor;
	DepthStencilDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDescriptor.Alignment = 0;
	DepthStencilDescriptor.Width = Width;
	DepthStencilDescriptor.Height = Height;
	DepthStencilDescriptor.DepthOrArraySize = 1;
	DepthStencilDescriptor.MipLevels = 1;
	DepthStencilDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilDescriptor.SampleDesc.Count = 4; // 4x MSAA
	DepthStencilDescriptor.SampleDesc.Quality = 1;
	DepthStencilDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE OptimizedClear;
	OptimizedClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	OptimizedClear.DepthStencil.Depth = 1.0f;
	OptimizedClear.DepthStencil.Stencil = 0;

	Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&DepthStencilDescriptor,
		D3D12_RESOURCE_STATE_COMMON,
		&OptimizedClear,
		IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the
	// format of the resource.
	Device->CreateDepthStencilView(
		DepthStencilBuffer.Get(),
		nullptr,
		GetDepthStencilHeap());

	// Transition the resource from its initial state to be used as a depth buffer.
	CommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

D3D12_CPU_DESCRIPTOR_HANDLE CRenderer::GetCurrentBackBufferHeap() const {
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		CurrentBackBuffer,
		RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE CRenderer::GetDepthStencilHeap() const {
	return DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

bool CRenderer::Frame() {
	return true;
}



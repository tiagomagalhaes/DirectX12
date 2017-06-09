#include "Renderer.h"
#include <d3dx12.h>
#include <comdef.h>

bool CRenderer::Init(int ScreenWidth, int ScreenHeight, HWND hWnd) {

#if(_DEBUG)
	// Enable debugging if necessary
	ComPtr<ID3D12Debug> DebugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(DebugController.GetAddressOf()));
	DebugController->EnableDebugLayer();
#endif // DEBUG

	CreateDevice();
	CreateDescriptorHeaps();
	SetupSwapChain(ScreenWidth, ScreenHeight, hWnd);
	SetupViewport(ScreenWidth, ScreenHeight);

	return true;
}

void CRenderer::CreateDevice() {
	HRESULT hr;
	const TCHAR *errorText;

	// DXGI Factory
	CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));

#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));
	debugController->EnableDebugLayer();
#endif // DEBUG

	// D3D Device
	hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1,
		IID_PPV_ARGS(&Device));

	errorText = _com_error(hr).ErrorMessage();

	hr = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf()));

	errorText = _com_error(hr).ErrorMessage();

	// Get descriptor sizes
	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DsvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	SrvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Command Objects
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};

	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	hr = Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(CommandQueue.GetAddressOf()));
	errorText = _com_error(hr).ErrorMessage();
	hr = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandAllocator.GetAddressOf()));
	errorText = _com_error(hr).ErrorMessage();
	hr = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(CommandList.GetAddressOf()));
	errorText = _com_error(hr).ErrorMessage();

	hr = CommandList->Close();
	errorText = _com_error(hr).ErrorMessage();
}

void CRenderer::CreateDescriptorHeaps() {
	HRESULT hr;
	const TCHAR *errorText;

	D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc;
	RtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RtvHeapDesc.NodeMask = 0;
	hr = Device->CreateDescriptorHeap(&RtvHeapDesc, IID_PPV_ARGS(RtvHeap.GetAddressOf()));
	errorText = _com_error(hr).ErrorMessage();

	D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
	DsvHeapDesc.NumDescriptors = 1;
	DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DsvHeapDesc.NodeMask = 0;
	hr = Device->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(DsvHeap.GetAddressOf()));
	errorText = _com_error(hr).ErrorMessage();
}

void CRenderer::SetupSwapChain(int Width, int Height, HWND hWnd) {
	HRESULT hr;
	const TCHAR *errorText;

	SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC SD;

	SD.BufferDesc.Width = Width;
	SD.BufferDesc.Height = Height;
	SD.BufferDesc.RefreshRate.Numerator = 60;
	SD.BufferDesc.RefreshRate.Denominator = 1;
	SD.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SD.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SD.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	SD.SampleDesc.Count = 1;
	SD.SampleDesc.Quality = 1;

	SD.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SD.BufferCount = SwapChainBufferCount;
	SD.OutputWindow = hWnd;
	SD.Windowed = true;
	SD.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SD.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = DxgiFactory->CreateSwapChain(CommandQueue.Get(), &SD, SwapChain.GetAddressOf());
	errorText = _com_error(hr).ErrorMessage();

	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHeapHandle(
		RtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < SwapChainBufferCount; i++) {
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffers[i]));
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
	DepthStencilDescriptor.SampleDesc.Count = 1;
	DepthStencilDescriptor.SampleDesc.Quality = 1;
	DepthStencilDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE OptimizedClear;
	OptimizedClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	OptimizedClear.DepthStencil.Depth = 1.0f;
	OptimizedClear.DepthStencil.Stencil = 0;

	hr = Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&DepthStencilDescriptor,
		D3D12_RESOURCE_STATE_COMMON,
		&OptimizedClear,
		IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf()));
	errorText = _com_error(hr).ErrorMessage();

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

void CRenderer::SetupViewport(int ScreenWidth, int ScreenHeight) {
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(ScreenWidth);
	vp.Height = static_cast<float>(ScreenHeight);

	CommandList->RSSetViewports(1, &vp);
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



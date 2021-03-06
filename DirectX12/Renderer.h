#pragma once
#include <Windows.h>
#include <dxgi.h>
#include <d3d12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class CRenderer {
public:
	bool Init(int, int, HWND);
	bool Frame();

private:
	static const int SwapChainBufferCount = 2;

	ComPtr<ID3D12Device> Device;
	ComPtr<IDXGIFactory1> DxgiFactory;
	ComPtr<ID3D12Fence> Fence;

	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12CommandAllocator> CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> CommandList;

	ComPtr<IDXGISwapChain> SwapChain;

	ComPtr<ID3D12DescriptorHeap> RtvHeap;
	ComPtr<ID3D12DescriptorHeap> DsvHeap;

	ComPtr<ID3D12Resource> SwapChainBuffers[SwapChainBufferCount];
	ComPtr<ID3D12Resource> DepthStencilBuffer;

	UINT RtvDescriptorSize, DsvDescriptorSize, SrvDescriptorSize;

	int CurrentBackBuffer = 0;

	void CreateDevice();
	void SetupSwapChain(int, int, HWND);
	void CreateDescriptorHeaps();
	void SetupViewport(int ScreenWidth, int ScreenHeight);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferHeap() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilHeap() const;

};
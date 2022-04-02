#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer
      Summary:  Constructor
      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    //TIP : 이니셜라이저랑 할당은 매우 다르다. 초기화를 해줘야 처음부터 '유효한 개체'라고 할 수 있다. 무조건 필요하다 그런 건가?
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL), m_featureLevel(D3D_FEATURE_LEVEL_11_0), m_d3dDevice(nullptr),
        m_d3dDevice1(nullptr), m_immediateContext(nullptr), m_immediateContext1(nullptr),
        m_swapChain(nullptr), m_swapChain1(nullptr), m_renderTargetView(nullptr),
        m_vertexShader(nullptr), m_pixelShader(nullptr), m_vertexLayout(nullptr), m_vertexBuffer(nullptr)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize
      Summary:  Creates Direct3D device and swap chain
      Args:     HWND hWnd
                  Handle to the window
      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView].
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
   
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
        {
            Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
            hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(dxgiDevice.GetAddressOf()));
            if (SUCCEEDED(hr))
            {
                Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(factory.GetAddressOf()));
                }
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        Microsoft::WRL::ComPtr<IDXGIFactory2> factory2;
        hr = factory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(factory2.GetAddressOf()));
        if (factory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(m_d3dDevice1.GetAddressOf()));
            if (SUCCEEDED(hr))
            {
                (void)m_immediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(m_immediateContext1.GetAddressOf()));
            }

            DXGI_SWAP_CHAIN_DESC1 sd = {};
            sd.Width = width;
            sd.Height = height;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;

            hr = factory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, &m_swapChain1);
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(m_swapChain.GetAddressOf()));
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd = {};
            sd.BufferCount = 1;
            sd.BufferDesc.Width = width;
            sd.BufferDesc.Height = height;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = hWnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;

            hr = factory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
            return hr;

        // Create a render target view
        Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
        //QUESTION : &renderTargetView와 renderTargetView.GetAddressOf()의 차이가 뭔데 되냐? swapChain->Present(0, 0); 여기서 문제 생기더라.
        // 
        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_immediateContext->RSSetViewports(1, &vp);

        //Create VertexBuffer
        {
            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(SimpleVertex) * 3;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = 0;
            bd.MiscFlags = 0;

            SimpleVertex vertices[] =
            {
                XMFLOAT3(0.0f, 0.5f, 0.5f),
                XMFLOAT3(0.5f, -0.5f, 0.5f),
                XMFLOAT3(-0.5f, -0.5f, 0.5f),
            };

            D3D11_SUBRESOURCE_DATA initData;
            ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
            initData.pSysMem = vertices;
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;
            hr = m_d3dDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());

            if (FAILED(hr)) 
                return hr;
        }

        //Create IndexBuffer
        /*{
            WORD indices[] =
            {
                0, 1, 2
            };

            D3D11_BUFFER_DESC bd;
            ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(WORD) * 3;
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = 0;
            bd.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA initData;
            ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
            initData.pSysMem = indices;
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;
            hr = m_d3dDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());

            if (FAILED(hr))
                return hr;
        }*/

        //Create Shader
        {
            ComPtr<ID3DBlob> pVSBlob = nullptr;
            //QUESTION : 경로 왜 못 찾지? ../Library 이거 왜 해야 함?
            hr = compileShaderFromFile(
                L"../Library/Shaders/Lab03.fxh",
                "VS",
                "vs_5_0",
                pVSBlob.GetAddressOf()
            );
            if (FAILED(hr))
                return hr;

            hr = m_d3dDevice->CreateVertexShader(
                pVSBlob->GetBufferPointer(),
                pVSBlob->GetBufferSize(),
                nullptr,
                m_vertexShader.GetAddressOf());
            if (FAILED(hr))
                return hr;

            ComPtr<ID3DBlob> pPSBlob = nullptr;
            hr = compileShaderFromFile(
                L"../Library/Shaders/Lab03.fxh",
                "PS",
                "ps_5_0",
                pPSBlob.GetAddressOf()
            );
            if (FAILED(hr))
                return hr;

            hr = m_d3dDevice->CreatePixelShader(
                pPSBlob->GetBufferPointer(),
                pPSBlob->GetBufferSize(),
                nullptr,
                m_pixelShader.GetAddressOf());
            if (FAILED(hr))
                return hr;

            //Create InputLayout
            D3D11_INPUT_ELEMENT_DESC layouts[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
            };
            UINT uNumElements = ARRAYSIZE(layouts);
            
            hr = m_d3dDevice->CreateInputLayout(
                layouts,
                uNumElements,
                pVSBlob->GetBufferPointer(),
                pVSBlob->GetBufferSize(),
                m_vertexLayout.GetAddressOf()
            );
            
            if (FAILED(hr)) 
                return hr;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render
      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

    void Renderer::Render()
    {

        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;
        m_immediateContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
        m_immediateContext->IASetInputLayout(m_vertexLayout.Get());
        m_immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        m_immediateContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
        m_immediateContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

        m_immediateContext->Draw(3, 0);

        m_swapChain->Present(0, 0);
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);  //Draw ~ Present 사이에만 없으면 되나 봄.
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::compileShaderFromFile
      Summary:  Helper for compiling shaders with D3DCompile
      Args:     PCWSTR pszFileName
                  A pointer to a constant null-terminated string that
                  contains the name of the file that contains the
                  shader code
                PCSTR pszEntryPoint
                  A pointer to a constant null-terminated string that
                  contains the name of the shader entry point function
                  where shader execution begins
                PCSTR pszShaderModel
                  A pointer to a constant null-terminated string that
                  specifies the shader target or set of shader
                  features to compile against
                ID3DBlob** ppBlobOut
                  A pointer to a variable that receives a pointer to
                  the ID3DBlob interface that you can use to access
                  the compiled code
      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    
    HRESULT Renderer::compileShaderFromFile(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR szShaderModel, _Outptr_ ID3DBlob** ppBlobOut)
    {
        if (!pszFileName || !pszEntryPoint || !szShaderModel || !ppBlobOut)
            return E_INVALIDARG;

        *ppBlobOut = nullptr;

        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
        flags |= D3DCOMPILE_DEBUG;
#endif

        const D3D_SHADER_MACRO defines[] =
        {
            "EXAMPLE_DEFINE", "1",
            NULL, NULL
        };

        ComPtr<ID3DBlob> errorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(pszFileName, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            pszEntryPoint, szShaderModel, flags, 0, ppBlobOut, errorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            if (errorBlob)
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());

            return hr;
        }

        return hr;
    }
}
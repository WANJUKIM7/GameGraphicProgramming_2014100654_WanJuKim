#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                 m_immediateContext, m_immediateContext1, m_swapChain,
                 m_swapChain1, m_renderTargetView, m_depthStencil,
                 m_depthStencilView, m_cbChangeOnResize, m_camera,
                 m_projection, m_renderables, m_vertexShaders,
                 m_pixelShaders].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
   //TIP : 이니셜라이저랑 할당은 매우 다르다. 초기화를 해줘야 처음부터 '유효한 개체'라고 할 수 있다. 무조건 필요하다 그런 건가?
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_depthStencil(nullptr)
        , m_depthStencilView(nullptr)
        , m_cbChangeOnResize(nullptr)
        , m_cbLights(nullptr)
        , m_pszMainSceneName(L"")
        , m_camera(XMVectorSet(0.0f,0.0f,-10.0f,0.0f))
        , m_projection(XMMatrixIdentity())
        , m_renderables()   //Question : 초기화 이거 맞나? NULL로 하는 건 아니다.
        , m_aPointLights()
        , m_vertexShaders() //TIP : default는 이런 식으로 하는 거야~.  default가 있다는 거 자체가 default로 생성해도 괜찮다는 거. 없으면 그렇게 하면 안 된다는 거. 이게 바로 암묵적인 룰?
        , m_pixelShaders(std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>())  //이렇게 초기화 하는 사람도 있는데?
        , m_scenes()
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_cbChangeOnResize,
                 m_projection, m_cbLights, m_camera, m_vertexShaders,
                 m_pixelShaders, m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - (UINT)rc.left;
        UINT height = rc.bottom - (UINT)rc.top;

        //마우스 고정
        POINT p1, p2;
        p1.x = rc.left;
        p1.y = rc.top;
        p2.x = rc.right;
        p2.y = rc.bottom;

        ClientToScreen(hWnd, &p1);
        ClientToScreen(hWnd, &p2);

        rc.left = p1.x;
        rc.top = p1.y - 25; //Question : GetWindowRect()라는 함수는 윈도우 창 크기를 가져오면 창 부분을 깔끔하게 가져올 수 있다.
        rc.right = p2.x;
        rc.bottom = p2.y;

        ClipCursor(&rc);

        //Create device
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
        ComPtr<IDXGIFactory1> factory(nullptr);
        {
            ComPtr<IDXGIDevice> dxgiDevice(nullptr);
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter(nullptr);
                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(factory.GetAddressOf()));
                }
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        ComPtr<IDXGIFactory2> factory2;
        //TIP : Comptr를 쓰기 때문에 QueryInterface를 쓰면 안된다? 
        hr = factory.As(&factory2);
        if (factory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                (void)m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc =
                {
                    .Count = 1,
                    .Quality = 0
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                //.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
            };

            hr = factory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, &m_swapChain1);
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd
            {
                .BufferDesc =
                {
                    .Width = width,
                    .Height = height,
                    .RefreshRate =
                    {
                        .Numerator = 60,
                        .Denominator = 1
                    },
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                },
                .SampleDesc =
                {
                    .Count = 1,
                    .Quality = 0,
                },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = hWnd,
                .Windowed = false,
                //.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL //QUESTION : Warning 뭐 어떻게 하라고?
            };

            hr = factory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
            return hr;

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
        //QUESTION : &renderTargetView와 renderTargetView.GetAddressOf()의 차이가 뭔데 되냐? swapChain->Present(0, 0); 여기서 문제 생기더라.

        // Setup the viewport
        {
            D3D11_VIEWPORT vp =
            {
                .TopLeftX = 0,
                .TopLeftY = 0,
                .Width = (FLOAT)width,
                .Height = (FLOAT)height,
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f
            };
            m_immediateContext->RSSetViewports(1, &vp);
        }

        // Create Depth Stencil View
        {
            D3D11_TEXTURE2D_DESC descDepth =
            {
                .Width = width,
                .Height = height,
                .MipLevels = 1,
                .ArraySize = 1,
                .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                .SampleDesc = 
                {
                    .Count = 1,
                    .Quality = 0,
                },
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                .CPUAccessFlags = 0,
                .MiscFlags = 0,
            };

            hr = m_d3dDevice->CreateTexture2D(&descDepth, NULL, m_depthStencil.GetAddressOf());
            if (FAILED(hr))
                return hr;

            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
            {
                .Format = descDepth.Format,
                .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
                .Texture2D =
                {
                    .MipSlice = 0
                }
            };

            hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
            if (FAILED(hr))
                return hr;

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
        }

        //Create View Buffer
        m_camera.Initialize(m_d3dDevice.Get());

        //Create Projection Buffer
        {
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(CBChangeOnResize),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u,
                .StructureByteStride = 0u
            };
            hr = m_d3dDevice->CreateBuffer(&bd, 0, m_cbChangeOnResize.GetAddressOf());
            if (FAILED(hr))
                return hr;
            m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<FLOAT>(width) / static_cast<FLOAT>(height), 0.01f, 100.0f);
        }

        //Creat CbLights
        {
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(CBLights),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u,
                .StructureByteStride = 0u
            };
            hr = m_d3dDevice->CreateBuffer(&bd, 0, m_cbLights.GetAddressOf());
            if (FAILED(hr))
                return hr;
        }

        //Initialize m_vertexShaders, m_pixelShaders, m_renderables, m_scenes
        for (auto m_renderable = begin(m_renderables); m_renderable != end(m_renderables); m_renderable++)
            m_renderable->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

        for (auto m_vertexShader = begin(m_vertexShaders); m_vertexShader != end(m_vertexShaders); m_vertexShader++)
            m_vertexShader->second->Initialize(m_d3dDevice.Get());

        for (auto m_pixelShader = begin(m_pixelShaders); m_pixelShader != end(m_pixelShaders); m_pixelShader++)
            m_pixelShader->second->Initialize(m_d3dDevice.Get());
        
        for (auto m_scene = begin(m_scenes); m_scene != end(m_scenes); m_scene++)
            m_scene->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable

      Summary:  Add a renderable object and initialize the object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Unique pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddRenderable(
        _In_ PCWSTR pszRenderableName,
        _In_ const std::shared_ptr<Renderable>& renderable)
    {
        if (m_renderables.find(pszRenderableName) == m_renderables.end())
        {
            m_renderables[pszRenderableName] = renderable;
            return S_OK;
        }
        else
            return E_FAIL;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPointLight

      Summary:  Add a point light

      Args:     size_t index
                  Index of the point light
                const std::shared_ptr<PointLight>& pointLight
                  Shared pointer to the point light object

      Modifies: [m_aPointLights].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pointLight)
    {
        if (index >= NUM_LIGHTS)
            return E_NOTIMPL;
        else
        {
            m_aPointLights[index] = pointLight;
            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader

      Summary:  Add the vertex shader into the renderer

      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add

      Modifies: [m_vertexShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddVertexShader(
        _In_ PCWSTR pszVertexShaderName,
        _In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        if (m_vertexShaders.find(pszVertexShaderName) == m_vertexShaders.end())
        {
            m_vertexShaders[pszVertexShaderName] = vertexShader;
            return S_OK;
        }
        else
            return E_FAIL;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader

      Summary:  Add the pixel shader into the renderer

      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add

      Modifies: [m_pixelShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPixelShader(
        _In_ PCWSTR pszPixelShaderName,
        _In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        if (m_pixelShaders.find(pszPixelShaderName) == m_pixelShaders.end())
        {
            m_pixelShaders[pszPixelShaderName] = pixelShader;
            return S_OK;
        }
        else
            return E_FAIL;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add a scene

      Args:     PCWSTR pszSceneName
                  Key of a scene
                const std::filesystem::path& sceneFilePath
                  File path to initialize a scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT library::Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath)
    {
        if (m_scenes.find(pszSceneName) == m_scenes.end())
        {
            m_scenes[pszSceneName] = std::make_shared<library::Scene>(sceneFilePath);
            return S_OK;
        }
        else
            return E_FAIL;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  Name of the scene to set as the main scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT library::Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        m_pszMainSceneName = pszSceneName;  //QUESTION : 실패할 이유가 있나? 왜 HRESULT?
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Add the pixel shader into the renderer and initialize it

      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data

      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(
        _In_ const DirectionsInput& directions, 
        _In_ const MouseRelativeMovement& mouseRelativeMovement, 
        _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(
            directions,
            mouseRelativeMovement,
            deltaTime
        );
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_camera.Update(deltaTime);
        for (UINT i = 0; i < NUM_LIGHTS; ++i)
            m_aPointLights[i].get()->Update(deltaTime);
        for (auto m_renderable : m_renderables)
        {
            m_renderable.second->Update(deltaTime);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        //Draw ~ Present 사이에만 없으면 되나 봄.
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);    
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT stride[2] =
        {
            sizeof(SimpleVertex),
            sizeof(InstanceData)
        };
        UINT offset[2] = { 0 };
        for (auto m_renderable : m_renderables)
        {
            m_immediateContext->IASetVertexBuffers(0, 1, m_renderable.second->GetVertexBuffer().GetAddressOf(), &stride[0], &offset[0]);
            m_immediateContext->IASetInputLayout(m_renderable.second->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(m_renderable.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);    //WORD라서 R_16이 들어가네.
            
            CBChangeOnCameraMovement cb0 =
            {
                .View = m_camera.GetView(),
            };
            XMStoreFloat4(&cb0.CameraPosition, m_camera.GetEye());
            CBChangeOnResize cb1 =
            {
                .Projection = m_projection
            };

            CBChangesEveryFrame cb2 =
            {
                .World = m_renderable.second->GetWorldMatrix(),
                .OutputColor = m_renderable.second->GetOutputColor()
            };
            
            CBLights cb3 =
            {
                .LightPositions =
                {
                    m_aPointLights[0].get()->GetPosition(), //TIP : 얻어 걸리기;;
                    m_aPointLights[1].get()->GetPosition()
                },
                .LightColors = 
                {
                    m_aPointLights[0].get()->GetColor(),
                    m_aPointLights[1].get()->GetColor()    //벡터는 전치 필요 X.
                }
            };
            
            //Transpose
            cb0.View = XMMatrixTranspose(cb0.View);
            cb1.Projection = XMMatrixTranspose(cb1.Projection);
            cb2.World = XMMatrixTranspose(cb2.World);
            
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cb0, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cb1, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_renderable.second->GetConstantBuffer().Get(), 0u, nullptr, &cb2, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cb3, 0u, 0u);
            
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, m_renderable.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, m_renderable.second->GetConstantBuffer().GetAddressOf());    //TIP : 상상도 못했다; PS에서 쓰는 거라 PS에 세팅해줘야 된다.
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->VSSetShader(m_renderable.second->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(m_renderable.second->GetPixelShader().Get(), nullptr, 0u);

            if (m_renderable.second->HasTexture())
            {
                for (UINT i = 0; i < m_renderable.second->GetNumMaterials(); ++i)
                {
                    UINT materialIndex = m_renderable.second->GetMesh(i).uMaterialIndex;    //TIP : (이틀 절약) 같은 material를 다른 mesh에 사용하는 경우도 있다. 그래서 number마다 해야 한다. 그냥 냅다 0번부터 하는 게 아니라.
                    m_immediateContext->PSSetShaderResources(0, 1, m_renderable.second->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());  //Question : 이 부분 관해서 FXH 좀 봐야할듯. → FBX랑 관련 없었다. DrawIndexed...
                    m_immediateContext->PSSetSamplers(0, 1, m_renderable.second->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());
                    m_immediateContext->DrawIndexed(m_renderable.second->GetMesh(i).uNumIndices, m_renderable.second->GetMesh(i).uBaseIndex, m_renderable.second->GetMesh(i).uBaseVertex);  //TIP : 저번에 buffer 어쩌구 warning은 왜 생긴 거지? 그거 때문에 안 했는데..
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(m_renderable.second->GetNumIndices(), 0u, 0);
            }
        }
        
        // DrawInstanced
        for (auto voxel : m_scenes[m_pszMainSceneName]->GetVoxels())
        {
            ComPtr<ID3D11Buffer> vertexInstanceBuffer[2] =
            {
                voxel->GetVertexBuffer(),
                voxel->GetInstanceBuffer()
            };
            m_immediateContext->IASetVertexBuffers(0u, 2u, vertexInstanceBuffer->GetAddressOf(), stride, offset);
            m_immediateContext->IASetInputLayout(voxel->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);    //WORD라서 R_16이 들어가네.

            CBChangeOnCameraMovement cb0 =
            {
                .View = m_camera.GetView(),
            };
            XMStoreFloat4(&cb0.CameraPosition, m_camera.GetEye());
            CBChangeOnResize cb1 =
            {
                .Projection = m_projection
            };

            CBChangesEveryFrame cb2 =
            {
                .World = voxel->GetWorldMatrix(),
                .OutputColor = voxel->GetOutputColor()
            };

            CBLights cb3 =
            {
                .LightPositions =
                {
                    m_aPointLights[0].get()->GetPosition(), //TIP : 얻어 걸리기;;
                    m_aPointLights[1].get()->GetPosition()
                },
                .LightColors =
                {
                    m_aPointLights[0].get()->GetColor(),
                    m_aPointLights[1].get()->GetColor()    //벡터는 전치 필요 X.
                }
            };

            //Transpose
            cb0.View = XMMatrixTranspose(cb0.View);
            cb1.Projection = XMMatrixTranspose(cb1.Projection);
            cb2.World = XMMatrixTranspose(cb2.World);

            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cb0, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cb1, 0u, 0u);
            m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cb2, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cb3, 0u, 0u);

            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2, 1, voxel->GetConstantBuffer().GetAddressOf());    //TIP : 상상도 못했다; PS에서 쓰는 거라 PS에 세팅해줘야 된다.
            m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

            m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);
            
            m_immediateContext->DrawIndexedInstanced(voxel->GetNumIndices(), voxel->GetNumInstances(), 0u, 0u, 0u);
        }

        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable

      Summary:  Sets the vertex shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        if (m_renderables.find(pszRenderableName) != m_renderables.end())
        {
            if (m_vertexShaders.find(pszVertexShaderName) != m_vertexShaders.end())
            {
                m_renderables[pszRenderableName]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
                return S_OK;
            }
            else
                return E_FAIL;
        }
        else
            return E_FAIL;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable

      Summary:  Sets the pixel shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
    {
        if (m_renderables.find(pszRenderableName) != m_renderables.end())
        {
            if (m_pixelShaders.find(pszPixelShaderName) != m_pixelShaders.end())
            {
                m_renderables.find(pszRenderableName)->second->SetPixelShader(m_pixelShaders.find(pszPixelShaderName)->second);
                return S_OK;
            }
            else
                return E_FAIL;
        }
        else
            return E_FAIL;
    }
    
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfScene

      Summary:  Sets the vertex shader for the voxels in a scene

      Args:     PCWSTR pszSceneName
                  Key of the scene
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT library::Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
    {
        if (m_scenes.find(pszSceneName) != m_scenes.end())
        {
            if (m_vertexShaders.find(pszVertexShaderName) != m_vertexShaders.end())
            {
                for (auto voxel : m_scenes.find(pszSceneName)->second->GetVoxels())
                {
                    voxel.get()->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
                }
                return S_OK;
            }
            else
                return E_FAIL;
        }
        else
            return E_FAIL;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfScene

      Summary:  Sets the pixel shader for the voxels in a scene

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT library::Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
    {
        if (m_scenes.find(pszSceneName) != m_scenes.end())
        {
            if (m_pixelShaders.find(pszPixelShaderName) != m_pixelShaders.end())
            {
                for (auto voxel : m_scenes.find(pszSceneName)->second->GetVoxels())
                {
                    voxel.get()->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
                }
                return S_OK;
            }
            else
                return E_FAIL;
        }
        else
            return E_FAIL;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}
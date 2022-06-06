#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_depthStencil,
                  m_depthStencilView, m_cbChangeOnResize, m_cbShadowMatrix,
                  m_pszMainSceneName, m_camera, m_projection, m_scenes
                  m_invalidTexture, m_shadowMapTexture, m_shadowVertexShader,
                  m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
   //TIP : 이니셜라이저랑 할당은 매우 다르다. 초기화를 해줘야 처음부터 '유효한 개체'라고 할 수 있다. 무조건 필요하다 그런 건가?
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice()
        , m_d3dDevice1()
        , m_immediateContext()
        , m_immediateContext1()
        , m_swapChain()
        , m_swapChain1()
        , m_renderTargetView()
        , m_depthStencil()
        , m_depthStencilView()
        , m_cbChangeOnResize()
        , m_cbShadowMatrix()
        , m_pszMainSceneName(nullptr)
        , m_padding{ '\0' }
        , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_projection()
        , m_scenes()
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
        , m_shadowMapTexture()
        , m_shadowVertexShader()
        , m_shadowPixelShader()
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView, m_vertexShader,
                  m_vertexLayout, m_pixelShader, m_vertexBuffer
                  m_cbShadowMatrix].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (SUCCEEDED(hr))
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u, .Quality = 0u },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        bd.ByteWidth = sizeof(CBLights) * NUM_LIGHTS;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_camera.Initialize(m_d3dDevice.Get());

        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create Shadow ConstantBuffer.
        bd.ByteWidth = sizeof(CBShadowMatrix);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbShadowMatrix.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // initialize m_shadowMapTexture.
        m_shadowMapTexture = std::make_shared<RenderTexture>(uWidth, uHeight);
        hr = m_shadowMapTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        // initialize all point lights.
        for (UINT i = 0u; i < NUM_LIGHTS; ++i)
            m_scenes[m_pszMainSceneName]->GetPointLight(i)->Initialize(uWidth, uHeight);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add scene to renderer

      Args:     PCWSTR pszSceneName
                  The name of the scene
                const std::shared_ptr<Scene>&
                  The shared pointer to Scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull

      Summary:  Return scene with the given name or null

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetShadowMapShaders

      Summary:  Set shaders for the shadow mapping

      Args:     std::shared_ptr<ShadowVertexShader>
                  vertex shader
                std::shared_ptr<PixelShader>
                  pixel shader

      Modifies: [m_shadowVertexShader, m_shadowPixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::SetShadowMapShaders(_In_ std::shared_ptr<ShadowVertexShader> vertexShader, _In_ std::shared_ptr<PixelShader> pixelShader)
    {
        m_shadowVertexShader = move(vertexShader);
        m_shadowPixelShader = move(pixelShader);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Handle user mouse input

      Args:     DirectionsInput& directions
                MouseRelativeMovement& mouseRelativeMovement
                FLOAT deltaTime
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_camera.Update(deltaTime);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        //RenderSceneToTexture();
        //Draw ~ Present 사이에만 없으면 되나 봄.
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);    

        UINT strides[3] =
        {
            static_cast<UINT>(sizeof(SimpleVertex)),
            static_cast<UINT>(sizeof(NormalData)),
            static_cast<UINT>(sizeof(AnimationData))
        };
        UINT offsets[3] = { 0u };
        std::shared_ptr<library::Scene> scene = m_scenes[m_pszMainSceneName];

        // Skybox.
        if (scene->GetSkyBox() != nullptr)
        {
            m_immediateContext->IASetVertexBuffers(0u, 1u, scene->GetSkyBox()->GetVertexBuffer().GetAddressOf(), &strides[0], &offsets[0]);
            m_immediateContext->IASetInputLayout(scene->GetSkyBox()->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(scene->GetSkyBox()->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);    //WORD라서 R_16이 들어가네.

            CBChangeOnCameraMovement cb0 =
            {
                .View = m_camera.GetView(),
            };
            XMStoreFloat4(&cb0.CameraPosition, m_camera.GetEye());
            CBChangeOnResize cb1 =  //QUESTION : projection 관련한 것들은 없어도 되나? initialize에서 했는데.
            {
                .Projection = m_projection
            };

            CBChangesEveryFrame cb2 =
            {
                .World =  scene->GetSkyBox()->GetWorldMatrix(),
                .OutputColor = scene->GetSkyBox()->GetOutputColor(),
                .HasNormalMap = scene->GetSkyBox()->HasNormalMap()
            };
            cb2.World *= XMMatrixTranslationFromVector(m_camera.GetEye());

            for (UINT i = 0u; i < NUM_LIGHTS; ++i)
            {
                FLOAT attenuationDistance = scene->GetPointLight(i)->GetAttenuationDistance();
                FLOAT attenuationDistanceSquared = attenuationDistance * attenuationDistance;

                CBLights cb3 =
                {
                    //.LightPositions = scene->GetPointLight(0)->GetPosition(),
                    //.LightColors = scene->GetPointLight(0)->GetColor(),    //벡터는 전치 필요 X.
                    //.LightViews = scene->GetPointLight(0)->GetViewMatrix(),
                    //.LightProjections = scene->GetPointLight(0)->GetProjectionMatrix()
                    .Position = scene->GetPointLight(i)->GetPosition(),
                    .Color = scene->GetPointLight(i)->GetColor(),
                    .AttenuationDistance = XMFLOAT4(attenuationDistance, attenuationDistance, attenuationDistanceSquared, attenuationDistanceSquared),
                };

                m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cb3, 0u, 0u);
            }

            //Transpose
            cb0.View = XMMatrixTranspose(cb0.View);
            cb1.Projection = XMMatrixTranspose(cb1.Projection);
            cb2.World = XMMatrixTranspose(cb2.World);
            /*cb3.LightViews[0] = XMMatrixTranspose(cb3.LightViews[0]);
            cb3.LightProjections[0] = XMMatrixTranspose(cb3.LightProjections[0]);*/

            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cb0, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cb1, 0u, 0u);
            m_immediateContext->UpdateSubresource(scene->GetSkyBox()->GetConstantBuffer().Get(), 0u, nullptr, &cb2, 0u, 0u);
            //m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cb3, 0u, 0u);

            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, scene->GetSkyBox()->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, scene->GetSkyBox()->GetConstantBuffer().GetAddressOf());    //TIP : 상상도 못했다; PS에서 쓰는 거라 PS에 세팅해줘야 된다.
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->VSSetShader(scene->GetSkyBox()->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(scene->GetSkyBox()->GetPixelShader().Get(), nullptr, 0u);

            if (scene->GetSkyBox()->HasTexture())
            {
                for (UINT i = 0u; i < scene->GetSkyBox()->GetNumMeshes(); ++i)
                {
                    UINT materialIndex = scene->GetSkyBox()->GetMesh(i).uMaterialIndex;    //TIP : (이틀 절약) 같은 material를 다른 mesh에 사용하는 경우도 있다. 그래서 number마다 해야 한다. 그냥 냅다 0번부터 하는 게 아니라.
                    eTextureSamplerType textureSamplerType = scene->GetSkyBox()->GetMaterial(materialIndex)->pDiffuse->GetSamplerType();
                    if (scene->GetSkyBox()->GetMaterial(materialIndex)->pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(2u, 1u, scene->GetSkyBox()->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(0u, 1u, Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf());
                    }
                    m_immediateContext->DrawIndexed(
                        scene->GetSkyBox()->GetMesh(i).uNumIndices
                        , scene->GetSkyBox()->GetMesh(i).uBaseIndex
                        , scene->GetSkyBox()->GetMesh(i).uBaseVertex);  //TIP : 저번에 buffer 어쩌구 warning은 왜 생긴 거지? 그거 때문에 안 했는데..
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(scene->GetSkyBox()->GetNumIndices(), 0u, 0);
            }
        }

        for (auto renderable : scene->GetRenderables())
        {
            ComPtr<ID3D11Buffer> vertexNormalBuffers[2] =
            {
                renderable.second->GetVertexBuffer(),
                renderable.second->GetNormalBuffer()
            };

            m_immediateContext->IASetVertexBuffers(0u, 2u, vertexNormalBuffers->GetAddressOf(), strides, offsets);
            m_immediateContext->IASetInputLayout(renderable.second->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(renderable.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);    //WORD라서 R_16이 들어가네.
            
            CBChangeOnCameraMovement cb0 =
            {
                .View = m_camera.GetView(),
            };
            XMStoreFloat4(&cb0.CameraPosition, m_camera.GetEye());
            CBChangeOnResize cb1 =  //QUESTION : projection 관련한 것들은 없어도 되나? initialize에서 했는데
            {
                .Projection = m_projection
            };

            CBChangesEveryFrame cb2 =
            {
                .World = renderable.second->GetWorldMatrix(),
                .OutputColor = renderable.second->GetOutputColor(),
                .HasNormalMap = renderable.second->HasNormalMap()
            };
            
            for (UINT i = 0u; i < NUM_LIGHTS; ++i)
            {
                FLOAT attenuationDistance = scene->GetPointLight(i)->GetAttenuationDistance();
                FLOAT attenuationDistanceSquared = attenuationDistance * attenuationDistance;

                CBLights cb3 =
                {
                    //.LightPositions = scene->GetPointLight(0)->GetPosition(),
                    //.LightColors = scene->GetPointLight(0)->GetColor(),    //벡터는 전치 필요 X.
                    //.LightViews = scene->GetPointLight(0)->GetViewMatrix(),
                    //.LightProjections = scene->GetPointLight(0)->GetProjectionMatrix()
                    .Position = scene->GetPointLight(i)->GetPosition(),
                    .Color = scene->GetPointLight(i)->GetColor(),
                    .AttenuationDistance = XMFLOAT4(attenuationDistance, attenuationDistance, attenuationDistanceSquared, attenuationDistanceSquared),
                };

                m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cb3, 0u, 0u);
            }

            //Transpose
            cb0.View = XMMatrixTranspose(cb0.View);
            cb1.Projection = XMMatrixTranspose(cb1.Projection);
            cb2.World = XMMatrixTranspose(cb2.World);
            /*cb3.LightViews[0] = XMMatrixTranspose(cb3.LightViews[0]);
            cb3.LightProjections[0] = XMMatrixTranspose(cb3.LightProjections[0]);*/
            
            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cb0, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cb1, 0u, 0u);
            m_immediateContext->UpdateSubresource(renderable.second->GetConstantBuffer().Get(), 0u, nullptr, &cb2, 0u, 0u);
            //m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cb3, 0u, 0u);
            
            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, renderable.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, renderable.second->GetConstantBuffer().GetAddressOf());    //TIP : 상상도 못했다; PS에서 쓰는 거라 PS에 세팅해줘야 된다.
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->VSSetShader(renderable.second->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(renderable.second->GetPixelShader().Get(), nullptr, 0u);

            if (scene->GetSkyBox() != nullptr)
            {
                if (scene->GetSkyBox()->GetMaterial(0))
                {
                    eTextureSamplerType textureSamplerType = scene->GetSkyBox()->GetMaterial(0)->pDiffuse->GetSamplerType();
                    m_immediateContext->PSSetShaderResources(2u, 1u, scene->GetSkyBox()->GetMaterial(0)->pDiffuse->GetTextureResourceView().GetAddressOf());
                    m_immediateContext->PSSetSamplers(0u, 1u, Texture::s_samplers[static_cast<size_t>(textureSamplerType)].GetAddressOf());
                }
            }

            m_immediateContext->DrawIndexed(renderable.second->GetNumIndices(), 0u, 0u);
        }
        
        // Model.
        for (auto model : scene->GetModels())
        {
            ComPtr<ID3D11Buffer> vertexNormalAnimationBuffers[3] =
            {
                model.second->GetVertexBuffer(),
                model.second->GetNormalBuffer(),
                model.second->GetAnimationBuffer()
            };

            m_immediateContext->IASetVertexBuffers(0u, 3u, vertexNormalAnimationBuffers->GetAddressOf(), strides, offsets);
            m_immediateContext->IASetInputLayout(model.second->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(model.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);    //WORD라서 R_16이 들어가네.

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
                .World = model.second->GetWorldMatrix(),
                .OutputColor = model.second->GetOutputColor(),
                .HasNormalMap = model.second->HasNormalMap()
            };

            CBLights cb3[2];
            for (UINT i = 0u; i < NUM_LIGHTS; ++i)
            {
                FLOAT attenuationDistance = scene->GetPointLight(i)->GetAttenuationDistance();
                FLOAT attenuationDistanceSquared = attenuationDistance * attenuationDistance;

                cb3[i].Position = scene->GetPointLight(i)->GetPosition();
                cb3[i].Color = scene->GetPointLight(i)->GetColor();
                cb3[i].AttenuationDistance = XMFLOAT4(attenuationDistance, attenuationDistance, attenuationDistanceSquared, attenuationDistanceSquared);
            }

            //Transpose
            cb0.View = XMMatrixTranspose(cb0.View);
            cb1.Projection = XMMatrixTranspose(cb1.Projection);
            cb2.World = XMMatrixTranspose(cb2.World);
            /*cb3.LightViews[0] = XMMatrixTranspose(cb3.LightViews[0]);
            cb3.LightProjections[0] = XMMatrixTranspose(cb3.LightProjections[0]);*/

            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cb0, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cb1, 0u, 0u);
            m_immediateContext->UpdateSubresource(model.second->GetConstantBuffer().Get(), 0u, nullptr, &cb2, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, cb3, 0u, 0u);

            ComPtr<ID3D11Buffer> lightBuffers[2] =
            {
                m_cbLights,
                m_cbLights,
            };

            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, model.second->GetConstantBuffer().GetAddressOf());
            //m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, model.second->GetConstantBuffer().GetAddressOf());    //TIP : 상상도 못했다; PS에서 쓰는 거라 PS에 세팅해줘야 된다.
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->VSSetShader(model.second->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(model.second->GetPixelShader().Get(), nullptr, 0u);

            if (model.second->HasTexture())
            {
                for (UINT i = 0u; i < model.second->GetNumMeshes(); ++i)
                {
                    UINT materialIndex = model.second->GetMesh(i).uMaterialIndex;    //TIP : (이틀 절약) 같은 material를 다른 mesh에 사용하는 경우도 있다. 그래서 number마다 해야 한다. 그냥 냅다 0번부터 하는 게 아니라.
                    if (model.second->GetMaterial(materialIndex)->pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(0u, 1u, model.second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(0u, 1u, model.second->GetMaterial(materialIndex)->pDiffuse->GetSamplerState().GetAddressOf());
                    }
                    if (model.second->GetMaterial(materialIndex)->pNormal)
                    {
                        m_immediateContext->PSSetShaderResources(1u, 1u, model.second->GetMaterial(materialIndex)->pNormal->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(1u, 1u, model.second->GetMaterial(materialIndex)->pNormal->GetSamplerState().GetAddressOf());
                    }
                    // Set Shadow ShaderResources & Samplers
                    /*m_immediateContext->PSSetShaderResources(2u, 1u, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
                    m_immediateContext->PSSetSamplers(2u, 1u, m_shadowMapTexture->GetSamplerState().GetAddressOf());*/
                    m_immediateContext->DrawIndexed(
                          model.second->GetMesh(i).uNumIndices
                        , model.second->GetMesh(i).uBaseIndex
                        , model.second->GetMesh(i).uBaseVertex);  //TIP : 저번에 buffer 어쩌구 warning은 왜 생긴 거지? 그거 때문에 안 했는데..
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(model.second->GetNumIndices(), 0u, 0);
            }
        }

        // DrawInstanced
        strides[2] = static_cast<UINT>(sizeof(InstanceData));
        for (auto voxel : m_scenes[m_pszMainSceneName]->GetVoxels())
        {
            ComPtr<ID3D11Buffer> vertexNormalInstanceBuffer[3] =
            {
                voxel->GetVertexBuffer(),
                voxel->GetNormalBuffer(),
                voxel->GetInstanceBuffer()
            };
            m_immediateContext->IASetVertexBuffers(0u, 3u, vertexNormalInstanceBuffer->GetAddressOf(), strides, offsets);
            m_immediateContext->IASetInputLayout(voxel->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

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
                .OutputColor = voxel->GetOutputColor(),
                .HasNormalMap = voxel->HasNormalMap()
            };

            CBLights cb3 =
            {
                //.LightPositions = scene->GetPointLight(0)->GetPosition(),
                //.LightColors = scene->GetPointLight(0)->GetColor(),    //벡터는 전치 필요 X.
                //.LightViews = scene->GetPointLight(0)->GetViewMatrix(),
                //.LightProjections = scene->GetPointLight(0)->GetProjectionMatrix()
            };

            //Transpose
            cb0.View = XMMatrixTranspose(cb0.View);
            cb1.Projection = XMMatrixTranspose(cb1.Projection);
            cb2.World = XMMatrixTranspose(cb2.World);
            /*cb3.LightViews[0] = XMMatrixTranspose(cb3.LightViews[0]);
            cb3.LightProjections[0] = XMMatrixTranspose(cb3.LightProjections[0]);*/

            m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0u, nullptr, &cb0, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0u, nullptr, &cb1, 0u, 0u);
            m_immediateContext->UpdateSubresource(voxel->GetConstantBuffer().Get(), 0u, nullptr, &cb2, 0u, 0u);
            m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0u, nullptr, &cb3, 0u, 0u);

            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2u, 1u, voxel->GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, voxel->GetConstantBuffer().GetAddressOf());    //TIP : 상상도 못했다; PS에서 쓰는 거라 PS에 세팅해줘야 된다.
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            m_immediateContext->VSSetShader(voxel->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(voxel->GetPixelShader().Get(), nullptr, 0u);

            if (voxel->HasTexture())
            {
                for (UINT i = 0u; i < voxel->GetNumMaterials(); ++i)
                {
                    UINT materialIndex = voxel->GetMesh(i).uMaterialIndex;    //TIP : (이틀 절약) 같은 material를 다른 mesh에 사용하는 경우도 있다. 그래서 number마다 해야 한다. 그냥 냅다 0번부터 하는 게 아니라.
                    if (voxel->GetMaterial(materialIndex)->pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(0u, 1u, voxel->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(0u, 1u, voxel->GetMaterial(materialIndex)->pDiffuse->GetSamplerState().GetAddressOf());
                    }
                    if (voxel->GetMaterial(materialIndex)->pNormal)
                    {
                        m_immediateContext->PSSetShaderResources(1u, 1u, voxel->GetMaterial(materialIndex)->pNormal->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(1u, 1u, voxel->GetMaterial(materialIndex)->pNormal->GetSamplerState().GetAddressOf());
                    }
                    // Set Shadow ShaderResources & Samplers
                    /*m_immediateContext->PSSetShaderResources(2u, 1u, m_shadowMapTexture->GetShaderResourceView().GetAddressOf());
                    m_immediateContext->PSSetSamplers(2u, 1u, m_shadowMapTexture->GetSamplerState().GetAddressOf());*/
                    m_immediateContext->DrawIndexedInstanced(
                          voxel->GetMesh(i).uNumIndices
                        , voxel->GetNumInstances()
                        , voxel->GetMesh(i).uBaseIndex
                        , voxel->GetMesh(i).uBaseVertex
                        , 0u);  //TIP : 저번에 buffer 어쩌구 warning은 왜 생긴 거지? 그거 때문에 안 했는데..
                }
            }
            else
            {
                m_immediateContext->DrawIndexedInstanced(voxel->GetNumIndices(), voxel->GetNumInstances(), 0u, 0, 0u);
            }
        }

        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::RenderSceneToTexture

      Summary:  Render scene to the texture
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::RenderSceneToTexture()
    {
        //Unbind current pixel shader resources
        ComPtr<ID3D11ShaderResourceView> const pSRV[2] = { NULL, NULL };
        m_immediateContext->PSSetShaderResources(0, 2, pSRV->GetAddressOf());
        m_immediateContext->PSSetShaderResources(2, 1, pSRV->GetAddressOf());

        m_immediateContext->OMSetRenderTargets(1, m_shadowMapTexture->GetRenderTargetView().GetAddressOf(), m_depthStencilView.Get());
        m_immediateContext->ClearRenderTargetView(m_shadowMapTexture->GetRenderTargetView().Get(), Colors::White);
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

        UINT stride[3] =
        {
            static_cast<UINT>(sizeof(SimpleVertex)),
            static_cast<UINT>(sizeof(InstanceData)),
        };
        UINT offset[2] = { 0u };
        std::shared_ptr<library::Scene> scene = m_scenes[m_pszMainSceneName];

        for (auto renderable : scene->GetRenderables())
        {
            m_immediateContext->IASetVertexBuffers(0u, 1u, renderable.second->GetVertexBuffer().GetAddressOf(), &stride[0], &offset[0]);
            m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(renderable.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);    //WORD라서 R_16이 들어가네.

            CBShadowMatrix cb0 =
            {
                .World = renderable.second->GetWorldMatrix(),
                .View = scene->GetPointLight(0)->GetViewMatrix(),
                .Projection = scene->GetPointLight(0)->GetProjectionMatrix(),
                .IsVoxel = false
            };

            //Transpose
            cb0.World = XMMatrixTranspose(cb0.World);
            cb0.View = XMMatrixTranspose(cb0.View);
            cb0.Projection = XMMatrixTranspose(cb0.Projection);

            m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cb0, 0u, 0u);

            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

            if (renderable.second->HasTexture())
            {
                for (UINT i = 0u; i < renderable.second->GetNumMeshes(); ++i)
                {
                    UINT materialIndex = renderable.second->GetMesh(i).uMaterialIndex;    //TIP : (이틀 절약) 같은 material를 다른 mesh에 사용하는 경우도 있다. 그래서 number마다 해야 한다. 그냥 냅다 0번부터 하는 게 아니라.
                    m_immediateContext->DrawIndexed(
                        renderable.second->GetMesh(i).uNumIndices
                        , renderable.second->GetMesh(i).uBaseIndex
                        , renderable.second->GetMesh(i).uBaseVertex);  //TIP : 저번에 buffer 어쩌구 warning은 왜 생긴 거지? 그거 때문에 안 했는데..
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(renderable.second->GetNumIndices(), 0u, 0);
            }
        }

        // Model.
        for (auto model : scene->GetModels())
        {
            m_immediateContext->IASetVertexBuffers(0u, 1u, model.second->GetVertexBuffer().GetAddressOf(), &stride[0], &offset[0]);
            m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(model.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);    //WORD라서 R_16이 들어가네.

            CBShadowMatrix cb0 =
            {
                .World = model.second->GetWorldMatrix(),
                .View = scene->GetPointLight(0)->GetViewMatrix(),
                .Projection = scene->GetPointLight(0)->GetProjectionMatrix(),
                .IsVoxel = false
            };

            //Transpose
            cb0.World = XMMatrixTranspose(cb0.World);
            cb0.View = XMMatrixTranspose(cb0.View);
            cb0.Projection = XMMatrixTranspose(cb0.Projection);

            m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cb0, 0u, 0u);

            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

            if (model.second->HasTexture())
            {
                for (UINT i = 0u; i < model.second->GetNumMeshes(); ++i)
                {
                    UINT materialIndex = model.second->GetMesh(i).uMaterialIndex;    //TIP : (이틀 절약) 같은 material를 다른 mesh에 사용하는 경우도 있다. 그래서 number마다 해야 한다. 그냥 냅다 0번부터 하는 게 아니라.
                    m_immediateContext->DrawIndexed(
                        model.second->GetMesh(i).uNumIndices
                        , model.second->GetMesh(i).uBaseIndex
                        , model.second->GetMesh(i).uBaseVertex);  //TIP : 저번에 buffer 어쩌구 warning은 왜 생긴 거지? 그거 때문에 안 했는데..
                }
            }
            else
            {
                m_immediateContext->DrawIndexed(model.second->GetNumIndices(), 0u, 0);
            }
        }

        // DrawInstanced
        for (auto voxel : m_scenes[m_pszMainSceneName]->GetVoxels())
        {
            ComPtr<ID3D11Buffer> vertexNormalInstanceBuffer[2] =
            {
                voxel->GetVertexBuffer(),
                voxel->GetInstanceBuffer()
            };
            m_immediateContext->IASetVertexBuffers(0u, 2u, vertexNormalInstanceBuffer->GetAddressOf(), stride, offset);
            m_immediateContext->IASetInputLayout(m_shadowVertexShader->GetVertexLayout().Get());
            m_immediateContext->IASetIndexBuffer(voxel->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0u);

            CBShadowMatrix cb0 =
            {
                .World = voxel->GetWorldMatrix(),
                .View = scene->GetPointLight(0)->GetViewMatrix(),
                .Projection = scene->GetPointLight(0)->GetProjectionMatrix(),
                .IsVoxel = true
            };

            //Transpose
            cb0.World = XMMatrixTranspose(cb0.World);
            cb0.View = XMMatrixTranspose(cb0.View);
            cb0.Projection = XMMatrixTranspose(cb0.Projection);

            m_immediateContext->UpdateSubresource(m_cbShadowMatrix.Get(), 0u, nullptr, &cb0, 0u, 0u);

            m_immediateContext->VSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_cbShadowMatrix.GetAddressOf());

            m_immediateContext->VSSetShader(m_shadowVertexShader->GetVertexShader().Get(), nullptr, 0u);
            m_immediateContext->PSSetShader(m_shadowPixelShader->GetPixelShader().Get(), nullptr, 0u);

            if (voxel->HasTexture())
            {
                for (UINT i = 0u; i < voxel->GetNumMaterials(); ++i)
                {
                    UINT materialIndex = voxel->GetMesh(i).uMaterialIndex;    //TIP : (이틀 절약) 같은 material를 다른 mesh에 사용하는 경우도 있다. 그래서 number마다 해야 한다. 그냥 냅다 0번부터 하는 게 아니라.
                    m_immediateContext->DrawIndexedInstanced(
                        voxel->GetMesh(i).uNumIndices
                        , voxel->GetNumInstances()
                        , voxel->GetMesh(i).uBaseIndex
                        , voxel->GetMesh(i).uBaseVertex
                        , 0u);  //TIP : 저번에 buffer 어쩌구 warning은 왜 생긴 거지? 그거 때문에 안 했는데..
                }
            }
            else
            {
                m_immediateContext->DrawIndexedInstanced(voxel->GetNumIndices(), voxel->GetNumInstances(), 0u, 0, 0u);
            }
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
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
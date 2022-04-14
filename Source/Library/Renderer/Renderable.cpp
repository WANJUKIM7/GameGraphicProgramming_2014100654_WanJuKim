#include "Renderer/Renderable.h"
#include "Texture/DDSTextureLoader.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::Renderable

      Summary:  Constructor

      Args:     const std::filesystem::path& textureFilePath
                  Path to the texture to use

      Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer,
                 m_textureRV, m_samplerLinear, m_vertexShader,
                 m_pixelShader, m_textureFilePath, m_world].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderable::Renderable(_In_ const std::filesystem::path& textureFilePath)
        : m_vertexBuffer(nullptr)
        , m_indexBuffer(nullptr)
        , m_constantBuffer(nullptr)
        , m_textureRV(nullptr)
        , m_samplerLinear(nullptr)
        , m_vertexShader(nullptr)
        , m_pixelShader(nullptr)
        , m_textureFilePath(textureFilePath)
        , m_world(XMMatrixIdentity())
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::initialize

      Summary:  Initializes the buffers and the world matrix

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers

      Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer, 
                  m_world].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderable::initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
    {
        if (pDevice == nullptr || pImmediateContext == nullptr)
            return E_FAIL;

        HRESULT hr = S_OK;
        
        //Create VertexBuffer
        {
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),   //Question : 어떻게 BaseCube의 함수를 가져올 수 있지? 포함도 안 하는데..? 자식 걸 가져왔네.
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_VERTEX_BUFFER,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData =
            {
                .pSysMem = getVertices(),
                .SysMemPitch = 0u,
                .SysMemSlicePitch = 0u
            };
            hr = pDevice->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());

            if (FAILED(hr))
                return hr;
        }

        //Create IndexBuffer
        {
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(WORD) * GetNumIndices(),    //sizeof(WORD) 주의하자...
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_INDEX_BUFFER,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u
            };

            D3D11_SUBRESOURCE_DATA initData = 
            {
                .pSysMem = getIndices(),
                .SysMemPitch = 0u,
                .SysMemSlicePitch = 0u
            };
            hr = pDevice->CreateBuffer(&bd, &initData, m_indexBuffer.GetAddressOf());

            if (FAILED(hr))
                return hr;
        }
        
        //Create ConstantBuffer
        {
            m_world = XMMatrixIdentity();
            //TIP : D3D11_USAGE_DYNAMIC은 메모리 변경 == size 변경일 때 사용하는 거였어. 그냥 수정이 아니라.
            D3D11_BUFFER_DESC bd =
            {
                .ByteWidth = sizeof(CBChangesEveryFrame),
                .Usage = D3D11_USAGE_DEFAULT,
                .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                .CPUAccessFlags = 0u,
                .MiscFlags = 0u,
                .StructureByteStride = 0u
            };
            hr = pDevice->CreateBuffer(&bd, 0, m_constantBuffer.GetAddressOf());
            if (FAILED(hr))
                return hr;
        }

        // Load the Texture
        hr = CreateDDSTextureFromFile(pDevice, m_textureFilePath.filename().wstring().c_str(), nullptr, m_textureRV.GetAddressOf());
        if (FAILED(hr))
            return hr;
            
        // Create the sample state
        D3D11_SAMPLER_DESC sampDesc =    //TIP : 이런 방법이 desginated initialize. 장점은? 생성할 때 한번에 하기 때문에 효율적. 할당은 하나씩 하나씩 함.
        {
            .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
            .ComparisonFunc = D3D11_COMPARISON_NEVER,
            .MinLOD = 0.0f,
            .MaxLOD = D3D11_FLOAT32_MAX
        };
        hr = pDevice->CreateSamplerState(&sampDesc, m_samplerLinear.GetAddressOf());
        if (FAILED(hr))
            return hr;

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::SetVertexShader

      Summary:  Sets the vertex shader to be used for this renderable 
                object

      Args:     const std::shared_ptr<VertexShader>& vertexShader
                  Vertex shader to set to

      Modifies: [m_vertexShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        m_vertexShader = vertexShader;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::SetPixelShader

      Summary:  Sets the pixel shader to be used for this renderable
                object

      Args:     const std::shared_ptr<PixelShader>& pixelShader
                  Pixel shader to set to

      Modifies: [m_pixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        m_pixelShader = pixelShader;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexShader

      Summary:  Returns the vertex shader

      Returns:  ComPtr<ID3D11VertexShader>&
                  Vertex shader. Could be a nullptr
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
    {
        return m_vertexShader->GetVertexShader();
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetPixelShader

      Summary:  Returns the vertex shader

      Returns:  ComPtr<ID3D11PixelShader>&
                  Pixel shader. Could be a nullptr
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
    {
        return m_pixelShader->GetPixelShader();
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexLayout

      Summary:  Returns the vertex input layout

      Returns:  ComPtr<ID3D11InputLayout>&
                  Vertex input layout
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
    {
        return m_vertexShader->GetVertexLayout();
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexBuffer

      Summary:  Returns the vertex buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Vertex buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
    {
        return m_vertexBuffer;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetIndexBuffer

      Summary:  Returns the index buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Index buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
    {
        return m_indexBuffer;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetConstantBuffer

      Summary:  Returns the constant buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Constant buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
    {
        return m_constantBuffer;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetWorldMatrix

      Summary:  Returns the world matrix

      Returns:  const XMMATRIX&
                  World matrix
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const XMMATRIX& Renderable::GetWorldMatrix() const
    {
        return m_world;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetTextureResourceView

      Summary:  Returns the texture resource view

      Returns:  ComPtr<ID3D11ShaderResourceView>&
                  The texture resource view
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11ShaderResourceView>& Renderable::GetTextureResourceView()
    {
        return m_textureRV;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetSamplerState

      Summary:  Returns the sampler state

      Returns:  ComPtr<ID3D11SamplerState>&
                  The sampler state
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11SamplerState>& Renderable::GetSamplerState()
    {
        return m_samplerLinear;
    }
}
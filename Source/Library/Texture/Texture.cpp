#include "Texture.h"

#include "Texture/DDSTextureLoader.h"
#include "Texture/WICTextureLoader.h"

namespace library
{
    ComPtr<ID3D11SamplerState> Texture::s_samplers[static_cast<size_t>(eTextureSamplerType::COUNT)];

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::Texture

      Summary:  Constructor

      Args:     const std::filesystem::path& textureFilePath
                  Path to the texture to use

      Modifies: [m_filePath, m_textureRV, m_samplerLinear].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*Texture::Texture(_In_ const std::filesystem::path& filePath)
        : m_filePath(filePath)
        , m_textureRV(nullptr)
        , m_samplerLinear(nullptr)
    {
    }*/

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::Texture

      Summary:  Constructor

      Args:     const std::filesystem::path& textureFilePath
                  Path to the texture to use
                eTextureSamplerType textureSamplerType
                  Texture sampler type of this texture

      Modifies: [m_filePath, m_textureRV, m_textureSamplerType].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Texture::Texture(_In_ const std::filesystem::path& filePath, _In_opt_ eTextureSamplerType textureSamplerType)
        : m_filePath(filePath)
        , m_textureSamplerType(textureSamplerType)
        , m_textureRV(nullptr)
        , m_samplerLinear(nullptr)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::Initialize

      Summary:  Initializes the texture

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers

      Modifies: [m_textureRV, m_samplerLinear].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Texture::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
    {
        if (pDevice == nullptr || pImmediateContext == nullptr)
            return E_NOTIMPL;
        
        HRESULT hr = CreateWICTextureFromFile(
            pDevice,
            pImmediateContext,
            m_filePath.c_str(),
            nullptr,
            m_textureRV.GetAddressOf());
        if (FAILED(hr))
        {
            hr = CreateDDSTextureFromFile(pDevice, m_filePath.c_str(), nullptr, m_textureRV.GetAddressOf());
            if (FAILED(hr))
            {
                OutputDebugString(L"Can't load texture from \"");
                OutputDebugString(m_filePath.c_str());
                OutputDebugString(L"\n");
                return hr;
            }
        }
        
        // Create the sample state
        if (!s_samplers[static_cast<size_t>(eTextureSamplerType::TRILINEAR_WRAP)].Get())
        {
            D3D11_SAMPLER_DESC sampDesc =
            {
                .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
                .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
                .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
                .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
                .ComparisonFunc = D3D11_COMPARISON_NEVER,
                .MinLOD = 0.0f,
                .MaxLOD = D3D11_FLOAT32_MAX
            };
            hr = pDevice->CreateSamplerState(&sampDesc, s_samplers[static_cast<size_t>(eTextureSamplerType::TRILINEAR_WRAP)].GetAddressOf());
            if (FAILED(hr))
            {
                return hr;  
            }
        }

        if (!s_samplers[static_cast<size_t>(eTextureSamplerType::TRILINEAR_CLAMP)].Get())
        {
            D3D11_SAMPLER_DESC sampDesc =
            {
                .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
                .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
                .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
                .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
                .ComparisonFunc = D3D11_COMPARISON_NEVER,
                .MinLOD = 0.0f,
                .MaxLOD = D3D11_FLOAT32_MAX
            };
            hr = pDevice->CreateSamplerState(&sampDesc, s_samplers[static_cast<size_t>(eTextureSamplerType::TRILINEAR_CLAMP)].GetAddressOf());
            if (FAILED(hr))
            {
                return hr;
            }
        }

        // Create SamplerState
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

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::GetTextureResourceView

      Summary:  Constructor

      Returns:  ComPtr<ID3D11ShaderResourceView>&
                  Shader resource view
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11ShaderResourceView>& Texture::GetTextureResourceView()
    {
        return m_textureRV;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::GetSamplerState

      Summary:  Constructor

      Returns:  ComPtr<ID3D11SamplerState>&
                  Sampler state
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11SamplerState>& Texture::GetSamplerState()
    {
        return m_samplerLinear;
    }
    
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::GetSamplerType

      Summary:  Returns the sampler type

      Returns:  eTextureSamplerType
                  Sampler type
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    eTextureSamplerType Texture::GetSamplerType() const
    {
        return m_textureSamplerType;
    }
}
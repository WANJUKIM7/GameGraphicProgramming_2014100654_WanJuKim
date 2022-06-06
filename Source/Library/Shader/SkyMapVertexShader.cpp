#include "Shader/SkyMapVertexShader.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   SkyMapVertexShader::SkyMapVertexShader

      Summary:  Constructor

      Args:     PCWSTR pszFileName
                  Name of the file that contains the shader code
                PCSTR pszEntryPoint
                  Name of the shader entry point functino where shader
                  execution begins
                PCSTR pszShaderModel
                  Specifies the shader target or set of shader features
                  to compile against
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    SkyMapVertexShader::SkyMapVertexShader(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR pszShaderModel)
        : VertexShader(pszFileName, pszEntryPoint, pszShaderModel)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   SkyMapVertexShader::Initialize

      Summary:  Initializes the vertex shader and the input layout

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the vertex shader

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT SkyMapVertexShader::Initialize(_In_ ID3D11Device* pDevice)
    {
        if (pDevice == nullptr)
            return E_INVALIDARG;

        // Create VertexShader
        ComPtr<ID3DBlob> pVSBlob = nullptr;
        HRESULT hr = compile(pVSBlob.GetAddressOf());

        if (FAILED(hr))
        {
            MessageBox(nullptr, L"The VS shader cannot be compiled.", L"Error", MB_OK);
            return hr;
        }

        hr = pDevice->CreateVertexShader(
            pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf());
        if (FAILED(hr))
            return hr;

        // Create InputLayout
        D3D11_INPUT_ELEMENT_DESC alayouts[] =
        {
            {"POSITION", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u},
        };
        UINT uNumElements = ARRAYSIZE(alayouts);

        hr = pDevice->CreateInputLayout(
            alayouts,
            uNumElements,
            pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(),
            m_vertexLayout.GetAddressOf()
        );

        if (FAILED(hr))
            return hr;

        return hr;
    }
}
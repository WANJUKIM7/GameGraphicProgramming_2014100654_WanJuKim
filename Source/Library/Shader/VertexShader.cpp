#include "Shader/VertexShader.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   VertexShader::VertexShader

      Summary:  Constructor

      Args:     PCWSTR pszFileName
                  Name of the file that contains the shader code
                PCSTR pszEntryPoint
                  Name of the shader entry point functino where shader
                  execution begins
                PCSTR pszShaderModel
                  Specifies the shader target or set of shader features
                  to compile against

      Modifies: [m_vertexShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    VertexShader::VertexShader(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR pszShaderModel)
        : Shader(pszFileName, pszEntryPoint, pszShaderModel)
        , m_vertexShader(nullptr)
        , m_vertexLayout(nullptr)
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   VertexShader::Initialize

      Summary:  Initializes the vertex shader and the input layout

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the vertex shader

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT VertexShader::Initialize(_In_ ID3D11Device* pDevice)
    {
        if (pDevice == nullptr)
            return E_FAIL;

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
        D3D11_INPUT_ELEMENT_DESC layouts[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},

            // instance // Question : 하나의 inputlayout을 공유해도 되나? 
            // InputSlot에 따라 다른 버퍼에 넣을 수 있다!! ex. 0은 buffer[0], 1은 buffer[1] 등등!! → 아예 다른 shader을 사용할 수 있는 근거?! 놀라운 기술 ㄷㄷ.
            // InputSlotClass에 따라 각 vertex마다, 혹은 instance마다 진행할 것인지 알려줌!!
            // InstanceDataStepRate은 다음 instance buffer를 그리기 전에 그려야 하는 instance의 갯수.
            // QUESTION : 나무 400개 그리고 나서 나뭇잎 1000개를 그린다면 뭐 어떻게 한다는 거? 아니면 2번씩 200번 그릴 수 있다는 건가?
            /*{"INSTANCE_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
            {"INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            {"INSTANCE_NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1},*/ // TIP : 버퍼를 2번 나눠서 전달하니까 이런 게 필요 없는 거구나...
            // SemanticName이랑 SemanticIndex랑 모두 같게하니까 SemanticName이 같다고 warning 나오고, SemanticIndex를 바꾸니까 워닝 안 나오네? 잘 전달되는지가 의문.
            {"INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            {"INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            {"INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            {"INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        };
        UINT uNumElements = ARRAYSIZE(layouts);

        hr = pDevice->CreateInputLayout(
            layouts,
            uNumElements,
            pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(),
            m_vertexLayout.GetAddressOf()
        );

        if (FAILED(hr))
            return hr;

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   VertexShader::GetVertexShader

      Summary:  Returns the vertex shader

      Returns:  ComPtr<ID3D11VertexShader>&
                  Vertex shader. Could be a nullptr
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11VertexShader>& VertexShader::GetVertexShader()
    {
        return m_vertexShader;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   VertexShader::GetVertexLayout

      Summary:  Returns the vertex input layout

      Returns:  ComPtr<ID3D11InputLayout>&
                  Vertex input layout
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11InputLayout>& VertexShader::GetVertexLayout()
    {
        return m_vertexLayout;
    }
}
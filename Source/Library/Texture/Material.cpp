#include "Texture/Material.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Material::Material

      Summary:  Constructor

      Modifies: [pDiffuse, pSpecular].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Material::Material()
        : pDiffuse(std::make_shared<Texture>(L""))
        , pSpecular(nullptr)
    {
    }
}
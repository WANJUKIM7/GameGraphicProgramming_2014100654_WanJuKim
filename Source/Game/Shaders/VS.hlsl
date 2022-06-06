#include "Shaders.fxh"
//#include "PhongShaders.fxh"
//#include "VoxelShaders.fxh"
//#include "SkinningShaders.fxh"
//Question : PhongShaders를 포함하지 않았는데도 Cube들이 잘 나오는 이유는? → Shader를 include하지 않아도 된다?
//Shader가 여러 개일 때 굳이 모든 Shader를 include 할 필요 없고, 하나만 포함하고 그 fxh에 있는 함수 hlsl의 '진입점 이름'으로 적어주면 된다.
//include한 파일내에 없는 함수를 '진입점 이름'으로 적으면 에러가 남.

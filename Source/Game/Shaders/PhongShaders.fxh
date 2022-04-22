//--------------------------------------------------------------------------------------
// File: PhongShaders.fx
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
Texture2D TxDiffuse : register(ps, t0);
SamplerState SamLinear : register(ps, s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation and shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement : register(b0)
{
    matrix View;
    float4 CameraPosition;
}

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize

  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnResize : register(b1)
{
    matrix Projection;
}

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangesEveryFrame : register(b2)
{
    matrix World;
    float4 OutputColor;
}

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights

  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbLights : register(b3)
{
    float4 LightPosition[NUM_LIGHTS];
    float4 LightColors[NUM_LIGHTS];
}

//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_PHONG_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_PHONG_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_PHONG_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_PHONG_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 WorldPosition : WORLDPOS;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_LIGHT_CUBE_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_LIGHT_CUBE_INPUT
{
    float4 Position : SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_PHONG_INPUT VSPhong(VS_PHONG_INPUT input)
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT) 0;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.TexCoord = input.TexCoord;
    
    output.Normal = normalize(mul(float4(input.Normal, 1), World).xyz);
    
    output.WorldPosition = mul(input.Position, World);
    //Question : 뭔가 받아들이기가 어렵다. 월드 공간에 있는 건 알겠는데, 공간에서 어디 있다는 거지? VIEW & PROJECTION 곱한 거랑 차이는? 
    //그냥 월드에 있다고 하자;; 절두체 안에만 안 들어오는 건가?
    
    return output;
}

PS_LIGHT_CUBE_INPUT VSLightCube(VS_PHONG_INPUT input)
{
    PS_LIGHT_CUBE_INPUT output = (PS_LIGHT_CUBE_INPUT) 0;
    
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSPhong(PS_PHONG_INPUT input) : SV_Target
{
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f); //TIP : 와 이거 초기화 안 하니까 컴파일 안되네;;
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);
    
    //Helper Code
    for (uint i = 0; i < 2; i++)
    {
        //diffuse
        float3 lightDirection = normalize(input.WorldPosition - LightPosition[i].xyz);
        //Question : 월드 공간에 있는 거랑 지역 공간에 있는 거랑 계산 가능?
        //LightPosition 자체가 월드 공간의 좌표다. 이유는? (0,0,0)을 기준으로 Position을 만들었기 때문. Vector4 쓴다 뭐 그런 게 아니고.
        diffuse += saturate(dot(input.Normal, -lightDirection) * LightColors[i].xyz);
        //QUESTION : saturate를 해줘야 하는 이유?
        
        //specular
        float3 reflectDirection = reflect(lightDirection, input.Normal);
        specular += pow(saturate(dot(-viewDirection, reflectDirection)), 20.0f) * LightColors[i]; //오 불빛이 지나간다. ㄷㄷ 숫자가 커질수록 작아지네.
    }
    diffuse += specular;
    diffuse += ambient;
    diffuse *= TxDiffuse.Sample(SamLinear, input.TexCoord);
    return float4(diffuse, 1.0f);
}

float4 PSLightCube(PS_LIGHT_CUBE_INPUT input) : SV_Target
{
    return OutputColor;
}

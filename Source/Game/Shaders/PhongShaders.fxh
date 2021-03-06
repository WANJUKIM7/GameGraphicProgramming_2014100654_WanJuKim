//--------------------------------------------------------------------------------------
// File: PhongShaders.fx
//
// Copyright (c) Kyung Hee University.
//--------------------------------------------------------------------------------------

#define NUM_LIGHTS (2)
#define NEAR_PLANE (0.01f)
#define FAR_PLANE (1000.0f)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D shadowMapTexture : register(t2);
SamplerState diffuseSampler : register(s0);
SamplerState normalSampler : register(s1);
SamplerState shadowMapSampler : register(s2);

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
    bool HasNormalMap;
};

struct PointLight
{
    float4 Position;
    float4 Color;
    float4 AttenuationDistance;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights
  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbLights : register(b3)
{
    //float4 LightPositions[NUM_LIGHTS];
    //float4 LightColors[NUM_LIGHTS];
    //matrix LightViews[NUM_LIGHTS];
    //matrix LightProjections[NUM_LIGHTS];
    PointLight PointLights[NUM_LIGHTS];
}

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_PHONG_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_PHONG_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    row_major matrix mTransform : INSTANCE_TRANSFORM;
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
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    //float4 LightViewPosition : TEXCOORD1;
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
    
    output.Normal = normalize(mul(float4(input.Normal, 1.0f), World).xyz);
    
    output.WorldPosition = mul(input.Position, World);
    //Question : ???? ???????????? ??????. ???? ?????? ???? ?? ????????, ???????? ???? ?????? ????? VIEW & PROJECTION ???? ???? ??????? 
    //???? ?????? ?????? ????;; ?????? ?????? ?? ???????? ?????
    
	if (HasNormalMap)
	{
        output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }
    
    //output.LightViewPosition = mul(input.Position, World);
    //output.LightViewPosition = mul(output.LightViewPosition, LightViews[0]);
    //output.LightViewPosition = mul(output.LightViewPosition, LightProjections[0]);
    
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

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return ((2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE))) / FAR_PLANE;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSPhong(PS_PHONG_INPUT input) : SV_Target
{
    // Shadow
    //float4 color = diffuseTexture.Sample(diffuseSampler, input.TexCoord);
    //float3 ambient = float3(0.1f, 0.1f, 0.1f) * color.rgb;
    //float2 depthTexCoord = float2(0.0f, 0.0f);
    //depthTexCoord.x = input.LightViewPosition.x / input.LightViewPosition.w / 2.0f + 0.5f;
    //depthTexCoord.y = -input.LightViewPosition.y / input.LightViewPosition.w / 2.0f + 0.5f;
    //float closestDepth = shadowMapTexture.Sample(shadowMapSampler, depthTexCoord).r;
    //float currentDepth = input.LightViewPosition.z / input.LightViewPosition.w;
    //closestDepth = LinearizeDepth(closestDepth);
    //currentDepth = LinearizeDepth(currentDepth);
    //if (currentDepth > closestDepth + 0.001f)
    //    return float4(ambient, 1.0f);
    
    // Phong
    float4 color = diffuseTexture.Sample(diffuseSampler, input.TexCoord);
    float3 ambient = float3(0.1f, 0.1f, 0.1f) * color.rgb;
    float3 diffuse = float3(0.0f, 0.0f, 0.0f); //TIP : ?? ???? ?????? ?? ?????? ?????? ??????;;
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);
    float3 normal = normalize(input.Normal);
    
    // Normal
    if (HasNormalMap)
    {
         // Sample the pixel in the normal map.
        float4 bumpMap = normalTexture.Sample(normalSampler, input.TexCoord);
         // Expand the range of the normal value from (0, +1) to (-1, +1).
        bumpMap = (bumpMap * 2.0f) - 1.0f;
         // Calculate the normal from the data in the normal map.
        float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);
         // Normalize the resulting bump normal and replace existing normal
        normal = normalize(bumpNormal);
    }
    
    // diffuse & specular
    //for (uint i = 0u; i < NUM_LIGHTS; i++)
    //{
    //    //diffuse
    //    float3 lightDirection = normalize(input.WorldPosition - LightPositions[i].xyz);
    //    //Question : ???? ?????? ???? ???? ???? ?????? ???? ???? ???? ?????
    //    //LightPosition ?????? ???? ?????? ??????. ??????? (0,0,0)?? ???????? Position?? ???????? ????. Vector4 ???? ?? ???? ?? ??????.
    //    diffuse += saturate(dot(normal, -lightDirection) * LightColors[i].xyz);
    //    //QUESTION : saturate?? ?????? ???? ?????
        
    //    //specular
    //    float3 reflectDirection = normalize(reflect(lightDirection, input.Normal));
    //    specular += pow(saturate(dot(-viewDirection, reflectDirection)), 20.0f) * LightColors[i]; //?? ?????? ????????. ???? ?????? ???????? ????????.
    //}
    
    // attenuation
    float attenuation = float(0.0f);
    for (uint i = 0u; i < NUM_LIGHTS; i++)
    {
        float distanceSquared = dot(input.Position.xyz, input.WorldPosition);
        float epsilon = 0.000001f;
        attenuation += PointLights[i].AttenuationDistance.z / (epsilon);
    }
    
    ambient += attenuation;
    return float4(diffuse + specular + ambient, 1.0f);
}

float4 PSLightCube(PS_LIGHT_CUBE_INPUT input) : SV_Target
{
    return OutputColor;
}
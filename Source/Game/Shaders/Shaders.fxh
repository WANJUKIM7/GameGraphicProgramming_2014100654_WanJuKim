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
TextureCube EnvironmentMap : register(t2);
SamplerState diffuseSampler : register(s0);
SamplerState normalSampler : register(s1);

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
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    row_major matrix mTransform : INSTANCE_TRANSFORM;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_INPUT
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
PS_INPUT VSPhong(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.TexCoord = input.TexCoord;
    
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), World).xyz);
    
    output.WorldPosition = mul(input.Position, World);
    //Question : 뭔가 받아들이기가 어렵다. 월드 공간에 있는 건 알겠는데, 공간에서 어디 있다는 거지? VIEW & PROJECTION 곱한 거랑 차이는? 
    //그냥 월드에 있다고 하자;; 절두체 안에만 안 들어오는 건가?
    
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

PS_LIGHT_CUBE_INPUT VSLightCube(VS_INPUT input)
{
    PS_LIGHT_CUBE_INPUT output = (PS_LIGHT_CUBE_INPUT) 0;
    
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    return output;
}

PS_INPUT VSVoxel(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    output.Position = mul(input.Position, input.mTransform);
    
    output.WorldPosition = mul(output.Position, World);
    
    output.Position = mul(output.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.TexCoord = input.TexCoord;
    
    output.Normal = normalize(mul(float4(input.Normal, 1.0f), World).xyz);
    
    if (HasNormalMap)
    {
        output.Tangent = normalize(mul(float4(input.Tangent, 0.0f), World).xyz);
        output.Bitangent = normalize(mul(float4(input.Bitangent, 0.0f), World).xyz);
    }
    
    return output;
}

PS_INPUT VSEnvironmentMap(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Projection);
    
    output.WorldPosition = mul(input.Position, World);
    
    output.TexCoord = input.TexCoord;
    
    output.Normal = input.Normal;
    
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
float4 PSPhong(PS_INPUT input) : SV_Target
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
    float3 normal = normalize(input.Normal);
    
    // Normal
    //if (HasNormalMap)
    //{
    //     // Sample the pixel in the normal map.
    //    float4 bumpMap = normalTexture.Sample(normalSampler, input.TexCoord);
    //     // Expand the range of the normal value from (0, +1) to (-1, +1).
    //    bumpMap = (bumpMap * 2.0f) - 1.0f;
    //     // Calculate the normal from the data in the normal map.
    //    float3 bumpNormal = (bumpMap.x * input.Tangent) + (bumpMap.y * input.Bitangent) + (bumpMap.z * normal);
    //     // Normalize the resulting bump normal and replace existing normal
    //    normal = normalize(bumpNormal);
    //}
    
    // ambient
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    for (uint i = 0u; i < NUM_LIGHTS; ++i)
    {
        ambient += float4(float3(0.1f, 0.1f, 0.1f) * PointLights[i].Color.xyz, 1.0f);
    }
    
    // diffuse
    float3 lightDirection = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f); //TIP : 와 이거 초기화 안 하니까 컴파일 안되네;;
    for (uint j = 0u; j < NUM_LIGHTS; ++j)
    {
        lightDirection = normalize(PointLights[j].Position.xyz - input.WorldPosition);
        diffuse += saturate(dot(normal, lightDirection)) * PointLights[j].Color.xyz;
    }
    
    // specular
    float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float3 reflectDirection = float3(0.0f, 0.0f, 0.0f);
    float shiness = 20.0f;
    for (uint k = 0; k < NUM_LIGHTS; ++k)
    {
        lightDirection = normalize(PointLights[k].Position.xyz - input.WorldPosition);
        reflectDirection = reflect(-lightDirection, normal);
        specular += pow(saturate(dot(reflectDirection, viewDirection)), shiness) * PointLights[k].Color;
    }
    
    // diffuse & specular
    //for (uint i = 0u; i < NUM_LIGHTS; i++)
    //{
    //    //diffuse
    //    float3 lightDirection = normalize(input.WorldPosition - PointLights[i].Position.xyz);
    //    //Question : 월드 공간에 있는 거랑 지역 공간에 있는 거랑 계산 가능?
    //    //LightPosition 자체가 월드 공간의 좌표다. 이유는? (0,0,0)을 기준으로 Position을 만들었기 때문. Vector4 쓴다 뭐 그런 게 아니고.
    //    diffuse += saturate(dot(normal, -lightDirection) * PointLights[i].Color.xyz);
    //    //QUESTION : saturate를 해줘야 하는 이유?
        
    //    //specular
    //    float3 reflectDirection = normalize(reflect(lightDirection, input.Normal));
    //    specular += pow(saturate(dot(-viewDirection, reflectDirection)), 20.0f) * PointLights[i].Color.xyz; //오 불빛이 지나간다. ㄷㄷ 숫자가 커질수록 작아지네.
    //}
    
    // attenuation
    float attenuation = float(0.0f);
    for (uint l = 0u; l < NUM_LIGHTS; l++)
    {
        float distanceSquared = dot(input.Position.xyz, PointLights[l].Position.xyz);
        float epsilon = 0.000001f;
        attenuation += PointLights[l].AttenuationDistance.z / (distanceSquared + epsilon);
    }
    
    ambient += attenuation;
    return float4(diffuse + specular + ambient, 1.0f) * diffuseTexture.Sample(diffuseSampler, input.TexCoord);
}

float4 PSLightCube(PS_LIGHT_CUBE_INPUT input) : SV_Target
{
    return OutputColor;
}

float4 PSVoxel(PS_INPUT input) : SV_Target
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
    float3 diffuse = float3(0.0f, 0.0f, 0.0f); //TIP : 와 이거 초기화 안 하니까 컴파일 안되네;;
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
    //    //Question : 월드 공간에 있는 거랑 지역 공간에 있는 거랑 계산 가능?
    //    //LightPosition 자체가 월드 공간의 좌표다. 이유는? (0,0,0)을 기준으로 Position을 만들었기 때문. Vector4 쓴다 뭐 그런 게 아니고.
    //    diffuse += saturate(dot(normal, -lightDirection) * LightColors[i].xyz);
    //    //QUESTION : saturate를 해줘야 하는 이유?
        
    //    //specular
    //    float3 reflectDirection = normalize(reflect(lightDirection, input.Normal));
    //    specular += pow(saturate(dot(-viewDirection, reflectDirection)), 20.0f) * LightColors[i]; //오 불빛이 지나간다. ㄷㄷ 숫자가 커질수록 작아지네.
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

float4 PSEnvironmentMap(PS_INPUT input) : SV_Target
{
    float3 viewDirection = normalize(input.WorldPosition - CameraPosition.xyz);

	// Reflection Vector
    float3 reflectionVector = normalize(reflect(viewDirection, input.Normal));
    
    float3 environment = EnvironmentMap.Sample(diffuseSampler, reflectionVector).rgb;
    
    return float4(environment, 1.0f);
}
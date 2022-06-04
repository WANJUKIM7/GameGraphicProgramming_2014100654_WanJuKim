#pragma once

#include "Common.h"

namespace library
{
    #define NUM_LIGHTS (1)
    #define MAX_NUM_BONES (256)
    #define MAX_NUM_BONES_PER_VERTEX (16)

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:    SimpleVertex

      Summary:  Simple vertex structure containing a single field of the
                type XMFLOAT3
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct SimpleVertex
    {
        XMFLOAT3 Position;
        XMFLOAT2 TexCoord;
        XMFLOAT3 Normal;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   InstanceData

      Summary:  Instance data containing a per instance transformation
                matrix
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct InstanceData
    {
        XMMATRIX Transformation;
    };

    struct AnimationData
    {
        XMUINT4 aBoneIndices;
        XMFLOAT4 aBoneWeights;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   NormalData

      Summary:  NormalData structure containing tangent space vetors
                of the vertex
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct NormalData
    {
        XMFLOAT3 Tangent;
        XMFLOAT3 Bitangent;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   CBChangeOnCameraMovement

      Summary:  Constant buffer containing view matrix
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct CBChangeOnCameraMovement
    {
        XMMATRIX View;
        XMFLOAT4 CameraPosition;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   CBChangeOnResize

      Summary:  Constant buffer containing projection matrix
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct CBChangeOnResize
    {
        XMMATRIX Projection;
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   CBChangesEveryFrame

      Summary:  Constant buffer containing world matrix
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct CBChangesEveryFrame
    {
        XMMATRIX World;
        XMFLOAT4 OutputColor;
        BOOL HasNormalMap;
    };

    struct CBSkinning
    {
        XMMATRIX BoneTransforms[MAX_NUM_BONES];
    };

    /*S+S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S+++S
      Struct:   CBLights

      Summary:  Constant buffer containing lights' information
    S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S---S-S*/
    struct CBLights
    {
        XMFLOAT4 LightPositions[NUM_LIGHTS];
        XMFLOAT4 LightColors[NUM_LIGHTS];
        XMMATRIX LightViews[NUM_LIGHTS];
        XMMATRIX LightProjections[NUM_LIGHTS];
    };

    struct CBShadowMatrix
    {
        XMMATRIX World;
        XMMATRIX View;
        XMMATRIX Projection;
        BOOL IsVoxel;
    };
}
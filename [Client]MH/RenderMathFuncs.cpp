// RenderMathFuncs.cpp: 渲染和数学相关函数
//

#include "stdafx.h"
#include "../[CC]Header/CommonStruct.h"

// VRand - 生成随机向量
void VRand(VECTOR3* pResult, VECTOR3* pOriginPos, VECTOR3* pRandom)
{
    if(!pResult || !pOriginPos || !pRandom) return;

    pResult->x = pOriginPos->x + (rand() % 1000 / 1000.0f - 0.5f) * pRandom->x;
    pResult->y = pOriginPos->y + (rand() % 1000 / 1000.0f - 0.5f) * pRandom->y;
    pResult->z = pOriginPos->z + (rand() % 1000 / 1000.0f - 0.5f) * pRandom->z;
}

// TransToRelatedCoordinate - 转换到相对坐标
void TransToRelatedCoordinate(VECTOR3* pResult, VECTOR3* pOriginPos, float fAngleRadY)
{
    if(!pResult || !pOriginPos) return;

    // 计算旋转后的相对坐标
    float cosAngle = cos(fAngleRadY);
    float sinAngle = sin(fAngleRadY);

    pResult->x = pOriginPos->x * cosAngle - pOriginPos->z * sinAngle;
    pResult->y = pOriginPos->y;
    pResult->z = pOriginPos->x * sinAngle + pOriginPos->z * cosAngle;
}

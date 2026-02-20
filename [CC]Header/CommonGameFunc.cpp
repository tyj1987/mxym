#include "stdafx.h"
#include "CommonGameFunc.h"

// Global variable definitions
BOOL g_bAssertMsgBox = FALSE;

// Additional global variables for date calculations
unsigned long DayOfMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
unsigned long DayOfMonth_Yundal[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// Bit operation functions implementations
void SetOnBit(unsigned char* BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned char)*8) -1 || BitFlag == NULL )
        return;
    *BitFlag |= (1 << nIndex);
}

void SetOnBit(unsigned short* BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned short)*8) -1 || BitFlag == NULL )
        return;
    *BitFlag |= (1 << nIndex);
}

void SetOnBit(unsigned long* BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned long)*8) -1 || BitFlag == NULL )
        return;
    *BitFlag |= (1 << nIndex);
}

void SetOffBit(unsigned char* BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned char)*8) -1 || BitFlag == NULL )
        return;
    *BitFlag &= ~(0x01 << nIndex);
}

void SetOffBit(unsigned short* BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned short)*8) -1 || BitFlag == NULL )
        return;
    *BitFlag &= ~(0x01 << nIndex);
}

void SetOffBit(unsigned long* BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned long)*8) -1 || BitFlag == NULL )
        return;
    *BitFlag &= ~(0x01 << nIndex);
}

BOOL CheckBit(unsigned char BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned char)*8) -1 )
        return FALSE;
    return (BitFlag & (0x01 << nIndex)) != 0;
}

BOOL CheckBit(unsigned short BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned short)*8) -1 )
        return FALSE;
    return (BitFlag & (0x01 << nIndex)) != 0;
}

BOOL CheckBit(unsigned long BitFlag, int nIndex)
{
    if( nIndex < 0 || nIndex > (sizeof(unsigned long)*8) -1 )
        return FALSE;
    return (BitFlag & (0x01 << nIndex)) != 0;
}

// Vector related functions implementations
void SetVector3(VECTOR3* vec, float x, float y, float z)
{
    if( vec != NULL )
    {
        vec->x = x;
        vec->y = y;
        vec->z = z;
    }
}

float CalcDistanceXZ(VECTOR3* v1, VECTOR3* v2)
{
    if( v1 == NULL || v2 == NULL )
        return 0.0f;
    
    float dx = v2->x - v1->x;
    float dz = v2->z - v1->z;
    return sqrtf(dx*dx + dz*dz);
}

void BeelinePoint(VECTOR3* origin, VECTOR3* dir, float dist, VECTOR3* result)
{
    if( origin == NULL || dir == NULL || result == NULL )
        return;
    
    result->x = origin->x + dir->x * dist;
    result->y = origin->y + dir->y * dist;
    result->z = origin->z + dir->z * dist;
}

void AdjacentPointForOriginPoint(VECTOR3* origin, VECTOR3* target, float dist, VECTOR3* result)
{
    if( origin == NULL || target == NULL || result == NULL )
        return;
    
    VECTOR3 dir;
    dir.x = target->x - origin->x;
    dir.y = target->y - origin->y;
    dir.z = target->z - origin->z;
    
    float length = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    if( length > 0.0f )
    {
        float scale = dist / length;
        result->x = origin->x + dir.x * scale;
        result->y = origin->y + dir.y * scale;
        result->z = origin->z + dir.z * scale;
    }
    else
    {
        result->x = origin->x;
        result->y = origin->y;
        result->z = origin->z;
    }
}

void RotateVectorAxisY(VECTOR3* pResult, VECTOR3* pOriginVector, float fAngleRadY)
{
    if( pResult == NULL || pOriginVector == NULL )
        return;
    
    float cosY = cosf(fAngleRadY);
    float sinY = sinf(fAngleRadY);
    
    pResult->x = pOriginVector->x * cosY - pOriginVector->z * sinY;
    pResult->y = pOriginVector->y;
    pResult->z = pOriginVector->x * sinY + pOriginVector->z * cosY;
}

// Date and time functions implementations
DWORD GetCurTime()
{
    return GetTickCount();
}

// Utility functions implementations
BOOL CheckValidPosition(VECTOR3& pos)
{
    // Basic position validation
    return TRUE;
}

// Assert related functions implementations
void CriticalAssertMsg(char* pStrFileName, int Line, char* pMsg)
{
    // Basic implementation
    MessageBox(NULL, pMsg, "Critical Assert", MB_OK | MB_ICONERROR);
}

// Debug functions implementations
void WriteDebugFile(char* pStrFileName, int Line, char* pMsg)
{
    // Basic implementation
    FILE* fp = NULL;
    errno_t err = fopen_s(&fp, "debug.txt", "a");
    if( err == 0 && fp != NULL )
    {
        fprintf(fp, "%s(%d): %s\n", pStrFileName, Line, pMsg);
        fclose(fp);
    }
}

// Effect related functions implementations
int FindEffectNum(char* pFileName)
{
    return 0;
}

// Game logic functions implementations
eObjectKindGroup GetObjectKindGroup(WORD wObjectKind)
{
    return static_cast<eObjectKindGroup>(0);
}

eITEMTABLE GetTableIdxPosition(WORD abs_pos)
{
    return static_cast<eITEMTABLE>(0);
}

eITEM_KINDBIT GetItemKind(WORD iconIdx)
{
    return static_cast<eITEM_KINDBIT>(0);
}

eQUICKICON_KIND GetIconKind(WORD wIdx)
{
    return static_cast<eQUICKICON_KIND>(0);
}

POSTYPE ConvAbsPos2MugongPos(POSTYPE abspos)
{
    return abspos;
}

// Target related functions implementations
BOOL GetMainTargetPos(MAINTARGET* pMainTarget, VECTOR3* pRtPos, CObject** ppObject)
{
    return FALSE;
}

// Time related functions implementations
char* GetCurTimeToString()
{
    static char szTime[32] = "";
    SYSTEMTIME st;
    GetLocalTime(&st);
    _snprintf_s(szTime, sizeof(szTime), "%04d-%02d-%02d %02d:%02d:%02d", 
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return szTime;
}

DWORD Convert2MilliSecond(DWORD Day, DWORD Hour, DWORD Minute, DWORD Second)
{
    return ((Day * 24 + Hour) * 60 + Minute) * 60 * 1000 + Second * 1000;
}

// Other functions implementations (stubs)
eMUGONGITEM_KIND GetItemKindDetail(WORD iconIdx)
{
    return static_cast<eMUGONGITEM_KIND>(0);
}

WP_KIND GetWeaponType(WORD mugongIdx)
{
    return static_cast<WP_KIND>(0);
}

DWORD GetMainTargetID(MAINTARGET* pMainTarget)
{
    return 0;
}

void SetProtocol(MSGBASE* pMsg, BYTE bCategory, BYTE bProtocol)
{
    if( pMsg != NULL )
    {
        // Direct access to the first two bytes of MSGBASE structure
        *((BYTE*)pMsg + 0) = bCategory;
        *((BYTE*)pMsg + 1) = bProtocol;
    }
}

void SendAssertMsg(char* pStrFileName, int Line, char* pMsg)
{
    // Basic implementation
}

void WriteAssertMsg(char* pStrFileName, int Line, char* pMsg)
{
    // Basic implementation
}

DWORD GetCurTimeValue()
{
    return GetTickCount();
}

DWORD GetCurTimeToSecond()
{
    return GetTickCount() / 1000;
}

void StringtimeToSTTIME(stTIME* Time, char* strTime)
{
    // Basic implementation
}

void LoadEffectFileTable(char* pListFileName)
{
    // Basic implementation
}

BOOL IsVillage()
{
    return FALSE;
}

void SafeStrCpy(char* pDest, const char* pSrc, int nDestBufSize)
{
    if( pDest != NULL && pSrc != NULL && nDestBufSize > 0 )
    {
        strcpy_s(pDest, nDestBufSize, pSrc);
    }
}

void SafeStrCpyEmptyChange(char* pDest, const char* pSrc, int nDestBufSize)
{
    SafeStrCpy(pDest, pSrc, nDestBufSize);
}

float roughGetLength(float fDistX, float fDistY)
{
    return sqrtf(fDistX*fDistX + fDistY*fDistY);
}

void ERRORBSGBOX(char* str, ...)
{
    // Basic implementation
}

const char* GetDay(WORD nDay)
{
    return "Monday";
}

WORD GetDay(const char* strDay)
{
    return 0;
}

const char* GetWeather(WORD nWeatherState)
{
    return "Clear";
}

WORD GetWeather(const char* strWeather)
{
    return 0;
}

float GetAlphaValue(DWORD dwTime, DWORD dwStartTime, BOOL bFlag)
{
    return 1.0f;
}
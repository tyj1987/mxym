#ifndef __COMMONGAMEFUNC_H__
#define __COMMONGAMEFUNC_H__

// ----------------------------------------------------------------------------------
// Common global functions for both client and server
// ----------------------------------------------------------------------------------

#include "CommonStruct.h"
#include "vector.h"

class CObject;

// Ensure min/max functions are available
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>

// Global variable declarations
extern BOOL g_bAssertMsgBox;
extern char g_szHeroIDName[];

// Time related functions
char* GetCurTimeToString();
#ifdef _MHCLIENT_
char* GetCurTimeToString(DWORD milliSec);
char* GetCurTimeToString(stTIME* stTime);
#endif
DWORD GetCurTimeValue();
DWORD GetCurTime();
DWORD GetCurTimeToSecond();
void StringtimeToSTTIME(stTIME* Time, char* strTime);

// Network related functions
void SetProtocol(MSGBASE* pMsg,BYTE bCategory,BYTE bProtocol);

// Assert related functions
void SendAssertMsg(char* pStrFileName,int Line,char* pMsg);
void WriteAssertMsg(char* pStrFileName,int Line,char* pMsg);
void CriticalAssertMsg(char* pStrFileName,int Line,char* pMsg);

// 游戏逻辑函数
WP_KIND GetWeaponType(WORD mugongIdx);
eITEMTABLE GetTableIdxPosition(WORD abs_pos);
eQUICKICON_KIND GetIconKind(WORD wIdx);
eITEM_KINDBIT GetItemKind(WORD iconIdx);
eMUGONGITEM_KIND GetItemKindDetail(WORD iconIdx);
POSTYPE ConvAbsPos2MugongPos(POSTYPE abspos);

// Target related functions
DWORD GetMainTargetID(MAINTARGET* pMainTarget);
BOOL GetMainTargetPos(MAINTARGET* pMainTarget,VECTOR3* pRtPos, CObject ** ppObject);

// Date related functions
BOOL IsSameDay(char* strDay,WORD Year,WORD Month,WORD Day);

// Vector related functions
void SetVector3(VECTOR3* vec,float x,float y,float z);
float CalcDistanceXZ(VECTOR3* v1,VECTOR3* v2);
void VRand(VECTOR3* pResult,VECTOR3* pOriginPos,VECTOR3* pRandom);
void TransToRelatedCoordinate(VECTOR3* pResult,VECTOR3* pOriginPos,float fAngleRadY);
void RotateVectorAxisX(VECTOR3* pResult,VECTOR3* pOriginVector,float fAngleRadX);
void RotateVectorAxisY(VECTOR3* pResult,VECTOR3* pOriginVector,float fAngleRadY);
void RotateVectorAxisZ(VECTOR3* pResult,VECTOR3* pOriginVector,float fAngleRadZ);
void RotateVector( VECTOR3* pResult, VECTOR3* pOriginVector, float x, float y, float z );
void RotateVector( VECTOR3* pResult, VECTOR3* pOriginVector, VECTOR3 vRot );
void BeelinePoint(VECTOR3 * origin, VECTOR3 * dir, float dist, VECTOR3 * result);
void AdjacentPointForOriginPoint(VECTOR3 * origin, VECTOR3 * target, float dist, VECTOR3 * result);

// Client specific functions
#ifdef _MHCLIENT_
void AddComma( char* pBuf );
char* AddComma( DWORD dwMoney );
char* RemoveComma( char* str );
#endif

// Common utility functions
BOOL IsVillage();
void SafeStrCpy( char* pDest, const char* pSrc, int nDestBufSize );
void SafeStrCpyEmptyChange( char* pDest, const char* pSrc, int nDestBufSize );
BOOL CheckValidPosition(VECTOR3& pos);
float roughGetLength( float fDistX,float fDistY );

// Effect related functions
void LoadEffectFileTable(char* pListFileName);
int FindEffectNum(char* pFileName);

// Assert macros
#define ASSERTVALID_POSITION(vec)	CheckValidPosition(vec)

// Error handling
void ERRORBSGBOX(char* str,...);

// Date and weather functions
const char* GetDay(WORD nDay);
WORD GetDay(const char* strDay);
const char* GetWeather(WORD nWeatherState);
WORD GetWeather(const char* strWeather);
DWORD Convert2MilliSecond( DWORD Day, DWORD Hour, DWORD Minute, DWORD Second );

// Bit operation function declarations
void SetOnBit(unsigned char* BitFlag, int nIndex);
void SetOnBit(unsigned short* BitFlag, int nIndex);
void SetOnBit(unsigned long* BitFlag, int nIndex);

void SetOffBit(unsigned char* BitFlag, int nIndex);
void SetOffBit(unsigned short* BitFlag, int nIndex);
void SetOffBit(unsigned long* BitFlag, int nIndex);

BOOL CheckBit(unsigned char BitFlag, int nIndex);
BOOL CheckBit(unsigned short BitFlag, int nIndex);
BOOL CheckBit(unsigned long BitFlag, int nIndex);

// 2007. 10. 25. CBH - Added object kind group function
eObjectKindGroup GetObjectKindGroup(WORD wObjectKind);

// Effect related functions
float GetAlphaValue(DWORD dwTime, DWORD dwStartTime, BOOL bFlag);

// Debug functions
void WriteDebugFile( char* pStrFileName, int Line, char* pMsg );

#endif //__COMMONGAMEFUNC_H__

// WeatherManager.cpp: implementation of the CWeatherManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerSystem.h"
#include "WeatherManager.h"
#include "Player.h"
#include "MhFile.h"
#include "..\[cc]header\GameResourceManager.h"
#include "UserTable.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//#ifdef _JAPAN_LOCAL_
GLOBALTON(CWeatherManager);
//#endif

CWeatherManager::CWeatherManager()
{
	m_dwLastCheckTime = 0;

	m_wState = eWS_None;

	m_wEventCount = 0;
	m_bExecution = FALSE;
	
	m_pWeatherSchedulePool = new CMemoryPoolTempl<stWeatherTime>;
	m_pWeatherSchedulePool->Init( 50, 10, "stWeatherTimePool" );
	m_WeatherSchedule.Initialize( 50 );

	m_pEventSchedulePool = new CMemoryPoolTempl<stWeatherTime>;
	m_pEventSchedulePool->Init( 50, 10, "stWeatherTimePool" );
	m_EventSchedule.Initialize( 50 );
	
}

CWeatherManager::~CWeatherManager()
{
	m_WeatherSchedule.SetPositionHead();
	m_EventSchedule.SetPositionHead();
	
	stWeatherTime* pData = NULL;
	
	while( pData = m_WeatherSchedule.GetData() )
	{
		m_pWeatherSchedulePool->Free( pData );
	}
	m_WeatherSchedule.RemoveAll();
	SAFE_DELETE( m_pWeatherSchedulePool );

	while( pData = m_EventSchedule.GetData() )
	{
		m_pEventSchedulePool->Free( pData );
	}
	m_EventSchedule.RemoveAll();
	SAFE_DELETE( m_pEventSchedulePool );
}

void CWeatherManager::Init()
{
	CMHFile file;
	
#ifdef _FILE_BIN_
	if(!file.Init("Resource/Server/Weather.bin","rb"))
		return;
#else
	if(!file.Init("Resource/Server/Weather.bin","rt"))
		return;
#endif

	char buff[256] ={0,};

	while( !file.IsEOF() )
	{
		file.GetString( buff );

		if( strcmp(buff,"#MAPNUM") == 0 )
		{
			int mapnum = file.GetInt();
			if( mapnum == GAMERESRCMNGR->GetLoadMapNum() )
			{
                m_wStateCount = file.GetWord();

                stWeatherTime* pData = NULL;
                for( DWORD i=0; i<m_wStateCount; ++i )
				{
					pData = m_pWeatherSchedulePool->Alloc();

                    pData->Index = file.GetWord();
                    pData->StartDay = file.GetWord();
                    pData->StartHour = file.GetWord();
                    pData->StartMinute = file.GetWord();
                    pData->EndDay = file.GetWord();
                    pData->EndHour = file.GetWord();
                    pData->EndMinute = file.GetWord();
                    pData->State = file.GetWord();
                    
                    m_WeatherSchedule.Add( pData, pData->Index );
				}
			}
		}
	}
	
	file.Release();
}
/*
void CWeatherManager::Init()
{
	CMHFile file;
	char filename[256];
#ifdef _FILE_BIN_
	//	sprintf(filename,"Resource/Weather_%02d.bin",GAMERESRCMNGR->GetLoadMapNum());
	sprintf(filename,"Resource/Server/Weather_%02d.bin",GAMERESRCMNGR->GetLoadMapNum());
	if(!file.Init(filename,"rb"))
		return ;
#else
	sprintf(filename,"Resource/Weather_%02d.txt",GAMERESRCMNGR->GetLoadMapNum());
	if(!file.Init(filename,"rt"))
		return ;
#endif

	m_wStateCount = file.GetWord();
	stWeatherTime* pData = NULL;

	for( DWORD i=0; i<m_wStateCount; ++i )
	{
		pData = m_pWeatherSchedulePool->Alloc();

		pData->Index = file.GetWord();
		pData->StartDay = file.GetWord();
		pData->StartHour = file.GetWord();
		pData->StartMinute = file.GetWord();
		pData->EndDay = file.GetWord();
		pData->EndHour = file.GetWord();
		pData->EndMinute = file.GetWord();
		pData->State = file.GetWord();

		m_WeatherSchedule.Add( pData, pData->Index );
	}

	file.Release();
}
*/
void CWeatherManager::NetworkMsgParse( BYTE Protocol, void* pMsg )
{
	MSGBASE* pmsg = (MSGBASE*)pMsg;

	CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( pmsg->dwObjectID );
	
	if( !pPlayer )	//   ´...  ǳ;;;
		return;
	
	if( pPlayer->GetUserLevel() > eUSERLEVEL_GM ) // GM ƴ  ´... Ȥó...
		return;

	switch( Protocol )
	{
	case MP_WEATHER_SET:
		{
			MSG_WEATHER* msg = (MSG_WEATHER*)pMsg;

            if( msg->MapNum != g_pServerSystem->GetMapNum() )
				return;

			stWeatherTime* pData = NULL;
			SYSTEMTIME cTime;
			GetLocalTime( &cTime );

			pData = m_pEventSchedulePool->Alloc();
			
			memcpy(pData, &msg->WeatherTime, sizeof(stWeatherTime));

			if( pData->State >= eWS_Max)
			{
				m_pEventSchedulePool->Free(pData);
				return;
			}

			pData->Index = m_wEventCount++;
			
			m_EventSchedule.Add( pData, pData->Index );
		}
		break;
	case MP_WEATHER_EXE:
		{
			MSG_WORD2* msg = (MSG_WORD2*)pMsg;

			if( msg->wData2 != g_pServerSystem->GetMapNum() )
				return;

			m_bExecution = TRUE;

			if( msg->wData1 >= eWS_Max)
				return;

			m_wState = msg->wData1;

			SendAllWeather();
		}
		break;
	case MP_WEATHER_RETURN:
		{
			MSG_WORD* msg = (MSG_WORD*)pMsg;

			if( msg->wData != g_pServerSystem->GetMapNum() )
				return;

			m_bExecution = FALSE;
		}
	}
}

void CWeatherManager::Process()
{
	// 1п ѹ 
	if(gCurTime - m_dwLastCheckTime < 60000)
		return;

	m_dwLastCheckTime = gCurTime;

	if( m_bExecution ) // GM     
		return;

	stWeatherTime* pData = NULL;
	
	SYSTEMTIME cTime;
	GetLocalTime( &cTime );
	WORD wState = m_wState;
	BOOL bState = FALSE;
	
	m_WeatherSchedule.SetPositionHead();
	
	//  ⺻ õ 
	while( pData = m_WeatherSchedule.GetData() )
	{
		//   Ȳ...
		//   
		if( pData->StartDay > cTime.wDayOfWeek )
			continue;
		
		//  ð 
		if( pData->StartDay == cTime.wDayOfWeek && pData->StartHour > cTime.wHour )
			continue;
		
		//   
		if( pData->StartHour == cTime.wHour && pData->StartMinute > cTime.wMinute )
			continue;
		
		// ̹  Ȳ...
		//  
		if( pData->EndDay < cTime.wDayOfWeek )
			continue;
		
		// ð 
		if( pData->EndDay == cTime.wDayOfWeek && pData->EndHour < cTime.wHour )
			continue;
		
		//  
		if( pData->EndHour == cTime.wHour && pData->EndMinute < cTime.wMinute )
			continue;		
		
		wState = pData->State;
		bState = TRUE;
	}
	
	m_EventSchedule.SetPositionHead();
	
	// GM  õ 
	while( pData = m_EventSchedule.GetData() )
	{
		BOOL bPassed = FALSE;
		
		//   Ȳ...
		//   
		if( pData->StartDay > cTime.wDayOfWeek )
			continue;
		
		//  ð 
		if( pData->StartDay == cTime.wDayOfWeek && pData->StartHour > cTime.wHour )
			continue;
		
		//   
		if( pData->StartHour == cTime.wHour && pData->StartMinute > cTime.wMinute )
			continue;
		
		// ̹  Ȳ...
		//  
		if( pData->EndDay < cTime.wDayOfWeek )
			bPassed = TRUE;
		
		// ð 
		if( pData->EndDay == cTime.wDayOfWeek && pData->EndHour < cTime.wHour )
			bPassed = TRUE;
		
		//  
		if( pData->EndHour == cTime.wHour && pData->EndMinute < cTime.wMinute )
			bPassed = TRUE;
		
		if( bPassed )
		{
			//  ̸  Ѵ
			m_EventSchedule.Remove( pData->Index );
			
			continue;
		}
		
		wState = pData->State;
		bState = TRUE;
	}
	
	// 쿡   Ͱ   
	if( !bState )
		wState = eWS_None;

	//  ° ٲ ü ش
	if(wState != m_wState)
	{
		m_wState = wState;

		SendAllWeather();
	}
}

void CWeatherManager::SendAllWeather()
{
	g_pUserTable->SetPositionUserHead();
	
	while( CObject* pObject = g_pUserTable->GetUserData() )
	{
		if( pObject->GetObjectKind() != eObjectKind_Player ) continue;
		
		CPlayer* pReceiver = (CPlayer*)pObject;
		
		SendWeather( pReceiver );	
	}
}

void CWeatherManager::SendWeather(CPlayer* pPlayer)
{
	MSG_WORD msg;

	msg.Category = MP_WEATHER;
	msg.dwObjectID = pPlayer->GetID();
	msg.Protocol = MP_WEATHER_STATE;	
	msg.wData = m_wState;
	
	pPlayer->SendMsg( &msg , sizeof( msg ) );
	// ̺Ʈ  ÿ ƹŸ ɼ ־ Ѵ
	WORD* pAvatar = pPlayer->GetShopItemStats()->Avatar;

	/* //SW  ̺Ʈ  ӽ ϵ ڵ
	if(	(pAvatar[eAvatar_Dress] == EVENT_SHOPITEM_SNOWMAN_DRESS && pAvatar[eAvatar_Hat] == EVENT_SHOPITEM_SNOWMAN_HAT) ||
		(pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_RUDOLP_DRESS && pAvatar[eAvatar_Hat] ==  EVENT_SHOPITEM_RUDOLP_HAT) ||
		(pAvatar[eAvatar_Dress] == EVENT_SHOPITEM_SNOWMAN_DRESS2 && pAvatar[eAvatar_Hat] == EVENT_SHOPITEM_SNOWMAN_HAT2) ||
		(pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_RUDOLP_DRESS2 && pAvatar[eAvatar_Hat] ==  EVENT_SHOPITEM_RUDOLP_HAT2) ||
		(pAvatar[eAvatar_Dress] == EVENT_SHOPITEM_SNOWMAN_DRESS && pAvatar[eAvatar_Hat] == EVENT_SHOPITEM_SNOWMAN_HAT2) ||
		(pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_RUDOLP_DRESS && pAvatar[eAvatar_Hat] ==  EVENT_SHOPITEM_RUDOLP_HAT2) ||
		(pAvatar[eAvatar_Dress] == EVENT_SHOPITEM_SNOWMAN_DRESS2 && pAvatar[eAvatar_Hat] == EVENT_SHOPITEM_SNOWMAN_HAT) ||
		(pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_RUDOLP_DRESS2 && pAvatar[eAvatar_Hat] ==  EVENT_SHOPITEM_RUDOLP_HAT) ||
		pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_SNOWMAN1_HK || pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_SNOWMAN2_HK ||
		pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_SNOWMAN3_HK || pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_RUDOLP1_HK ||
		pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_RUDOLP2_HK || pAvatar[eAvatar_Dress] ==  EVENT_SHOPITEM_RUDOLP3_HK ) */

#define SHOPITEM_COS_CAT_HAT		56019
#define SHOPITEM_COS_CAT_DRESS		56020
#define SHOPITEM_COS_WEDDING_MAN	56021
#define SHOPITEM_COS_WEDDING_WOMAN	56022

	if( (pAvatar[eAvatar_Dress] == SHOPITEM_COS_WEDDING_MAN) ||
		(pAvatar[eAvatar_Dress] == SHOPITEM_COS_WEDDING_WOMAN) ||
		(pAvatar[eAvatar_Dress] == SHOPITEM_COS_CAT_HAT) ||
		(pAvatar[eAvatar_Dress] == SHOPITEM_COS_CAT_DRESS) )
		pPlayer->GetShopItemManager()->CalcAvatarOption();
}

#include "stdafx.h"
#include "ServerSystem.h"
#include "AutoNoteManager.h"
#include "UserTable.h"
#include "Player.h"
#include "AutoNoteRoom.h"
#include "Network.h"
#include "MapDBMsgParser.h"


GLOBALTON(CAutoNoteManager)

CAutoNoteManager::CAutoNoteManager(void)
{
	m_bInited = FALSE;
}

CAutoNoteManager::~CAutoNoteManager(void)
{
	Release();
}

void CAutoNoteManager::Init()
{
	m_htAutoNoteRoom.Initialize( 100 );
	m_pmpAutoNoteRoom = new CMemoryPoolTempl< CAutoNoteRoom >;
	m_pmpAutoNoteRoom->Init( 100, 100, "CAutoNoteRoom" );

	m_dwReplyTime = 120;	// ��
	m_nReplyChance = 3;

	//---�̹����ε�
	m_pOriRaster[0] = new BYTE[16*16*3];
	m_pOriRaster[1] = new BYTE[16*16*3];
	m_pOriRaster[2] = new BYTE[16*16*3];
	m_pOriRaster[3] = new BYTE[16*16*3];
	m_pBGRaster		= new BYTE[128*32*3];
	m_pNoiseRaster	= new BYTE[16*16*3];
	m_pSendRaster	= new BYTE[128*32*3];

	if( BMP_RasterLoad( "./AutoNoteImage/01Red.bmp", m_pOriRaster[0] ) == FALSE ) return;
	if( BMP_RasterLoad( "./AutoNoteImage/02Yellow.bmp", m_pOriRaster[1] ) == FALSE ) return;
	if( BMP_RasterLoad( "./AutoNoteImage/03Blue.bmp", m_pOriRaster[2] ) == FALSE ) return;
	if( BMP_RasterLoad( "./AutoNoteImage/04Black.bmp", m_pOriRaster[3] ) == FALSE ) return;
	if( BMP_RasterLoad( "./AutoNoteImage/00BG.bmp", m_pBGRaster ) == FALSE ) return;
	if( BMP_RasterLoad( "./AutoNoteImage/00Noise.bmp", m_pNoiseRaster ) == FALSE ) return;

	m_bInited = TRUE;

	return;
}

void CAutoNoteManager::Release()
{
	m_htAutoNoteRoom.SetPositionHead();
	CAutoNoteRoom* pRoom = NULL;
	while( pRoom = m_htAutoNoteRoom.GetData() )
	{		
		m_pmpAutoNoteRoom->Free( pRoom );
	}
	m_htAutoNoteRoom.RemoveAll();	
	
	SAFE_DELETE( m_pmpAutoNoteRoom );

	/*
	m_pmpAutoNoteRoom->Release();
	delete m_pmpAutoNoteRoom;
	m_htAutoNoteRoom.RemoveAll();
	*/

	//---�̹�������
	delete[] m_pOriRaster[0];
	delete[] m_pOriRaster[1];
	delete[] m_pOriRaster[2];
	delete[] m_pOriRaster[3];
	delete[] m_pBGRaster;
	delete[] m_pNoiseRaster;
	delete[] m_pSendRaster;
}

BOOL CAutoNoteManager::BMP_RasterLoad( char* filename, BYTE* pRaster )
{
	HANDLE hFile = CreateFile( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
		return FALSE;

	DWORD dwFileSize = GetFileSize( hFile, NULL );
	BYTE* data = new BYTE[dwFileSize];

	DWORD dwRead;
	ReadFile( hFile, data, dwFileSize, &dwRead, NULL );
	CloseHandle( hFile );

	BITMAPFILEHEADER* fh = (BITMAPFILEHEADER*)data;
	BITMAPINFOHEADER* ih = (BITMAPINFOHEADER*)( data + sizeof(BITMAPFILEHEADER) );

	int RasterSize = ( ih->biWidth * ih->biHeight * ih->biBitCount ) / 8;
	if( RasterSize < 0 ) RasterSize = -RasterSize;

	memcpy( pRaster, data + ((BITMAPFILEHEADER*)fh)->bfOffBits, RasterSize );

	delete[] data;

	return TRUE;
}

void CAutoNoteManager::MakeSendRaster( DWORD* pData )
{
	int x, y, cx, cy, rot;
	float fszx, fszy;

	//---Clear
	memset( m_pSendRaster, 0xee, 128*32*3 ); 

	//---Background
	fszx = (rand()%5 + 11) / 10.f;	fszy = (rand()%5 + 11) / 10.f;
	BlitImage( m_pSendRaster, m_pBGRaster, 128, 32, 128, 32, 0, 0, fszx, fszy );

	//---noise
	for( int i = 0 ; i < 4 ; ++i )
	{
		x = rand()%(128-16);				y = rand()%(32-16);
		fszx = (rand()%10 + 10) / 10.f;		fszy = (rand()%10 + 10) / 10.f;
		cx = rand()%4 + 6;					cy = rand()%4 + 6;
		rot = rand()%41 - 20;
		BlitImage( m_pSendRaster, m_pNoiseRaster, 128, 32, 16, 16, x, y, fszx, fszy, rot, cx, cy );
	}

	//---txt
	int firstpos = rand()%32;
	int xrand = ( ( 128 - firstpos ) - 90 ) / 4;
	for( int i = 0 ; i < 4 ; ++i )
	{
		x = rand()%xrand + firstpos + i*24;	//i*32;
		y = rand()%8 + 3;
		fszx = (rand()%5 + 11) / 10.f;		fszy = (rand()%5 + 11) / 10.f;
		cx = rand()%4 + 6;					cy = rand()%4 + 6;
		rot = rand()%61 - 30;
		BlitImage( m_pSendRaster, m_pOriRaster[pData[i]], 128, 32, 16, 16, x, y, fszx, fszy, rot, cx, cy, BIF_RANDOMCOLOR );
	}
}

void CAutoNoteManager::BlitImage( BYTE* pDest, BYTE* pSrc, int dw, int dh, int sw, int sh, int x, int y, float fszw, float fszh, float fRot, int cx, int cy, int Flag )
{
	int lsw = sw * fszw;
	int lsh = sh * fszh;

	for( int row = 0 ; row < lsh ; ++row )
	{
		for( int col = 0 ; col < lsw ; ++col )
		{
			int osx = col / fszw + 0.5f;
			int osy = row / fszh + 0.5f;

			if( osx >= sw || osy >= sh )
				continue;

			float fRad = 3.14f * fRot / 180.f;
			int ldx = ( col - cx ) * cos( fRad ) - ( row - cy ) * sin( fRad ) + cx;
			int ldy = ( col - cx ) * sin( fRad ) + ( row - cy ) * cos( fRad ) + cy;

			if( ldx + x >= dw || ldy + y >= dh )
				continue;

			BYTE Color[3]; 
			Color[0] = *(pSrc + osx * 3 + osy * 3 * sw);
			Color[1] = *(pSrc + osx * 3 + osy * 3 * sw + 1);
			Color[2] = *(pSrc + osx * 3 + osy * 3 * sw + 2);

			if( Color[0] != 255 || Color[1] != 255 || Color[2] != 255 )	//����� �ƴϸ�
			{
				if( Flag == BIF_RANDOMCOLOR )
				{
					//���� ���� ����
					Color[0] = ( rand() % 100 );	//255 �������� ������ڳ�?
					Color[1] = ( rand() % 100 );
					Color[2] = ( rand() % 100 );
				}

				memcpy( pDest + ( ldx + x ) * 3 + ( ldy + y ) * 3 * dw, Color, 3 );
			}
		}
	}
}

void CAutoNoteManager::Process()
{
	CAutoNoteRoom* pANRoom = NULL;
	m_htAutoNoteRoom.SetPositionHead();
	while( pANRoom = m_htAutoNoteRoom.GetData() )
	{
		if( pANRoom->IsTimeOver() )
		{
			CPlayer* pAutoPlayer = (CPlayer*)g_pUserTable->FindUser( pANRoom->GetAutoCharacterIdx() );

			if( pANRoom->GetAutoNoteRoomState() == eANRS_WAITANSWER )	//---�亯 �Է��� ��ٸ��� ���̴�. �亯�� �ʾ���.
			{
				if( pAutoPlayer )
				{
//					MSG_DWORD msg1;
//					msg1.Category = MP_AUTONOTE;
//					msg1.Protocol = MP_AUTONOTE_ANSWER_TIMEOUT;
//					msg1.dwData = pANRoom->GetAutoUserIdx();
//					pAutoPlayer->SendMsg( &msg1, sizeof(msg1) );

					MSG_AUTOUSER_BLOCK msg1;
					msg1.Category = MP_AUTONOTE;
					msg1.Protocol = MP_AUTONOTE_ANSWER_TIMEOUT;
					msg1.dwAskUserIdx = pANRoom->GetAskUserIdx();
					msg1.dwAskCharacterIdx = pANRoom->GetAskCharacterIdx();
					SafeStrCpy( msg1.AskCharacterName, pANRoom->GetAskCharacterName(), MAX_NAME_LENGTH+1 );
					msg1.dwAutoUserIdx = pANRoom->GetAutoUserIdx();
					msg1.dwAutoCharacterIdx = pANRoom->GetAutoCharacterIdx();
					SafeStrCpy( msg1.AutoCharacterName, pANRoom->GetAutoCharacterName(), MAX_NAME_LENGTH+1 );
					pAutoPlayer->SendMsg( &msg1, sizeof(msg1) );
				}

				//---�Ű��ڿ��� �Ű������ ����Ǿ����� �˸� (��� ������ �𸣹Ƿ� ��� ������Ʈ�� ����)
				MSG_DWORD msg2;
				msg2.Category = MP_AUTONOTE;
				msg2.Protocol = MP_AUTONOTE_KILLAUTO;
				msg2.dwData = pANRoom->GetAskUserIdx();
				g_Network.Broadcast2AgentServer( (char*)&msg2, sizeof(msg2) );

				//---DB �Ű��� ���丮��Ʈ�� ���
				AutoNoteListAdd( pANRoom->GetAskCharacterIdx(), pANRoom->GetAutoUserIdx(), pANRoom->GetAutoCharacterIdx(), pANRoom->GetAutoCharacterName() );

				//---��� ���� ����
				MSG_DWORD msg3;
				msg3.Category = MP_AUTONOTE;
				msg3.Protocol = MP_AUTONOTE_DISCONNECT;
				msg3.dwData = pANRoom->GetAutoUserIdx();
				g_Network.Broadcast2AgentServer( (char*)&msg3, sizeof(msg3) );
			}
			else if( pANRoom->GetAutoNoteRoomState() == eANRS_FASTANSWER )	// ���� �亯�� ��� ����� üũ
			{
				//---�Ű��ڿ��� �Ű������ ����Ǿ����� �˸� (��� ������ �𸣹Ƿ� ��� ������Ʈ�� ����)
				MSG_DWORD msg2;
				msg2.Category = MP_AUTONOTE;
				msg2.Protocol = MP_AUTONOTE_KILLAUTO;
				msg2.dwData = pANRoom->GetAskUserIdx();
				g_Network.Broadcast2AgentServer( (char*)&msg2, sizeof(msg2) );

				//---DB �Ű��� ���丮��Ʈ�� ���
				AutoNoteListAdd( pANRoom->GetAskCharacterIdx(), pANRoom->GetAutoUserIdx(), pANRoom->GetAutoCharacterIdx(), pANRoom->GetAutoCharacterName() );

				//---��� ���� ����
				MSG_DWORD msg3;
				msg3.Category = MP_AUTONOTE;
				msg3.Protocol = MP_AUTONOTE_DISCONNECT;
				msg3.dwData = pANRoom->GetAutoUserIdx();
				g_Network.Broadcast2AgentServer( (char*)&msg3, sizeof(msg3) );
			}

			if( pAutoPlayer )	pAutoPlayer->SetAutoAskPlayerIdx( 0 );
			m_htAutoNoteRoom.Remove( pANRoom->GetAskCharacterIdx() );
			m_pmpAutoNoteRoom->Free( pANRoom );
			break;	//�ѹ��� �ϳ����� ������ (�����ϰ�)
		}
	}
}

void CAutoNoteManager::AutoPlayerLogOut( CPlayer* pAutoPlayer )
{
	CAutoNoteRoom* pANRoom = m_htAutoNoteRoom.GetData( pAutoPlayer->GetAutoAskPlayerIdx() );
	if( pANRoom == NULL ) return;

	//---�Ű������ �������ȴ�.
//	MSG_DWORD msg;
//	msg.Category = MP_AUTONOTE;
//	msg.Protocol = MP_AUTONOTE_ANSWER_LOGOUT;
//	msg.dwData = pANRoom->GetAutoUserIdx();
//	pAutoPlayer->SendMsg( &msg, sizeof(msg) );

	MSG_AUTOUSER_BLOCK msg1;
	msg1.Category = MP_AUTONOTE;
	msg1.Protocol = MP_AUTONOTE_ANSWER_LOGOUT;
	msg1.dwAskUserIdx = pANRoom->GetAskUserIdx();
	msg1.dwAskCharacterIdx = pANRoom->GetAskCharacterIdx();
	SafeStrCpy( msg1.AskCharacterName, pANRoom->GetAskCharacterName(), MAX_NAME_LENGTH+1 );
	msg1.dwAutoUserIdx = pANRoom->GetAutoUserIdx();
	msg1.dwAutoCharacterIdx = pANRoom->GetAutoCharacterIdx();
	SafeStrCpy( msg1.AutoCharacterName, pANRoom->GetAutoCharacterName(), MAX_NAME_LENGTH+1 );
	pAutoPlayer->SendMsg( &msg1, sizeof(msg1) );

	//---�Ű��ڿ��� �Ű������ ����Ǿ����� �˸� (��� ������ �𸣹Ƿ� ��� ������Ʈ�� ����)
	MSG_DWORD msg2;
	msg2.Category = MP_AUTONOTE;
	msg2.Protocol = MP_AUTONOTE_KILLAUTO;
	msg2.dwData = pANRoom->GetAskUserIdx();
	g_Network.Broadcast2AgentServer( (char*)&msg2, sizeof(msg2) );

	//---DB �Ű��� ���丮��Ʈ�� ���
	AutoNoteListAdd( pANRoom->GetAskCharacterIdx(), pANRoom->GetAutoUserIdx(), pANRoom->GetAutoCharacterIdx(), pANRoom->GetAutoCharacterName() );

	//---���� ����� ����
	pAutoPlayer->SetAutoAskPlayerIdx( 0 );
	m_htAutoNoteRoom.Remove( pANRoom->GetAskCharacterIdx() );
	m_pmpAutoNoteRoom->Free( pANRoom );
}

void CAutoNoteManager::NetworkMsgParse( DWORD dwConnectionIndex, BYTE Protocol, void* pMsg )
{
	switch( Protocol )
	{
	case MP_AUTONOTE_ASKTOAUTO_SYN:
		{
			MSG_DWORD2* pmsg = (MSG_DWORD2*)pMsg;

			CPlayer* pPlayer = (CPlayer*)g_pUserTable->FindUser( pmsg->dwObjectID );
			if( pPlayer == NULL )	return;

			int error = eAutoNoteError_None;

			if( CanUseAutoNote() == FALSE )						//---�̹��� �д� �ʱ�ȭ�� �����Ͽ����Ƿ� �����Ʈ ����� ����� �� ����.
			{
				error = eAutoNoteError_CantUse;					// �̹��� �б� ���� �� ������ ����...
				goto AutoNoteAskError;
			}

			if( pPlayer->GetSingleSpecialState(eSingleSpecialState_Hide) )
			{
				error = eAutoNoteError_CantUse;					// ������ ������...
				goto AutoNoteAskError;
			}

			if( g_pServerSystem->GetMap()->IsAutoNoteAllow() == FALSE )
			{
				error = eAutoNoteError_CantUseMap;				//---����� �� ���� ��
				goto AutoNoteAskError;				
			}

			if( CAutoNoteRoom* pRoom = m_htAutoNoteRoom.GetData( pPlayer->GetID() ) )
			{
				error = eAutoNoteError_AlreadyAsking;			//---�Ű���(�ѹ��� ��ĳ���͸� �Ű��� �� �ִ�)
				goto AutoNoteAskError;
			}

			CPlayer* pAutoPlayer = (CPlayer*)g_pUserTable->FindUser( pmsg->dwData1 );
			if( pAutoPlayer == NULL )
			{
				error = eAutoNoteError_CantFind;				//---��밡 ����
				goto AutoNoteAskError;
			}

			if( pAutoPlayer->GetAutoAskPlayerIdx() )		
			{
				error = eAutoNoteError_AlreadyAsked;			//---���� ���� �Ű��� ĳ���ʹ�
				goto AutoNoteAskError;
			}

			if( gCurTime > pAutoPlayer->GetLastAttackTimeForAutoNote() + 10*1000 )
			{
				error = eAutoNoteError_NotProperState;			//---��밡 ������ ��ų ��� �� 10�ʰ� ������. �Ű��� �� ���� ����.
				goto AutoNoteAskError;
			}

			CAutoNoteRoom* pANRoom = m_pmpAutoNoteRoom->Alloc();
			if( pANRoom == NULL )								//---����� ����
			{
				goto AutoNoteAskError;
			}

			m_htAutoNoteRoom.Add( pANRoom, pPlayer->GetID() );

			pANRoom->CreateRoom( pPlayer, pAutoPlayer, pmsg->dwData2 );
			pANRoom->AskToAuto();
			return;

AutoNoteAskError:
			MSG_DWORD msg;
			msg.Category = MP_AUTONOTE;
			msg.Protocol = MP_AUTONOTE_ASKTOAUTO_NACK;
			msg.dwData = error;
			pPlayer->SendMsg( &msg, sizeof(msg) );
		}
		break;

	case MP_AUTONOTE_ANSWER_SYN:
		{
			MSG_DWORD4* pmsg = (MSG_DWORD4*)pMsg;

			CPlayer* pAutoPlayer = (CPlayer*)g_pUserTable->FindUser( pmsg->dwObjectID );
			if( pAutoPlayer == NULL ) return;

			CAutoNoteRoom* pANRoom = m_htAutoNoteRoom.GetData( pAutoPlayer->GetAutoAskPlayerIdx() );
			if( pANRoom == NULL ) return;	//ACK MSG?

			//---���� üũ
			BOOL bCorrect = pANRoom->CheckAnswerFromAuto( pmsg->dwData1, pmsg->dwData2, pmsg->dwData3, pmsg->dwData4 );

			if( bCorrect )	//---����
			{
				//---�亯�ð� üũ
				DWORD dwAnswerTime = 0;
				if( pANRoom->GetChance() == 3 )
                    dwAnswerTime = pANRoom->GetAnswerTime();
				else
                    dwAnswerTime = pANRoom->GetLastAnswerTime();
				if( dwAnswerTime <= 2000 ) //2�ʳ��� �亯�� �ߴ�. ������ ��쿣 �Ұ���
				{
					//---���信�� ���߾��ٰ� �˸��� �Ƚɽ�Ų��.
					//---�Ű���󿡰� �亯�� ���߾��ٰ� �˸�
					MSG_DWORD msg;
					msg.Category = MP_AUTONOTE;
					msg.Protocol = MP_AUTONOTE_ANSWER_ACK;
					msg.dwData = pANRoom->GetAskUserIdx();
					pAutoPlayer->SendMsg( &msg, sizeof(msg) );

					pANRoom->FastAnswer();	//������ �ð��� �� ���並 ���������. process()�Լ����� ó���Ѵ�.
					return;
				}

				//---�Ű��� ������� ����� ���䰡 �ƴ��� �˸� (�ٸ� ������ ���� ���� ������, ��� ������Ʈ�� �����ش�.)
				MSG_DWORD msg1;
				msg1.Category = MP_AUTONOTE;
				msg1.Protocol = MP_AUTONOTE_NOTAUTO;
				msg1.dwData = pANRoom->GetAskUserIdx();
				g_Network.Broadcast2AgentServer( (char*)&msg1, sizeof(msg1) );

				//---�Ű���󿡰� �亯�� ���߾��ٰ� �˸�
				MSG_DWORD msg2;
				msg2.Category = MP_AUTONOTE;
				msg2.Protocol = MP_AUTONOTE_ANSWER_ACK;
				msg2.dwData = pANRoom->GetAskUserIdx();
				pAutoPlayer->SendMsg( &msg2, sizeof(msg2) );

				//---���� ����� ����
				pAutoPlayer->SetAutoAskPlayerIdx( 0 );
				m_htAutoNoteRoom.Remove( pANRoom->GetAskCharacterIdx() );
				m_pmpAutoNoteRoom->Free( pANRoom );
			}
			else	//---����
			{
				if( pANRoom->GetChance() <= 0 )							//---3�� ��� Ʋ�ȴ�.
				{
					//---��󿡰� �亯 ���������� �˸�
//					MSG_DWORD msg1;
//					msg1.Category = MP_AUTONOTE;
//					msg1.Protocol = MP_AUTONOTE_ANSWER_FAIL;
//					msg1.dwData	= pANRoom->GetAutoUserIdx();
//					pAutoPlayer->SendMsg( &msg1, sizeof(msg1) );

					MSG_AUTOUSER_BLOCK msg1;
					msg1.Category = MP_AUTONOTE;
					msg1.Protocol = MP_AUTONOTE_ANSWER_FAIL;
					msg1.dwAskUserIdx = pANRoom->GetAskUserIdx();
					msg1.dwAskCharacterIdx = pANRoom->GetAskCharacterIdx();
					SafeStrCpy( msg1.AskCharacterName, pANRoom->GetAskCharacterName(), MAX_NAME_LENGTH+1 );
					msg1.dwAutoUserIdx = pANRoom->GetAutoUserIdx();
                    msg1.dwAutoCharacterIdx = pANRoom->GetAutoCharacterIdx();
					SafeStrCpy( msg1.AutoCharacterName, pANRoom->GetAutoCharacterName(), MAX_NAME_LENGTH+1 );
					pAutoPlayer->SendMsg( &msg1, sizeof(msg1) );

					//---��� ���� ����
					MSG_DWORD msg2;
					msg2.Category = MP_AUTONOTE;
					msg2.Protocol = MP_AUTONOTE_DISCONNECT;
					msg2.dwData	= pANRoom->GetAutoUserIdx();
					pAutoPlayer->SendMsg( &msg2, sizeof(msg2) );	//---������Ʈ�� ��������

					//---�Ű��ڿ��� �Ű������ ����Ǿ����� �˸� (��� ������ �𸣹Ƿ� ��� ������Ʈ�� ����)
					MSG_DWORD msg3;
					msg3.Category = MP_AUTONOTE;
					msg3.Protocol = MP_AUTONOTE_KILLAUTO;
					msg3.dwData	= pANRoom->GetAskUserIdx();
					g_Network.Broadcast2AgentServer( (char*)&msg3, sizeof(msg3) );

					//---DB �Ű��� ���丮��Ʈ�� ���
					AutoNoteListAdd( pANRoom->GetAskCharacterIdx(), pANRoom->GetAutoUserIdx(), pANRoom->GetAutoCharacterIdx(), pANRoom->GetAutoCharacterName() );

					//---���� ����� ����
					pAutoPlayer->SetAutoAskPlayerIdx( 0 );
					m_htAutoNoteRoom.Remove( pANRoom->GetAskCharacterIdx() );
					m_pmpAutoNoteRoom->Free( pANRoom );
				}
				else
				{
					MSG_DWORD msg;
					msg.Category = MP_AUTONOTE;
					msg.Protocol = MP_AUTONOTE_ANSWER_NACK;
					msg.dwData = pANRoom->GetChance();
					pAutoPlayer->SendMsg( &msg, sizeof(msg) );
				}
			}
		}
		break;
	}
}

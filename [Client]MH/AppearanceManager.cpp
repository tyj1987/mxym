// AppearanceManager.cpp: implementation of the CAppearanceManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AppearanceManager.h"

#include "Player.h"
#include "Monster.h"
#include "BossMonster.h"
#include "Npc.h"
#include "ItemGround.h"
#include "TacticObject.h"
#include "Titan.h"

#include ".\Engine\EngineObject.h"
#include ".\Engine\EngineEffect.h"
#include "ObjectStateManager.h"

#include "ObjectManager.h"
#include "GameResourceManager.h"
#include "ItemManager.h"
#include "PeaceWarModeManager.h"
#include "MotionManager.h"
#include "CharMakeManager.h"
#include "MainGame.h"
#include "MHMap.h"
#include "Pet.h"
#include "MapObject.h"
#include "StreetStallManager.h"

#include "MHCamera.h"
#include "OptionManager.h"

// ������ �˻��� ���� �߰� by Stiner(8)
#include "GameIn.h"
#include "StallFindDlg.h"
//--

#ifndef _RESOURCE_WORK_NOT_APPLY_
#include "battle.h"
#include "BattleSystem_Client.h"
#include "..\[CC]BattleSystem\SiegeWar\Battle_SiegeWar.h"
#endif


enum
{
	eConfirmAddPlayer_NoErr=0,
	eConfirmAddPlayer_NoData,
	eConfirmAddPlayer_DontAddPlayer,
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
GLOBALTON(CAppearanceManager)

CAppearanceManager::CAppearanceManager()
{

}

CAppearanceManager::~CAppearanceManager()
{
	Release();
}

void CAppearanceManager::Init()
{
	GAMERESRCMNGR->LoadModFileList(GAMERESRCMNGR->m_ModFileList);
	GAMERESRCMNGR->LoadHairModFileList(GAMERESRCMNGR->m_HairModList);
	GAMERESRCMNGR->LoadFaceModFileList(GAMERESRCMNGR->m_FaceModList);
	GAMERESRCMNGR->LoadAvatarEquipList();
	GAMERESRCMNGR->LoadTitanModFileList(&GAMERESRCMNGR->m_TitanModFileList);

	m_AvatarItemExceptionTable.Initialize(50);
	LoadAvatarItemExcetionList();

#ifndef _RESOURCE_WORK_NOT_APPLY_
	m_bUseReservation			= FALSE;	
	
	m_pReserItemPlayer = new RESERVATION_ITEM_PLAYER[MAX_RESERVATION_ITEM_PLAYER];

	int i = 0;
	for (i = 0; i < MAX_RESERVATION_ITEM_PLAYER; i++)
	{
		m_lstFreeReserItemPlayer.AddTail(&m_pReserItemPlayer[i]);
	}

	m_dwUsedReserItemPlayerNum	= 0;

#endif	
}

void CAppearanceManager::Release()
{
	AVATARITEM_EXCEPTION* pAvatarItemException = NULL;
	m_AvatarItemExceptionTable.SetPositionHead();
	while(pAvatarItemException = m_AvatarItemExceptionTable.GetData())
	{
		SAFE_DELETE(pAvatarItemException);
	}
	m_AvatarItemExceptionTable.RemoveAll();

#ifndef _RESOURCE_WORK_NOT_APPLY_	
	m_lstFreeReserItemPlayer.RemoveAll();


	if (NULL != m_pReserItemPlayer)
	{
		delete[] m_pReserItemPlayer;
		m_pReserItemPlayer = NULL;
	}
#endif
}

void CAppearanceManager::InitAppearanceToEffect(CObject* pObject,CEngineEffect* pEffect)
{
	if(pObject->GetObjectKind() == eObjectKind_Player)
	{
		CPlayer* pPlayer = (CPlayer*)pObject;
		SetCharacterAppearanceToEffect(pPlayer,pEffect);
	}
	else if(pObject->GetObjectKind() == eObjectKind_Monster)
	{
		ASSERT(0);	
	}
	else if(pObject->GetObjectKind() == eObjectKind_Npc)
	{
		ASSERT(0);	
	}
}

BOOL CAppearanceManager::LoadAvatarItemExcetionList()
{
	CMHFile file;
	char filename[64];
	DWORD idx = 0;

	//////////////////////////////////////////////////////////////////////////		
#ifdef _FILE_BIN_	
	sprintf(filename,"Resource/Client/AvatarItemException.bin");
	if(!file.Init(filename,"rb"))
		return FALSE;
#else
	sprintf(filename,"Resource/Client/AvatarItemException.txt");
	if(!file.Init(filename,"rt"))
		return FALSE;
#endif

	while(1)
	{
		if(file.IsEOF())
		{
			break;
		}

		AVATARITEM_EXCEPTION* pAvatarItemExcetion = new AVATARITEM_EXCEPTION;
		pAvatarItemExcetion->dwItemIndex = file.GetDword();
		pAvatarItemExcetion->Kind = file.GetDword();
		
		if( NULL != m_AvatarItemExceptionTable.GetData(pAvatarItemExcetion->dwItemIndex) )
		{
			SAFE_DELETE(pAvatarItemExcetion);
			continue;
		}

		m_AvatarItemExceptionTable.Add(pAvatarItemExcetion, pAvatarItemExcetion->dwItemIndex);
	}		
	file.Release();

	return TRUE;
}

CAppearanceManager::AVATARITEM_EXCEPTION* CAppearanceManager::GetAvatarItemExcetion(WORD wItemIndex)
{
	return m_AvatarItemExceptionTable.GetData(wItemIndex);
}

BYTE CAppearanceManager::GetGenderFromMap(CPlayer* pPlayer)
{
	BYTE gender;

	///////// 2007. 6. 18. CBH - Ÿ��ź �� ������ȯ �߰� ///////////////////
//	if((MAP->IsTitanMap() == TRUE) && (MAP->GetMapNum() != 101))	//��õ���� ������ȯ���� �ʴ´�
	if((MAP->IsMapKind(eTitanMap)) && (MAP->GetMapNum() != 101))	//��õ���� ������ȯ���� �ʴ´�
	{
		if(pPlayer->m_CharacterInfo.Gender == GENDER_MALE)	//���ڸ�...		
			gender = GENDER_FEMALE;		
		else	// ���ڸ�...		
			gender = GENDER_MALE;		
	}
	else
	{
		gender = pPlayer->m_CharacterInfo.Gender;
	}
	/////////////////////////////////////////////////////////////////////
#define CHACRACTER_TALL	170
	pPlayer->SetObjectBalloonTall( (LONG)(CHACRACTER_TALL * pPlayer->GetCharacterTotalInfo()->Height) );	

	ASSERT(gender < 2);
	if( gender >= 2 ) 
		gender = 0;

	return gender;
}

BOOL CAppearanceManager::SetCharacterAppearanceToEngineObject(CPlayer* pPlayer,CEngineObject* pEngineObject)
{
	//int PartType,PartModelNum;	
	//BYTE gender;	

	WORD AvatarItemIdx[eAvatar_Max] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};	
	if( pPlayer->m_CharacterInfo.bNoAvatarView == FALSE)
	{
		memcpy( AvatarItemIdx, pPlayer->GetShopItemStats()->Avatar, sizeof(WORD)*eAvatar_Max );	
	}
	else
	{
		//2008. 6. 17. CBH - ������ ������ �������� �ƹ�Ÿ ���� �ɼ� ���� �ȹް� �Ѵ�.
		for(int i = eAvatar_Weared_Gum ; i <= eAvatar_Weared_Amgi ; i++)
		{
			AvatarItemIdx[i] = pPlayer->GetShopItemStats()->Avatar[i];
		}		
	}

	APPEARANCEDATA_INFO DataInfo;	
	ZeroMemory(&DataInfo, sizeof(DataInfo));
	DataInfo.gender = GetGenderFromMap(pPlayer);		
	DataInfo.hair = pPlayer->m_CharacterInfo.HairType;
	DataInfo.face = pPlayer->m_CharacterInfo.FaceType;
	DataInfo.WearedItemIdx = pPlayer->m_CharacterInfo.WearedItemIdx;
	DataInfo.pHairList = &GAMERESRCMNGR->m_HairModList[DataInfo.gender];
	DataInfo.pFaceList = &GAMERESRCMNGR->m_FaceModList[DataInfo.gender];
	DataInfo.pModList = &GAMERESRCMNGR->m_ModFileList[DataInfo.gender];
	DataInfo.AvatarItemIdx = AvatarItemIdx;

	ASSERT(DataInfo.pHairList->MaxModFile > DataInfo.hair);
	if( DataInfo.pHairList->MaxModFile <= DataInfo.hair )
		DataInfo.hair = 0;

	
	/*
	if( pPlayer->GetID() == HEROID )
	{
	if( !OPTIONMGR->GetGameOption()->bNoAvatarView )
	memcpy( AvatarItemIdx, pPlayer->GetShopItemStats()->Avatar, sizeof(WORD)*eAvatar_Max );
	}
	else
	{
	if( !pPlayer->m_CharacterInfo.bNoAvatarView )
	memcpy( AvatarItemIdx, pPlayer->GetShopItemStats()->Avatar, sizeof(WORD)*eAvatar_Max );
	}
	*/	

	//pEngineObject->Init(pModList->BaseObjectFile,pPlayer,eEngineObjectType_Character, eAttachObjectType_TargetObject);
	pEngineObject->Init(DataInfo.pModList->BaseObjectFile,pPlayer,eEngineObjectType_Character);	
	pEngineObject->ChangePart(eAppearPart_Face,"NULL.MOD");	

	// ȣ�� �ӽ� // ��Ÿ����	
	/*
	if(	WearedItemIdx[eWearedItem_Hat] == 53001 || WearedItemIdx[eWearedItem_Hat] == 53002 ||
		WearedItemIdx[eWearedItem_Hat] == 53003 || WearedItemIdx[eWearedItem_Hat] == 53006 ||
		WearedItemIdx[eWearedItem_Hat] == 53009 || WearedItemIdx[eWearedItem_Hat] == 53012 || 
		WearedItemIdx[eWearedItem_Hat] == 53015 || 
		WearedItemIdx[eWearedItem_Hat] == 53225 || WearedItemIdx[eWearedItem_Hat] == 53226 ||
		WearedItemIdx[eWearedItem_Hat] == 53235 || WearedItemIdx[eWearedItem_Hat] == 53236 ||
		WearedItemIdx[eWearedItem_Hat] == 53244 || WearedItemIdx[eWearedItem_Hat] == 53247 ||
		WearedItemIdx[eWearedItem_Hat] == 53250 || WearedItemIdx[eWearedItem_Hat] == 53253 ||
		WearedItemIdx[eWearedItem_Hat] == 53256 ||
		WearedItemIdx[eWearedItem_Hat] == 57695 || WearedItemIdx[eWearedItem_Hat] == 57696 ||
		WearedItemIdx[eWearedItem_Hat] == 57697 )		
	{	
		if( DataInfo.AvatarItemIdx[eAvatar_Hat] == 0 || DataInfo.AvatarItemIdx[eAvatar_Hair] == 0)
		{
			if(DataInfo.gender == 0)		
				pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
			else
				pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");
		}
	}
	else if( AvatarItemIdx[eAvatar_Weared_Hair] )
		pEngineObject->ChangePart(eAppearPart_Hair,DataInfo.pHairList->ModFile[DataInfo.hair]);
		*/

	//BOOL rt;

	SetCharacterAvatarAppearance(pPlayer, pEngineObject, &DataInfo);	//�ƹ�Ÿ ������ ��� ��� ����
	SetCharacterSkinAppearance(pPlayer, pEngineObject, &DataInfo);		//��Ų ������ ��� ��� ����
	SetCharacterWearedAppearance(pPlayer, pEngineObject, &DataInfo);	//�Ϲ� ������ ��� ��� ����
	SetCharacterEventAppearance(pPlayer, pEngineObject, &DataInfo);		//�̺�Ʈ ���� ��� ����

	/*
	// RaMa - �ƹ�Ÿ ������
	for(int i=0; i<=eAvatar_Effect; i++)
	{
		if( AvatarItemIdx[i] )
		{
			ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(AvatarItemIdx[i]);
			if(!pInfo)
				continue;
			PartType = pInfo->Part3DType;
			PartModelNum = pInfo->Part3DModelNum;

			AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( AvatarItemIdx[i] );
			if( i>=eAvatar_Hat && i<=eAvatar_Mustache )
			{
				if( i==eAvatar_Hat || i==eAvatar_Hair )
				{
					if( pAvatarItem && pAvatarItem->Item[eAvatar_Weared_Hair]==0 )
					{
						if(gender == 0)
							pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
						else
							pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");
					}
					else if( pAvatarItem && pAvatarItem->Item[eAvatar_Weared_Hair] )
						pEngineObject->ChangePart( eAppearPart_Hair, pHairList->ModFile[hair] );
				}
				
				if( i==eAvatar_Hat && !pAvatarItem->Item[eAvatar_Hat] )
					continue;

				if( pPlayer->GetFullMoonEventIndex() != eFULLMOONEVENT_NONE )
				{
					if(gender == 0)
						pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
					else
						pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");
				}
				else
				{
					CEngineObject* pPart = new CEngineObject;
					pPart->Init(pModList->ModFile[PartModelNum], NULL, eEngineObjectType_Weapon);
					pEngineObject->AttachDress(pPart,"Bip01 Head");
				}
			}
			else if( i==eAvatar_Shoulder )
			{
			}
			else if( i==eAvatar_Back )
			{
			}
			else
				pEngineObject->ChangePart( PartType, pModList->ModFile[PartModelNum] );

			if(  i == eAvatar_Dress )
			{
				if( pAvatarItem->Item[eAvatar_Hand] == 0 )
					pEngineObject->ChangePart(eAppearPart_Hand, "NULL.MOD");
				if( pAvatarItem->Item[eAvatar_Shoes] == 0 )
					pEngineObject->ChangePart(eAppearPart_Foot, "NULL.MOD");
			}
		}
	}

	if( AvatarItemIdx[eAvatar_Hat] == 55576 || AvatarItemIdx[eAvatar_Dress] == 55577 )
	{
		CEngineObject* pPart = new CEngineObject;
			pPart->Init( "gm_mona.MOD", NULL, eEngineObjectType_Effect );
		pEngineObject->AttachDress( pPart, "Bip01 Head" );
	}
	*/
	/*
	// ���
	for(int n=0;n<eWearedItem_Max;++n)
	{
		if(WearedItemIdx[n])
		{
			if( n<eWearedItem_Shoes && !AvatarItemIdx[n+eAvatar_Weared_Hat] && n != eWearedItem_Weapon )
				continue;
			else if( n==eWearedItem_Cape && !AvatarItemIdx[eAvatar_Weared_Hat] )
				continue;

			ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(WearedItemIdx[n]);
			if(!pInfo)
				continue;
			PartType = pInfo->Part3DType;
			PartModelNum = pInfo->Part3DModelNum;

			if(PartType == -1)
				continue;

			// ���ڴ� �Ӹ��� �ٲ��� �ʰ� �Ӹ��� ���δ�.
			if(gender == 1 && PartType == 0)
			{
				PartType = 6;
			}

			if(PartType == 7)// 7���� ���ڵ� ���ڵ� �Ӹ��� �ٲ۴�. (ȣ�ڸӸ�)
			{
				if(gender == 0)
					pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
				else
					pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");
					
				PartType = 6;
			}

			if(PartType == 6)	// �Ӹ���
			{
				CEngineObject* pHairBand = NULL;
				// �ƹ�Ÿ���� �Ծ����� �⺻�Ӹ� ����� �������� �Ǻ�
				if( AvatarItemIdx[eAvatar_Dress] > 0 )					
				{
					AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( AvatarItemIdx[eAvatar_Dress] );
					if( pAvatarItem )
					{
						if( !pAvatarItem->Item[eAvatar_Hat] )
							continue;
						if( !pAvatarItem->Item[eAvatar_Weared_Hat] )
							continue;
					}
				}

				if( pPlayer->GetFullMoonEventIndex() != eFULLMOONEVENT_NONE )
					continue;

				pHairBand = new CEngineObject;
				pHairBand->Init(pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);
				rt = pEngineObject->AttachDress(pHairBand,"Bip01 Head");				
				
				BYTE stage = pPlayer->GetStage();
				if( stage == eStage_Normal )
					pHairBand->SetMaterialIndex( 0 );
				else if( stage == eStage_Hwa )
					pHairBand->SetMaterialIndex( 1 );
				else
					pHairBand->SetMaterialIndex( 2 );

			}
			else if(PartType == 3)	//2007. 10. 1. CBH - �� ���� ó��
			{
				if(AvatarItemIdx[eAvatar_Weared_Gwun] > 1 && pInfo->WeaponType > 0)	//2007. 10. 12. CBH - �޾����� �� ���� ó��
				{
					ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(AvatarItemIdx[eAvatar_Weared_Gwun]);
					if(!pInfo)
						continue;				

					PartType = pInfo->Part3DType;
					PartModelNum = pInfo->Part3DModelNum;
				}

				pEngineObject->ChangePart(
					//��(���� 2�� Part)�� CHX���� ������ �ٶ��� ��ĭ�� ��������.
					//PartType>2 ? PartType : PartType - 1
					PartType
					,pModList->ModFile[PartModelNum]);
			}
			else if(PartType != 5) // �� �ȹٲٳ� ??	// ����� �ȹٲ۴� ����(yh)
			{
				// RaMa - 06.05.16  -> �ƹ�Ÿ�����ۿ� ���� ����� �չ���  Hide
				if( AvatarItemIdx[eAvatar_Dress] > 0 )					
				{
					AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( AvatarItemIdx[eAvatar_Dress] );
					if( pAvatarItem )
					{
						if( pAvatarItem->Item[eAvatar_Hand] == 0 )
							pEngineObject->ChangePart(eAppearPart_Hand, "NULL.MOD");
						if( pAvatarItem->Item[eAvatar_Shoes] == 0 )
							pEngineObject->ChangePart(eAppearPart_Foot, "NULL.MOD");

						if( PartType == 3 )
						{
							WORD weapon = pPlayer->GetWeaponEquipType();
							for(int i=eAvatar_Weared_Gum; i<=eAvatar_Weared_Amgi; ++i)
							{
								if( pAvatarItem->Item[i] == 0 && weapon == i-(eAvatar_Weared_Gum-1) )
								{
									HideWeapon( pPlayer, pEngineObject );
									goto _NotChangePart;
								}
							}
						}
						else if( PartType == 4 )
						{
							if( !AvatarItemIdx[eAvatar_Shoes] )
								goto _NotChangePart;
						}
					}
				}				

				pEngineObject->ChangePart(
					//��(���� 2�� Part)�� CHX���� ������ �ٶ��� ��ĭ�� ��������.
					//PartType>2 ? PartType : PartType - 1
					PartType
					,pModList->ModFile[PartModelNum]);

_NotChangePart:
				BYTE stage = pPlayer->GetStage();
				if( stage == eStage_Normal )
					pEngineObject->SetMaterialIndex( 0 );
				else if( stage == eStage_Hwa )
					pEngineObject->SetMaterialIndex( 1 );
				else
					pEngineObject->SetMaterialIndex( 2 );
				
			}
		}
	}
	*/

	/*
	// 06.09.25 RaMa - �߼��̺�Ʈ
	if( pPlayer->GetFullMoonEventIndex() != eFULLMOONEVENT_NONE )
	{
		if(gender == 0)
			pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
		else
			pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");

		CEngineObject* pPart = new CEngineObject;
		if( pPlayer->GetFullMoonEventIndex() == eFULLMOONEVENT_BUN )
		{
			if( pPlayer->GetGender() == GENDER_MALE )
				pPart->Init("M_mandoo.MOD", NULL, eEngineObjectType_Weapon);
			else
				pPart->Init("W_mandoo.MOD", NULL, eEngineObjectType_Weapon);
		}
		else if( pPlayer->GetFullMoonEventIndex() == eFULLMOONEVENT_CHINESECAKE )
		{
			if( pPlayer->GetGender() == GENDER_MALE )
				pPart->Init("M_wb.MOD", NULL, eEngineObjectType_Weapon);
			else
				pPart->Init("W_wb.MOD", NULL, eEngineObjectType_Weapon);
		}
		pEngineObject->AttachDress(pPart,"Bip01 Head");
	}	
	else if( pPlayer->GetFullMoonEventIndex() == eFULLMOONEVENT_NONE )
	{
		if( AvatarItemIdx[eAvatar_Weared_Face] )
		{
			CEngineObject* pFace = new CEngineObject;

			ASSERT( pFaceList->MaxModFile > face );
			if( pFaceList->MaxModFile <= face )		face = 0;
			pFace->Init(pFaceList->ModFile[face],NULL,eEngineObjectType_Weapon);
			rt = pEngineObject->AttachDress(pFace,"Bip01 Head");
		}
	}
	*/

	if(pPlayer->GetState() != eObjectState_Ungijosik && pPlayer->GetState() != eObjectState_StreetStall_Owner)
		ShowWeapon(pPlayer,pEngineObject);

	// magi82 - SetItem Test
//	CEngineObject* pSetItem = new CEngineObject;
//	pSetItem->Init("set_chun.chx",NULL,eEngineObjectType_Effect,eAttachObjectType_withAni);
//	rt = pEngineObject->AttachDress(pSetItem,"Bip01 Head");


	// RaMa -test
	/*
	GXOBJECT_HANDLE hGxObject = pEngineObject->GetGXOHandle();
	GXOBJECT_HANDLE hGxAttachObject = NULL;

	CEngineObject* pEff = NULL;
	pEff = new CEngineObject;
	pEff->Init( "do_eff.chx", NULL, eEngineObjectType_Effect, eAttachObjectType_withAni );
	hGxAttachObject = pEff->GetGXOHandle();
	rt = g_pExecutive->GXOAttach( hGxAttachObject, hGxObject, WEAPON_ATTACH_BONENAME_R );
	//*/

#define CHACRACTER_TALL	170
	pPlayer->SetObjectBalloonTall( (LONG)(CHACRACTER_TALL * pPlayer->GetCharacterTotalInfo()->Height) );	

	//������
	VECTOR3 scale;	
	
	if( pPlayer->m_CharacterInfo.Width > 1.1 ) pPlayer->m_CharacterInfo.Width = 1.1f;
	else if( pPlayer->m_CharacterInfo.Width < 0.9 ) pPlayer->m_CharacterInfo.Width = 0.9f;

	if( pPlayer->m_CharacterInfo.Height > 1.1 ) pPlayer->m_CharacterInfo.Height = 1.1f;
	else if( pPlayer->m_CharacterInfo.Height < 0.9 ) pPlayer->m_CharacterInfo.Height = 0.9f;
		
	SetVector3(&scale,
		pPlayer->m_CharacterInfo.Width,
		pPlayer->m_CharacterInfo.Height,
		pPlayer->m_CharacterInfo.Width);

	pEngineObject->SetScale(&scale);
		
	pEngineObject->ApplyHeightField(TRUE);
	pPlayer->SetPosition(&pPlayer->m_MoveInfo.CurPosition);
	pPlayer->SetAngle(pPlayer->m_RotateInfo.Angle.ToRad());
	
	if(pPlayer->GetID() == HEROID)
	{
		pPlayer->GetEngineObject()->DisablePick();
	}
/*	
	// �ܻ� �׽�Ʈ �ڵ�
	CEngineObject* pKGEffect = new CEngineObject;
	pKGEffect->Init("jansang.MOD",NULL,eEngineObjectType_Effect);
	pKGEffect->Attach(pEngineObject,"Bip01 Head");	
	pKGEffect->InitializeIllusionEffect(20,NULL);
	pKGEffect->BeginIllusionEffect();
*/

	//2007. 8. 7. CBH - ��Ʈ������ ����Ʈ ó�� �߰�
	ITEMMGR->SetItemEffectProcess(pPlayer);

	return TRUE;
}

//2007. 11. 29. CBH - �̺�Ʈ ���� ĳ���� ��� ���� �Լ�
void CAppearanceManager::SetCharacterEventAppearance(CPlayer* pPlayer, CEngineObject* pEngineObject, APPEARANCEDATA_INFO* pDataInfo)
{
	BOOL rt;

	// 06.09.25 RaMa - �߼��̺�Ʈ
	if( pPlayer->GetFullMoonEventIndex() != eFULLMOONEVENT_NONE )
	{
		if(pDataInfo->gender == 0)
			pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
		else
			pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");

		CEngineObject* pPart = new CEngineObject;
		if( pPlayer->GetFullMoonEventIndex() == eFULLMOONEVENT_BUN )
		{
			if( pPlayer->GetGender() == GENDER_MALE )
				pPart->Init("M_mandoo.MOD", NULL, eEngineObjectType_Weapon);
			else
				pPart->Init("W_mandoo.MOD", NULL, eEngineObjectType_Weapon);
		}
		else if( pPlayer->GetFullMoonEventIndex() == eFULLMOONEVENT_CHINESECAKE )
		{
			if( pPlayer->GetGender() == GENDER_MALE )
				pPart->Init("M_wb.MOD", NULL, eEngineObjectType_Weapon);
			else
				pPart->Init("W_wb.MOD", NULL, eEngineObjectType_Weapon);
		}
		pEngineObject->AttachDress(pPart,"Bip01 Head");
	}	
	else if( pPlayer->GetFullMoonEventIndex() == eFULLMOONEVENT_NONE )
	{
		WORD wItemIndex = 0;
		ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(pDataInfo->WearedItemIdx[eWearedItem_Hat]);
		if(pInfo != NULL)
			wItemIndex = pInfo->ItemIdx;

		//2007. 12. 4. CBH - ���� �׷��ָ� �ȵǴ� �������� �ƹ�Ÿ ����ó�� ����Ʈ�� �߰��ؼ� �ɷ����Ѵ�.
		if( (pDataInfo->AvatarItemIdx[eAvatar_Weared_Face]) )	//������ �ƹ�Ÿ �������� ���� �׷����� �ʴ°Ÿ� 0
		{
			/*
			SKIN_SELECT_ITEM_INFO* pSkinItemInfo = GAMERESRCMNGR->GetNomalClothesSkinList(pPlayer->GetShopItemStats()->dwSkinItemIndex);		

			//�����Ӹ�, ����Ų��� �� ���� �׸��� �ȵǴ� �κ� ó��
			AVATARITEM_EXCEPTION* pAvetarItemException = GetAvatarItemExcetion(wItemIndex);
			if( (pAvetarItemException != NULL) && (pSkinItemInfo == NULL) )			
			{				
				//�ƹ�Ÿ �������� ���ڵ� ���� ����ũ�� ������ �ȱ׸���.
				BOOL bFlag = FALSE;

				//pAvetarItemException->Kind �� 1�̸� �Ӹ��� �� �׷��ָ� �ȵǴ� ������
				if(pAvetarItemException->Kind != 0)
					bFlag = TRUE;

				if(pDataInfo->AvatarItemIdx[eAvatar_Dress] != 0)
				{
					AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
					if(pAvatarItem != NULL)
					{
						if( pAvatarItem->Item[eAvatar_Hat] == 0 )
							bFlag = TRUE;		
					}			
				}

				if( (pDataInfo->AvatarItemIdx[eAvatar_Mask] > 0) || (pDataInfo->AvatarItemIdx[eAvatar_Weared_Hair] == 0) 
					|| (pDataInfo->AvatarItemIdx[eAvatar_Hat] > 0) || (pDataInfo->AvatarItemIdx[eAvatar_Glasses] > 0))
					bFlag = TRUE;				

				if(bFlag == FALSE)
					return;
				
				//if( (pDataInfo->AvatarItemIdx[eAvatar_Weared_Hat] != 0) && (pDataInfo->AvatarItemIdx[eAvatar_Mask] == 0) )
				//	return;
					
			}
			*/
			/*
			AVATARITEM* pAvatarItem = NULL;
			BOOL bFlag = FALSE;	//���� �����Ұ��ΰ��� üũ�ϴ� ����
			
			WORD* wSkinItem = pPlayer->GetShopItemStats()->wSkinItem;
			//�����Ӹ�, ����Ų��� �� ���� �׸��� �ȵǴ� �κ� ó��
			AVATARITEM_EXCEPTION* pAvetarItemException = GetAvatarItemExcetion(wItemIndex);
			if( (pAvetarItemException != NULL) )
			{
				//pAvetarItemException->Kind �� 1�̸� �Ӹ��� �� �׷��ָ� �ȵǴ� ������
				if(pAvetarItemException->Kind != 0)
					bFlag = TRUE;

				//�ʰ� �Ӹ��� ��ü�� �� ���� üũ
				if(pDataInfo->AvatarItemIdx[eAvatar_Dress] != 0)
				{
					pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
					if(pAvatarItem != NULL)
					{
						if( pAvatarItem->Item[eAvatar_Hat] == 0 )
							bFlag = TRUE;		
					}			
				}

				//�ƹ�Ÿ ������ üũ
				if( (pDataInfo->AvatarItemIdx[eAvatar_Mask] > 0) || (pDataInfo->AvatarItemIdx[eAvatar_Weared_Hair] == 0) 
					|| (pDataInfo->AvatarItemIdx[eAvatar_Hat] > 0) || (pDataInfo->AvatarItemIdx[eAvatar_Glasses] > 0))
					bFlag = TRUE;

				//��Ų üũ
				if(wSkinItem[eSkinItem_Hat] != 0 || wSkinItem[eSkinItem_Mask] != 0)
				{					
					bFlag = TRUE;
				}

				//�ذ񰡸� ��Ų�� ���� �׷��ָ� �ȵȴ�. �ƹ�Ÿ ������ �������� ���� ������ ������ ����
				if(wSkinItem[eSkinItem_Mask] != 0)
				{
					pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( wSkinItem[eSkinItem_Mask] );
					if( (pAvatarItem != NULL) && (pAvatarItem->Item[eAvatar_Weared_Face] == 0) )
					{
						//���� �ƹ�Ÿ ��� �����Ǿ� ������ ���� �׸���.
						if(pDataInfo->AvatarItemIdx[eAvatar_Mask] > 0)
							bFlag = TRUE;
						else
							bFlag = FALSE;
					}
				}				
			}
			*/
			
			AVATARITEM* pAvatarItem = NULL;
			BOOL bFlag = TRUE;	//���� �����Ұ��ΰ��� üũ�ϴ� ����

			WORD* wSkinItem = pPlayer->GetShopItemStats()->wSkinItem;						

			//��Ų�� �������ε��� ������ �ƹ�Ÿ ���� ������������ ����
			pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( wSkinItem[eSkinItem_Mask] );
			if( (pAvatarItem != NULL) && ((pAvatarItem->Position == 3) || (pAvatarItem->Position == 4)) )
			{				
				//���� �׷����� �ȵǴ� ��Ų ����
				if( (pAvatarItem->Item[eAvatar_Weared_Face] == 0) && (wSkinItem[eSkinItem_Mask] > 0) )
				{
					//���� �׷����� �ƹ�Ÿ �� ����ϸ� ���� �׸���.
					//������ �ƹ�Ÿ �������� �Ȱ������ ����ũ�� ���� ������ �ְ� �󱼿� �ƹ��͵� ���������� ������ �׸���.
					if( (pDataInfo->AvatarItemIdx[eAvatar_Weared_Face] != 0) && ((pDataInfo->AvatarItemIdx[eAvatar_Mask] > 0) || (pDataInfo->AvatarItemIdx[eAvatar_Glasses] > 0)) )
						bFlag = TRUE;
					else
						bFlag = FALSE;
				}
				else	//���� �׷����� ��Ų ����
				{
					//���� �׷����� �ȵǴ� �ƹ�Ÿ ���� ����ϸ� ���� �ȱ׸���.
					if( (pDataInfo->AvatarItemIdx[eAvatar_Weared_Face] == 0) && (pDataInfo->AvatarItemIdx[eAvatar_Mask] > 0) )
						bFlag = FALSE;					
				}			
			}
			else
			{
				AVATARITEM_EXCEPTION* pAvetarItemException = GetAvatarItemExcetion(wItemIndex);
				if( (pAvetarItemException != NULL) )
				{
					//pAvetarItemException->Kind �� 0�̸� �Ӹ��� ���� �ȱ׷��ִ� ������				
					if( pAvetarItemException->Kind == 0)
					{
						//�ƹ�Ÿ ���� ����Ǿ� ������
						if( pDataInfo->AvatarItemIdx[eAvatar_Dress] != 0 )
						{
							//�ʰ� �Ӹ��� ��ü�� �� ���� üũ
							pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
							if(pAvatarItem != NULL)
							{
								//�Ӹ��� ��ü���� �ƴϸ� ���� �ȱ׸���.								
								if( pAvatarItem->Item[eAvatar_Hat] == 0 )
									bFlag = TRUE;
								//�ƹ�Ÿ ����, ��Ų ����, �ƹ�Ÿ ��, ��Ų ���� ���� �ȵǾ������� ���� �ȱ׸���.
								else if( (pDataInfo->AvatarItemIdx[eAvatar_Hat] == 0) && (wSkinItem[eSkinItem_Hat] == 0) && (pDataInfo->AvatarItemIdx[eAvatar_Mask] == 0) 
									      && (pDataInfo->AvatarItemIdx[eAvatar_Glasses] == 0) && (pDataInfo->AvatarItemIdx[eAvatar_Hair] == 0) )
									bFlag = FALSE;
							}							
						}
						//�ƹ�Ÿ ����, ��Ų ����, �ƹ�Ÿ ��, ��Ų ���� ���� �ȵǾ��ִ��� �˻�						
						else if( (pDataInfo->AvatarItemIdx[eAvatar_Hat] == 0) && (wSkinItem[eSkinItem_Hat] == 0) && (pDataInfo->AvatarItemIdx[eAvatar_Mask] == 0)
							      && (pDataInfo->AvatarItemIdx[eAvatar_Glasses] == 0) && (pDataInfo->AvatarItemIdx[eAvatar_Hair] == 0) )
                            bFlag = FALSE;
					}
				}				
			}			
			
			if(bFlag == FALSE)
				return;
			
			CEngineObject* pFace = new CEngineObject;

			ASSERT( pDataInfo->pFaceList->MaxModFile > pDataInfo->face );
			if( pDataInfo->pFaceList->MaxModFile <= pDataInfo->face )		
				pDataInfo->face = 0;
			pFace->Init(pDataInfo->pFaceList->ModFile[pDataInfo->face],NULL,eEngineObjectType_Weapon);
			rt = pEngineObject->AttachDress(pFace,"Bip01 Head");			
		}		
	}
}

//2007. 11. 29. CBH - �Ϲ���� ���� ĳ���� ��� ���� �Լ�
void CAppearanceManager::SetCharacterWearedAppearance(CPlayer* pPlayer, CEngineObject* pEngineObject, APPEARANCEDATA_INFO* pDataInfo)
{
	/*
	int PartType,PartModelNum;	
	BOOL rt;

	SKIN_SELECT_ITEM_INFO* pSkinItemInfo = GAMERESRCMNGR->GetNomalClothesSkinList(pPlayer->GetShopItemStats()->dwSkinItemIndex);

	for(int n=0;n<eWearedItem_Max;++n)
	{
		if(pDataInfo->WearedItemIdx[n])
		{
			if( n<eWearedItem_Shoes && !pDataInfo->AvatarItemIdx[n+eAvatar_Weared_Hat] && n != eWearedItem_Weapon )
				continue;
			else if( n==eWearedItem_Cape && !pDataInfo->AvatarItemIdx[eAvatar_Weared_Hat] )
				continue;

			ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(pDataInfo->WearedItemIdx[n]);
			if(!pInfo)
				continue;
			PartType = pInfo->Part3DType;
			PartModelNum = pInfo->Part3DModelNum;

			if(PartType == -1)
				continue;

			// ���ڴ� �Ӹ��� �ٲ��� �ʰ� �Ӹ��� ���δ�.
			if(pDataInfo->gender == 1 && PartType == 0)
			{
				PartType = 6;
			}

			if(PartType == 7)// 7���� ���ڵ� ���ڵ� �Ӹ��� �ٲ۴�. (ȣ�ڸӸ�)
			{
				if(pDataInfo->gender == 0)
					pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
				else
					pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");

				PartType = 6;
			}

			if(PartType == 6)	// �Ӹ���
			{
				CEngineObject* pHairBand = NULL;
				// �ƹ�Ÿ���� �Ծ����� �⺻�Ӹ� ����� �������� �Ǻ�
				if( pDataInfo->AvatarItemIdx[eAvatar_Dress] > 0 )					
				{
					AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
					if( pAvatarItem )
					{
						if( !pAvatarItem->Item[eAvatar_Hat] )
							continue;
						if( !pAvatarItem->Item[eAvatar_Weared_Hat] )
							continue;
					}
				}

				//2008. 1. 7. CBH - �����Ƹ� ������ ������ ���� �ȱ׷��ش�
				if( pDataInfo->AvatarItemIdx[eAvatar_Mask] || pDataInfo->AvatarItemIdx[eAvatar_Glasses] > 0)
					continue;

				//2007. 12. 6. CBH - �Ӹ��� ��Ų�� ������ �ȹٲ��ش�
				if( pSkinItemInfo != NULL ) 
				{
					if( pSkinItemInfo->wEquipItem[eSkinItem_Hat] != 0 )
						continue;
				}				

				if( pPlayer->GetFullMoonEventIndex() != eFULLMOONEVENT_NONE )
					continue;

				
				pHairBand = new CEngineObject;
				pHairBand->Init(pDataInfo->pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);
				rt = pEngineObject->AttachDress(pHairBand,"Bip01 Head");
				

				BYTE stage = pPlayer->GetStage();
				if( stage == eStage_Normal )
					pHairBand->SetMaterialIndex( 0 );
				else if( stage == eStage_Hwa )
					pHairBand->SetMaterialIndex( 1 );
				else
					pHairBand->SetMaterialIndex( 2 );
				

			}
			else if(PartType == 3)	//2007. 10. 1. CBH - �� ���� ó��
			{
				if(pDataInfo->AvatarItemIdx[eAvatar_Weared_Gwun] > 1 && pInfo->WeaponType > 0)	//2007. 10. 12. CBH - �޾����� �� ���� ó��
				{
					ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(pDataInfo->AvatarItemIdx[eAvatar_Weared_Gwun]);
					if(!pInfo)
						continue;				

					PartType = pInfo->Part3DType;
					PartModelNum = pInfo->Part3DModelNum;
				}

				pEngineObject->ChangePart(
					//��(���� 2�� Part)�� CHX���� ������ �ٶ��� ��ĭ�� ��������.
					//PartType>2 ? PartType : PartType - 1
					PartType
					,pDataInfo->pModList->ModFile[PartModelNum]);
			}
			else if(PartType != 5) // �� �ȹٲٳ� ??	// ����� �ȹٲ۴� ����(yh)
			{
				// RaMa - 06.05.16  -> �ƹ�Ÿ�����ۿ� ���� ����� �չ���  Hide
				if( pDataInfo->AvatarItemIdx[eAvatar_Dress] > 0 )					
				{
					AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
					if( pAvatarItem )
					{
						if( pAvatarItem->Item[eAvatar_Hand] == 0 )
							pEngineObject->ChangePart(eAppearPart_Hand, "NULL.MOD");
						if( pAvatarItem->Item[eAvatar_Shoes] == 0 )
							pEngineObject->ChangePart(eAppearPart_Foot, "NULL.MOD");

						if( PartType == 3 )
						{
							WORD weapon = pPlayer->GetWeaponEquipType();
							for(int i=eAvatar_Weared_Gum; i<=eAvatar_Weared_Amgi; ++i)
							{
								if( pAvatarItem->Item[i] == 0 && weapon == i-(eAvatar_Weared_Gum-1) )
								{
									HideWeapon( pPlayer, pEngineObject );
									goto _NotChangePart;
								}
							}
						}
						else if( PartType == 4 )
						{
							if( !pDataInfo->AvatarItemIdx[eAvatar_Shoes] )
								goto _NotChangePart;

							//2007. 12. 6. CBH - �Ź� ��Ų�� ������ �ȹٲ��ش�
							if( pSkinItemInfo != NULL ) 
							{
								if( pSkinItemInfo->wEquipItem[eSkinItem_Shoes] != 0 )
									goto _NotChangePart;
							}
						}
					}
				}

				//2007. 12. 6. CBH - �� ��Ų�� ������ �ȹٲ��ش�
				if( pSkinItemInfo != NULL ) 
				{
					if( pSkinItemInfo->wEquipItem[eSkinItem_Dress] != 0 )
						goto _NotChangePart;
				}

				pEngineObject->ChangePart(
					//��(���� 2�� Part)�� CHX���� ������ �ٶ��� ��ĭ�� ��������.
					//PartType>2 ? PartType : PartType - 1
					PartType
					,pDataInfo->pModList->ModFile[PartModelNum]);

_NotChangePart:
				BYTE stage = pPlayer->GetStage();
				if( stage == eStage_Normal )
					pEngineObject->SetMaterialIndex( 0 );
				else if( stage == eStage_Hwa )
					pEngineObject->SetMaterialIndex( 1 );
				else
					pEngineObject->SetMaterialIndex( 2 );

			}
		}
	}
	*/
	int PartType,PartModelNum;	
	BOOL rt;

	WORD* wSkinItem = pPlayer->GetShopItemStats()->wSkinItem;

	for(int n=0;n<eWearedItem_Max;++n)
	{
		if(pDataInfo->WearedItemIdx[n])
		{
			if( n<eWearedItem_Shoes && !pDataInfo->AvatarItemIdx[n+eAvatar_Weared_Hat] && n != eWearedItem_Weapon )
				continue;
			else if( n==eWearedItem_Cape && !pDataInfo->AvatarItemIdx[eAvatar_Weared_Hat] )
				continue;

			ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(pDataInfo->WearedItemIdx[n]);
			if(!pInfo)
				continue;
			PartType = pInfo->Part3DType;
			PartModelNum = pInfo->Part3DModelNum;

			if(PartType == -1)
				continue;

			// ���ڴ� �Ӹ��� �ٲ��� �ʰ� �Ӹ��� ���δ�.
			if(pDataInfo->gender == 1 && PartType == 0)
			{
				PartType = 6;
			}

			if(PartType == 7)// 7���� ���ڵ� ���ڵ� �Ӹ��� �ٲ۴�. (ȣ�ڸӸ�)
			{
				if(pDataInfo->gender == 0)
					pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
				else
					pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");

				PartType = 6;
			}

			if(PartType == 6)	// �Ӹ���
			{
				CEngineObject* pHairBand = NULL;
				// �ƹ�Ÿ���� �Ծ����� �⺻�Ӹ� ����� �������� �Ǻ�
				if( pDataInfo->AvatarItemIdx[eAvatar_Dress] > 0 )					
				{
					AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
					if( pAvatarItem )
					{
						if( !pAvatarItem->Item[eAvatar_Hat] )
							continue;
						if( !pAvatarItem->Item[eAvatar_Weared_Hat] )
							continue;
					}
				}

				//2008. 1. 7. CBH - �����Ƹ� ������ ������ ���� �ȱ׷��ش�
				if( (pDataInfo->AvatarItemIdx[eAvatar_Hair] != 0) || (pDataInfo->AvatarItemIdx[eAvatar_Mask] != 0) 
					|| (pDataInfo->AvatarItemIdx[eAvatar_Glasses] != 0) )
					continue;

				//2007. 12. 6. CBH - ����, ����ũ ��Ų�� ������ �ȹٲ��ش�
				if( (wSkinItem[eSkinItem_Hat] != 0) || (wSkinItem[eSkinItem_Mask] != 0) )
					continue;
				
				if( pPlayer->GetFullMoonEventIndex() != eFULLMOONEVENT_NONE )
					continue;


				pHairBand = new CEngineObject;
				pHairBand->Init(pDataInfo->pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);
				rt = pEngineObject->AttachDress(pHairBand,"Bip01 Head");


				BYTE stage = pPlayer->GetStage();
				if( stage == eStage_Normal )
					pHairBand->SetMaterialIndex( 0 );
				else if( stage == eStage_Hwa || stage == eStage_Hyun )
					pHairBand->SetMaterialIndex( 1 );
				else
					pHairBand->SetMaterialIndex( 2 );


			}
			else if(PartType == 3)	//2007. 10. 1. CBH - �� ���� ó��
			{
				if(pDataInfo->AvatarItemIdx[eAvatar_Weared_Gwun] > 1 && pInfo->WeaponType > 0)	//2007. 10. 12. CBH - �޾����� �� ���� ó��
				{
					ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(pDataInfo->AvatarItemIdx[eAvatar_Weared_Gwun]);
					if(!pInfo)
						continue;				

					PartType = pInfo->Part3DType;
					PartModelNum = pInfo->Part3DModelNum;
				}

				pEngineObject->ChangePart(
					//��(���� 2�� Part)�� CHX���� ������ �ٶ��� ��ĭ�� ��������.
					//PartType>2 ? PartType : PartType - 1
					PartType
					,pDataInfo->pModList->ModFile[PartModelNum]);
			}
			else if(PartType != 5) // �� �ȹٲٳ� ??	// ����� �ȹٲ۴� ����(yh)
			{
				// RaMa - 06.05.16  -> �ƹ�Ÿ�����ۿ� ���� ����� �չ���  Hide
				if( pDataInfo->AvatarItemIdx[eAvatar_Dress] > 0 )					
				{
					AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
					if( pAvatarItem )
					{
						if( pAvatarItem->Item[eAvatar_Hand] == 0 )
							pEngineObject->ChangePart(eAppearPart_Hand, "NULL.MOD");
						if( pAvatarItem->Item[eAvatar_Shoes] == 0 )
							pEngineObject->ChangePart(eAppearPart_Foot, "NULL.MOD");

						if( PartType == 3 )
						{
							WORD weapon = pPlayer->GetWeaponEquipType();
							for(int i=eAvatar_Weared_Gum; i<=eAvatar_Weared_Amgi; ++i)
							{
								if( pAvatarItem->Item[i] == 0 && weapon == i-(eAvatar_Weared_Gum-1) )
								{
									HideWeapon( pPlayer, pEngineObject );
									goto _NotChangePart;
								}
							}
						}
						else if( PartType == 4 )
						{
							if( !pDataInfo->AvatarItemIdx[eAvatar_Shoes] )
								goto _NotChangePart;

							//2007. 12. 6. CBH - �Ź� ��Ų�� ������ �ȹٲ��ش�
							if( wSkinItem[eSkinItem_Shoes] != 0 )
								goto _NotChangePart;							
						}
					}
				}

				//2007. 12. 6. CBH - �� ��Ų�� ������ �ȹٲ��ش�
				if( wSkinItem[eSkinItem_Dress] != 0 )
					goto _NotChangePart;				

				pEngineObject->ChangePart(
					//��(���� 2�� Part)�� CHX���� ������ �ٶ��� ��ĭ�� ��������.
					//PartType>2 ? PartType : PartType - 1
					PartType
					,pDataInfo->pModList->ModFile[PartModelNum]);

	_NotChangePart:
				BYTE stage = pPlayer->GetStage();
				if( stage == eStage_Normal )
					pEngineObject->SetMaterialIndex( 0 );
				else if( stage == eStage_Hwa || stage == eStage_Hyun )
					pEngineObject->SetMaterialIndex( 1 );				
				else
					pEngineObject->SetMaterialIndex( 2 );

			}
		}
	}
}

//2007. 11. 29. CBH - ��Ų ��� ���� ĳ���� ��� ���� �Լ�
void CAppearanceManager::SetCharacterSkinAppearance(CPlayer* pPlayer, CEngineObject* pEngineObject, APPEARANCEDATA_INFO* pDataInfo)
{
	/*
	int PartType,PartModelNum;
	BOOL rt;

	int nSkinIndex = pPlayer->GetShopItemStats()->dwSkinItemIndex;
	if(nSkinIndex < 0)	//�ʱⰪ -1
		return;

	//���õ� ��Ų �ε����� ������ ������ �����´�
	SKIN_SELECT_ITEM_INFO* pSkinItem = GAMERESRCMNGR->GetNomalClothesSkinList(nSkinIndex);
	if(pSkinItem == NULL)
		return;

	WORD wItemIndex = pSkinItem->wEquipItem[eSkinItem_Hat];
	//�ƹ�Ÿ �������� ���� ��Ų ������ ������ �������ش� (����)
	if( (pDataInfo->AvatarItemIdx[eAvatar_Hat] == 0) && (wItemIndex != 0) )
	{
		//�����Ӹ��϶� �Ӹ� ���� �����ָ� �ȳ����Ƿ� ���⼭ �ѹ��� �Ӹ� ���� ���ش�
		pEngineObject->ChangePart(eAppearPart_Hair,pDataInfo->pHairList->ModFile[pDataInfo->hair]);

		ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(wItemIndex);
		if(!pInfo)
			return;

		PartType = pInfo->Part3DType;
		PartModelNum = pInfo->Part3DModelNum;

		//pEngineObject->ChangePart(PartType, pDataInfo->pModList->ModFile[PartModelNum]);
		CEngineObject* pHairBand = NULL;
		pHairBand = new CEngineObject;
		pHairBand->Init(pDataInfo->pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);
		rt = pEngineObject->AttachDress(pHairBand,"Bip01 Head");
	}

	wItemIndex = pSkinItem->wEquipItem[eSkinItem_Dress];
	//�ƹ�Ÿ �������� ���� ��Ų ������ ������ �������ش� (��)
	if( (pDataInfo->AvatarItemIdx[eAvatar_Dress] == 0) && (wItemIndex != 0) )
	{
		ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(wItemIndex);
		if(!pInfo)
			return;

		PartType = pInfo->Part3DType;
		PartModelNum = pInfo->Part3DModelNum;

		pEngineObject->ChangePart(PartType, pDataInfo->pModList->ModFile[PartModelNum]);
	}
	
	wItemIndex = pSkinItem->wEquipItem[eSkinItem_Shoes];
	//�ƹ�Ÿ �������� ���� ��Ų ������ ������ �������ش� (�Ź�)
	//�ƹ�Ÿ �������� �Ź��� ���� ���ϰ� ��Ʈ�� ���� �ִ�.
	if( wItemIndex != 0 )
	{
		//�ʿ� �Ź��� ���� �پ��ִ� �������̸� ������ ���Ѵ�.
		if(pDataInfo->AvatarItemIdx[eAvatar_Dress] != 0)
		{
			AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
			if(pAvatarItem == NULL)
				return;

			//�巹���� �ƹ�Ÿ �������߿� �Ź��� ���ѵ� ���� üũ�ؼ� �ȹٲ��ش�.
			if(pAvatarItem->Item[eAvatar_Shoes] == 0)
				return;
		}	
		
		ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(wItemIndex);
		if(!pInfo)
			return;

		PartType = pInfo->Part3DType;
		PartModelNum = pInfo->Part3DModelNum;

		pEngineObject->ChangePart(PartType, pDataInfo->pModList->ModFile[PartModelNum]);
	}
	*/
	int PartType,PartModelNum;
	BOOL rt;

	//��Ų ����� ������ �����´�.
	WORD* wSkinItem = pPlayer->GetShopItemStats()->wSkinItem;
	
	WORD wItemIndex = wSkinItem[eSkinItem_Hat];
	//�ƹ�Ÿ �������� ���� ��Ų ������ ������ �������ش� (����)
	if( (pDataInfo->AvatarItemIdx[eAvatar_Hat] == 0) && (wItemIndex != 0) )
	{		
		
		//�� ��ü�� ������ �˻�
		BOOL bFlag = TRUE;
		if(pDataInfo->AvatarItemIdx[eAvatar_Dress] != 0)
		{
			AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
			if(pAvatarItem != NULL)
			{
				if( pAvatarItem->Item[eAvatar_Hat] == 0 )
					bFlag = FALSE;
			}			
		}		

		if(bFlag == TRUE)
		{
			ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(wItemIndex);
			if(!pInfo)
				return;

			PartType = pInfo->Part3DType;
			PartModelNum = pInfo->Part3DModelNum;

			CEngineObject* pHairBand = NULL;
			pHairBand = new CEngineObject;
			pHairBand->Init(pDataInfo->pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);
			rt = pEngineObject->AttachDress(pHairBand,"Bip01 Head");
		}
	}
	
	wItemIndex = wSkinItem[eSkinItem_Mask];
	//�ƹ�Ÿ �������� ���� ��Ų ������ ������ �������ش� (��)
	if( (pDataInfo->AvatarItemIdx[eAvatar_Mask] == 0) && (pDataInfo->AvatarItemIdx[eAvatar_Glasses] == 0) && (wItemIndex != 0) )
	{
		ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(wItemIndex);
		if(!pInfo)
			return;

		PartType = pInfo->Part3DType;
		PartModelNum = pInfo->Part3DModelNum;		

		CEngineObject* pPart = new CEngineObject;
		pPart->Init(pDataInfo->pModList->ModFile[PartModelNum], NULL, eEngineObjectType_Weapon);
		pEngineObject->AttachDress(pPart,"Bip01 Head");
	}

	wItemIndex = wSkinItem[eSkinItem_Dress];
	//�ƹ�Ÿ �������� ���� ��Ų ������ ������ �������ش� (��)
	if( (pDataInfo->AvatarItemIdx[eAvatar_Dress] == 0) && (wItemIndex != 0) )
	{
		ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(wItemIndex);
		if(!pInfo)
			return;

		PartType = pInfo->Part3DType;
		PartModelNum = pInfo->Part3DModelNum;

		pEngineObject->ChangePart(PartType, pDataInfo->pModList->ModFile[PartModelNum]);
	}

	wItemIndex = wSkinItem[eSkinItem_Shoes];
	//�ƹ�Ÿ �������� ���� ��Ų ������ ������ �������ش� (�Ź�)
	//�ƹ�Ÿ �������� �Ź��� ���� ���ϰ� ��Ʈ�� ���� �ִ�.
	if( wItemIndex != 0 )
	{
		//�ʿ� �Ź��� ���� �پ��ִ� �������̸� ������ ���Ѵ�.
		if(pDataInfo->AvatarItemIdx[eAvatar_Dress] != 0)
		{
			AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
			if(pAvatarItem == NULL)
				return;

			//�巹���� �ƹ�Ÿ �������߿� �Ź��� ���ѵ� ���� üũ�ؼ� �ȹٲ��ش�.
			if(pAvatarItem->Item[eAvatar_Shoes] == 0)
				return;
		}	

		ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(wItemIndex);
		if(!pInfo)
			return;

		PartType = pInfo->Part3DType;
		PartModelNum = pInfo->Part3DModelNum;

		pEngineObject->ChangePart(PartType, pDataInfo->pModList->ModFile[PartModelNum]);
	}
}

//2007. 11. 29. CBH - �ƹ�Ÿ ��� ���� ĳ���� ��� ���� �Լ�
void CAppearanceManager::SetCharacterAvatarAppearance(CPlayer* pPlayer, CEngineObject* pEngineObject, APPEARANCEDATA_INFO* pDataInfo)
{
	int PartType,PartModelNum;

	WORD* wSkinItem = pPlayer->GetShopItemStats()->wSkinItem;
	AVATARITEM_EXCEPTION* pAvetarItemException = GetAvatarItemExcetion(pDataInfo->WearedItemIdx[eWearedItem_Hat]);	
	if( pAvetarItemException != NULL)
	{
		//2008. 1. 7. CBH - �ƹ�Ÿ ������ �ʰ� �Ӹ� ��ü���� ���� �Ծ�����
		//ȣ�ڸӸ� �� �����Ӹ� ���� ���������� �Ӹ��� �ȳ����� ���װ� �ִ�.
		//�ƹ�Ÿ ������ �Ӹ��� �����Ǿ� ������ �Ӹ��� �׷��ش�.
		//���鵵 ���ϵ���		
		BOOL bFlag = TRUE;

		//pAvetarItemException->Kind �� 1�̸� �Ӹ��� �ȱ׷��ְ� ���� �׷��ִ� ������
		if( (pAvetarItemException->Kind == 0) || (pAvetarItemException->Kind == 1) )
			bFlag = FALSE;

		//�Ӹ��� �� ��ü�� �ƹ�Ÿ ������ �˻�
		if(pDataInfo->AvatarItemIdx[eAvatar_Dress] != 0)
		{
			AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[eAvatar_Dress] );
			if(pAvatarItem != NULL)
			{
				if( pAvatarItem->Item[eAvatar_Hat] == 0 )
					bFlag = TRUE;
			}			
		}

		//����ũ, �Ӹ�, ����, �Ȱ� �˻�
		if( (pDataInfo->AvatarItemIdx[eAvatar_Mask] > 0) || (pDataInfo->AvatarItemIdx[eAvatar_Weared_Hair] == 0) 
			|| (pDataInfo->AvatarItemIdx[eAvatar_Hat] > 0) || (pDataInfo->AvatarItemIdx[eAvatar_Glasses] > 0))		
			bFlag = TRUE;

		//��Ų üũ
		if(wSkinItem[eSkinItem_Hat] != 0 || wSkinItem[eSkinItem_Mask] != 0)
			bFlag = TRUE;

		if(TRUE == bFlag)
		{
			pEngineObject->ChangePart(eAppearPart_Hair,pDataInfo->pHairList->ModFile[pDataInfo->hair]);
		}
		else
		{
			if(pDataInfo->gender == 0)		
				pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
			else
				pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");
		}		
	}
	else if( pDataInfo->AvatarItemIdx[eAvatar_Weared_Hair] )
		pEngineObject->ChangePart(eAppearPart_Hair,pDataInfo->pHairList->ModFile[pDataInfo->hair]);

	for(int i=0; i<=eAvatar_Effect; i++)
	{
		if( pDataInfo->AvatarItemIdx[i] )
		{
			ITEM_INFO* pInfo = ITEMMGR->GetItemInfo(pDataInfo->AvatarItemIdx[i]);
			if(!pInfo)
				continue;
			PartType = pInfo->Part3DType;
			PartModelNum = pInfo->Part3DModelNum;

			AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pDataInfo->AvatarItemIdx[i] );
			if( i>=eAvatar_Hat && i<=eAvatar_Mustache )
			{
				if( i==eAvatar_Hat || i==eAvatar_Hair )
				{
					if( pAvatarItem && pAvatarItem->Item[eAvatar_Weared_Hair]==0 )
					{
						if(pDataInfo->gender == 0)
							pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
						else
							pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");
					}
					else if( pAvatarItem && pAvatarItem->Item[eAvatar_Weared_Hair] )
						pEngineObject->ChangePart( eAppearPart_Hair, pDataInfo->pHairList->ModFile[pDataInfo->hair] );
				}

				if( i==eAvatar_Hat && !pAvatarItem->Item[eAvatar_Hat] )
					continue;

				if( pPlayer->GetFullMoonEventIndex() != eFULLMOONEVENT_NONE )
				{
					if(pDataInfo->gender == 0)
						pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_M.MOD");
					else
						pEngineObject->ChangePart(eAppearPart_Hair,"NULLHAIR_W.MOD");
				}
				else
				{
					CEngineObject* pPart = new CEngineObject;
					pPart->Init(pDataInfo->pModList->ModFile[PartModelNum], NULL, eEngineObjectType_Weapon);
					pEngineObject->AttachDress(pPart,"Bip01 Head");
				}
			}
			else if( i==eAvatar_Shoulder )
			{
			}
			else if( i==eAvatar_Back )
			{
			}
			else
				pEngineObject->ChangePart( PartType, pDataInfo->pModList->ModFile[PartModelNum] );

			if(  i == eAvatar_Dress )
			{
				if( pAvatarItem->Item[eAvatar_Hand] == 0 )
					pEngineObject->ChangePart(eAppearPart_Hand, "NULL.MOD");
				if( pAvatarItem->Item[eAvatar_Shoes] == 0 )
					pEngineObject->ChangePart(eAppearPart_Foot, "NULL.MOD");
			}
		}
	}

	if( pDataInfo->AvatarItemIdx[eAvatar_Hat] == 55576 || pDataInfo->AvatarItemIdx[eAvatar_Dress] == 55577 )
	{
		CEngineObject* pPart = new CEngineObject;
		pPart->Init( "gm_mona.MOD", NULL, eEngineObjectType_Effect );
		pEngineObject->AttachDress( pPart, "Bip01 Head" );
	}
}

BOOL CAppearanceManager::SetTitanAppearanceToEngineObject( CPlayer* pPlayer, CEngineObject* pEngineObject )
{
	int PartType, PartModelNum;
	//
	TITAN_APPEARANCEINFO* pAppearInfo = NULL;

	if( pPlayer->IsTitanPreView() )
		pAppearInfo = pPlayer->GetTitanPreViewInfo();
	else
		pAppearInfo = pPlayer->GetTitanAppearInfo();
	BASE_TITAN_LIST* pBaseInfo = GAMERESRCMNGR->GetTitanListInfo( pAppearInfo->TitanKind );
	if( !pBaseInfo )
		return FALSE;

	//Ÿ��ź �̸� ���� ����
	TITANINFO_GRADE* pGradeInfo = &pBaseInfo->GradeInfo[pAppearInfo->TitanGrade - 1];
	if( !pGradeInfo )
		return FALSE;
	pPlayer->SetObjectBalloonTall(LONG(pGradeInfo->Tall*pGradeInfo->Scale));

	MOD_LIST* pModList = &GAMERESRCMNGR->m_TitanModFileList;
	
	pEngineObject->Init( pBaseInfo->TitanBaseChxName, 
		pPlayer, eEngineObjectType_Titan, eAttachObjectType_TargetObject );
	

	/*
	eAppearPart_Hair,
	eAppearPart_Face,
	eAppearPart_Body,
	eAppearPart_Hand,
	eAppearPart_Foot,
	eAppearPart_Weapon,
	*/
	BOOL rt;	
	pEngineObject->ChangePart( eAppearPart_Face, "NULL.MOD" );
	pEngineObject->ChangePart( 5, "NULL.MOD" );

	// �⺻�󱼼���
	if( !pAppearInfo->WearedItemIdx[eTitanWearedItem_HelMet] )
	{
		CEngineObject* pFace = new CEngineObject;	
		pFace->Init( pBaseInfo->TitanfaceModName, NULL, eEngineObjectType_Titan);
		rt= pEngineObject->AttachDress( pFace, "Bip01 Head" );
	}
	
	GXOBJECT_HANDLE hGxObject = pEngineObject->GetGXOHandle();
	GXOBJECT_HANDLE hGxAttachObject = NULL;

	for(int i=0; i<eTitanWearedItem_Max; ++i)
	{
		if( pAppearInfo->WearedItemIdx[i] )
		{
			ITEM_INFO* pInfo = ITEMMGR->GetItemInfo( pAppearInfo->WearedItemIdx[i] );
			if(!pInfo)
				continue;
			PartType = pInfo->Part3DType;
			PartModelNum = pInfo->Part3DModelNum;

			if( PartType == -1 )
				continue;

			if( i == eTitanWearedItem_HelMet )
			{
				pEngineObject->ChangePart( PartType, pModList->ModFile[PartModelNum] );

				// ����� �� ���� _h02_  -> _h01_  �� ����
				char buf[30] = { 0, };
				SafeStrCpy( buf, pModList->ModFile[PartModelNum], strlen(pModList->ModFile[PartModelNum])+1 );
				char* p = strstr( buf, "_h02_");
				int tlen = strlen(buf);
				int clen =strlen(p);
				buf[tlen-clen+3] = '1';

				CEngineObject* pFace = new CEngineObject;	
				pFace->Init( buf, NULL, eEngineObjectType_Titan);
				rt= pEngineObject->AttachDress( pFace, "Bip01 Head" );
			}			
			else if( i == eTitanWearedItem_Cloak )
			{
				pEngineObject->ChangePart( 5, pModList->ModFile[PartModelNum] );
			}
			else if( i == eTitanWearedItem_Shield )
			{
				CEngineObject* pShield = NULL;
				pShield = new CEngineObject;
				pShield->Init( pModList->ModFile[PartModelNum], NULL, eEngineObjectType_Titan );
				rt = pEngineObject->AttachDress( pShield, "Bone_Forearm" );
			}
			else
			{
				// ����� ���⼭ ���Ѵ�.
				if( PartType != 5 )
					pEngineObject->ChangePart( PartType, pModList->ModFile[PartModelNum] );
			}
		}
	}

	/*
	CEngineObject* pEff = NULL;
	pEff = new CEngineObject;
	pEff->Init( "do_eff.chx", NULL, eEngineObjectType_Effect, eAttachObjectType_withAni );
	hGxAttachObject = pEff->GetGXOHandle();
	rt = g_pExecutive->GXOAttach( hGxAttachObject, hGxObject, "Bone_R" );
	*/

	if(pPlayer->GetState() != eObjectState_Ungijosik && pPlayer->GetState() != eObjectState_StreetStall_Owner)
		ShowWeapon(pPlayer, pEngineObject);


	//������
	VECTOR3 scale = { 1.0f, 1.0f, 1.0f };
	if( pAppearInfo->TitanScale > 95 )		pAppearInfo->TitanScale = 95;
	else if( pAppearInfo->TitanScale < 85 )		pAppearInfo->TitanScale = 85;
	scale = scale * (pAppearInfo->TitanScale*0.01f); 
	pEngineObject->SetScale( &scale );

	pEngineObject->ApplyHeightField( TRUE );
	pPlayer->SetPosition( &pPlayer->m_MoveInfo.CurPosition );
	pPlayer->SetAngle( pPlayer->m_RotateInfo.Angle.ToRad() );

	if( pPlayer->GetID() == HEROID )
		pPlayer->GetEngineObject()->DisablePick();

	//2007. 8. 7. CBH - ��Ʈ������ ����Ʈ ó�� �߰�
	ITEMMGR->SetItemEffectProcess(pPlayer);

	return TRUE;
}

void CAppearanceManager::ShowWeapon(CPlayer* pPlayer)
{
	if( pPlayer->InTitan() )
		SetTitanAppearanceToEngineObject( pPlayer, &pPlayer->m_EngineObject );
	else
		SetCharacterAppearanceToEngineObject(pPlayer,&pPlayer->m_EngineObject);

	ShowWeapon(pPlayer,pPlayer->GetEngineObject());
	
	///////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ����/����
	if(pPlayer->GetSingleSpecialState(eSingleSpecialState_Hide))
	{
		OBJECTMGR->SetHide(pPlayer, TRUE);
	}
	///////////////////////////////////////////////////////////////////
	else if(pPlayer->GetCharacterTotalInfo()->bVisible == FALSE)	
	{
#ifdef _GMTOOL_
		if( MAINGAME->GetUserLevel() <= eUSERLEVEL_GM && CAMERA->GetCameraMode() != eCM_EyeView )
			pPlayer->GetEngineObject()->SetAlpha( 0.3f );
		else
#endif
			pPlayer->GetEngineObject()->HideWithScheduling();
	}

	OBJECTMGR->ApplyShadowOption(pPlayer);

	
	WORD* AvatarItemIdx = pPlayer->GetShopItemStats()->Avatar;
	if( AvatarItemIdx[eAvatar_Dress] > 0 )
	{		
		AVATARITEM* pAvatarItem = GAMERESRCMNGR->m_AvatarEquipTable.GetData( AvatarItemIdx[eAvatar_Dress] );
		if( pAvatarItem )
		{
			WORD weapon = pPlayer->GetWeaponEquipType();

			for(int i=eAvatar_Weared_Gum; i<=eAvatar_Weared_Amgi; ++i)
			{
				if( pAvatarItem->Item[i] == 0 && weapon == i-(eAvatar_Weared_Gum-1) && AvatarItemIdx[i] < 2 )
				{
					HideWeapon( pPlayer, &pPlayer->m_EngineObject );
					break;
				}				
			}
		}
	}
}
void CAppearanceManager::HideWeapon(CPlayer* pPlayer)
{
	if( pPlayer->InTitan() )
		SetTitanAppearanceToEngineObject( pPlayer, &pPlayer->m_EngineObject );
	else
		SetCharacterAppearanceToEngineObject(pPlayer,&pPlayer->m_EngineObject);
	HideWeapon(pPlayer,pPlayer->GetEngineObject());
	
	///////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ����/����
	if(pPlayer->GetSingleSpecialState(eSingleSpecialState_Hide))
	{
		OBJECTMGR->SetHide(pPlayer, TRUE);
	}
	///////////////////////////////////////////////////////////////////
	else if(pPlayer->GetCharacterTotalInfo()->bVisible == FALSE)	
	{
#ifdef _GMTOOL_
		if( MAINGAME->GetUserLevel() <= eUSERLEVEL_GM && CAMERA->GetCameraMode() != eCM_EyeView )
			pPlayer->GetEngineObject()->SetAlpha( 0.3f );
		else
#endif
			pPlayer->GetEngineObject()->HideWithScheduling();
	}

	OBJECTMGR->ApplyShadowOption(pPlayer);
}

void CAppearanceManager::SetCharacterAppearance(CPlayer* pPlayer)
{
	SetCharacterAppearanceToEngineObject(pPlayer,&pPlayer->m_EngineObject);

	if(pPlayer->GetState() == eObjectState_None)
		OBJECTSTATEMGR->EndObjectState(pPlayer,eObjectState_None);
	if( pPlayer->GetState() == eObjectState_Move &&
		pPlayer->m_MoveInfo.KyungGongIdx == 0)
		pPlayer->SetState(eObjectState_Move);

	/////////////////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ����/����
	if(pPlayer->IsHide())
		OBJECTMGR->SetHide(pPlayer, TRUE);
	
	if(pPlayer->IsDetect())
		OBJECTMGR->SetDetect(pPlayer, TRUE);
	
	/////////////////////////////////////////////////////////////////////////////////////

	if(pPlayer->GetCharacterTotalInfo()->bVisible == FALSE)	
	{
#ifdef _GMTOOL_
		if( MAINGAME->GetUserLevel() <= eUSERLEVEL_GM && CAMERA->GetCameraMode() != eCM_EyeView )
			pPlayer->GetEngineObject()->SetAlpha( 0.3f );
		else
#endif
			pPlayer->GetEngineObject()->HideWithScheduling();
			//pPlayer->GetEngineObject()->Hide();
	}
	
	OBJECTMGR->ApplyShadowOption(pPlayer);
}

void CAppearanceManager::SetBossMonsterAppearance(CBossMonster* pMonster)
{
	pMonster->m_EngineObject.Release();
	
	WORD mkind = pMonster->GetMonsterKind();
	BASE_MONSTER_LIST* pList = GAMERESRCMNGR->GetMonsterListInfo(mkind);
	
	pMonster->m_EngineObject.Init(pList->ChxName, pMonster, eEngineObjectType_Monster);
	pMonster->m_EngineObject.ApplyHeightField(TRUE);
	
	//////////////////////////////////////////////////////////////////////////
	// ���� ũ�� ����
	VECTOR3 Scale;
	// taiyo test
	Scale.x = Scale.y = Scale.z = pList->Scale;
	pMonster->m_EngineObject.SetScale(&Scale);
}

void CAppearanceManager::SetMonsterAppearance(CMonster* pMonster)
{
	pMonster->m_EngineObject.Release();
	
	WORD mkind = pMonster->GetMonsterKind();

	//////////////////////////////////////////////////////////////////////////
	// ��Ÿ���� üũ�� ���� �ӽ� �ڵ�
	if(mkind == 145)
		mkind = 145;
	//////////////////////////////////////////////////////////////////////////

	BASE_MONSTER_LIST* pList = GAMERESRCMNGR->GetMonsterListInfo(mkind);
	pMonster->m_EngineObject.Init(pList->ChxName,pMonster,eEngineObjectType_Monster);
	pMonster->m_EngineObject.ApplyHeightField(TRUE);


	/////////////////////////////////////////////////////////////////////////
	// taiyo 
	// ��� ���� �ε�
	/*
	char chtName[255];
//	char tmp[255]; 
	int lens = strlen(pMonsterChxName) - 4;
	strncpy( chtName, pMonsterChxName, lens );
	chtName[lens] = 0;
	#ifdef _FILE_BIN_
		strcat(chtName, ".bin");
	#else
		strcat(chtName, ".cht");
	#endif
	MOTIONMGR->LoadMotionInfo(pMonster, chtName);	
	*/
	MOTIONMGR->SetMotionInfo(pMonster, pList->MotionID);

	//////////////////////////////////////////////////////////////////////////
	// ���� ũ�� ����
	VECTOR3 Scale;
	// taiyo test
	Scale.x = Scale.y = Scale.z = pList->Scale;
	pMonster->m_EngineObject.SetScale(&Scale);
}
void CAppearanceManager::SetNpcAppearance(CNpc* pNpc)
{
	pNpc->m_EngineObject.Release();
		
	WORD nkind = pNpc->GetNpcKind();
	NPC_LIST* pList = GAMERESRCMNGR->GetNpcInfo(nkind);
	int ChxNum = pList->ModelNum;
	char* pNpcChxName = GAMERESRCMNGR->GetNpcChxName(ChxNum);
	
	pNpc->m_EngineObject.Init(pNpcChxName,pNpc,eEngineObjectType_Npc);
	pNpc->SetPosition(&pNpc->m_MoveInfo.CurPosition);
	pNpc->m_EngineObject.ApplyHeightField(TRUE);
	pNpc->SetAngle(pNpc->m_RotateInfo.Angle.ToRad());
	
	pNpc->m_EngineObject.ChangeMotion(1);

	pNpc->m_EngineObject.RandMotionFrame();
	
	if( pNpc->GetNpcJob() == 0 )
		pNpc->m_EngineObject.DisablePick();

	//////////////////////////////////////////////////////////////////////////
	// Npc ũ�� ����
	VECTOR3 Scale;
	//
	Scale.x = Scale.y = Scale.z = pList->Scale;
	pNpc->m_EngineObject.SetScale(&Scale);
}

void CAppearanceManager::SetPetAppearance(CPet* pPet)
{
	pPet->m_EngineObject.Release();

	WORD mKind = pPet->GetPetKind();
	WORD PetGrade = pPet->GetPetGrade();

	BASE_PET_LIST* pList = GAMERESRCMNGR->GetPetListInfo(mKind);
	
	pPet->m_EngineObject.Init(pList->ChxName_PerGrade[PetGrade-1],pPet,eEngineObjectType_Pet);

	pPet->SetPosition(&pPet->m_MoveInfo.CurPosition);
	pPet->m_EngineObject.ApplyHeightField(TRUE);
	pPet->SetAngle(pPet->m_RotateInfo.Angle.ToRad());

	pPet->m_EngineObject.ChangeMotion(1);

	MOTIONMGR->SetMotionInfo(pPet, pList->MotionID);

	VECTOR3 Scale;
	Scale.x = Scale.y = Scale.z = pList->Scale;
	pPet->m_EngineObject.SetScale(&Scale);
}

/*
void CAppearanceManager::SetPetAppearance(CPetBase* pPet)
{
	pPet->m_EngineObject.Release();


	WORD mkind = 42;
	BASE_MONSTER_LIST* pList = GAMERESRCMNGR->GetMonsterListInfo(mkind);
	pPet->m_EngineObject.Init(pList->ChxName,pPet,eEngineObjectType_Pet);

	pPet->SetPosition(&pPet->m_MoveInfo.CurPosition);
	pPet->m_EngineObject.ApplyHeightField(TRUE);
	pPet->SetAngle(pPet->m_RotateInfo.Angle.ToRad());
	
	pPet->m_EngineObject.ChangeMotion(1);

//	pPet->m_EngineObject.RandMotionFrame();

	MOTIONMGR->SetMotionInfo(pPet, pList->MotionID);

	//////////////////////////////////////////////////////////////////////////
	// ���� ũ�� ����
	VECTOR3 Scale;
	// taiyo test
	Scale.x = Scale.y = Scale.z = pList->Scale;
	pPet->m_EngineObject.SetScale(&Scale);
}*/

void CAppearanceManager::SetTitanAppearance(CPlayer* pPlayer)
{
	SetTitanAppearanceToEngineObject( pPlayer, &pPlayer->m_EngineObject );

	if(pPlayer->GetState() == eObjectState_None)
		OBJECTSTATEMGR->EndObjectState(pPlayer,eObjectState_None);
	/*if( pPlayer->GetState() == eObjectState_Move &&
		pPlayer->m_MoveInfo.KyungGongIdx == 0)
		pPlayer->SetState(eObjectState_Move);*/

	if(pPlayer->GetCharacterTotalInfo()->bVisible == FALSE)	
	{
#ifdef _GMTOOL_
		if( MAINGAME->GetUserLevel() <= eUSERLEVEL_GM && CAMERA->GetCameraMode() != eCM_EyeView )
			pPlayer->GetEngineObject()->SetAlpha( 0.3f );
		else
#endif
		pPlayer->GetEngineObject()->HideWithScheduling();
	}

	OBJECTMGR->ApplyShadowOption( pPlayer );
}


void CAppearanceManager::SetMapObjectAppearance(CMapObject* pMapObject, char* strData, float fScale )
{
	pMapObject->m_EngineObject.Release();

	pMapObject->m_EngineObject.Init( strData, pMapObject, eEngineObjectType_MapObject) ;

	pMapObject->SetPosition( &pMapObject->m_MoveInfo.CurPosition );
	pMapObject->m_EngineObject.ApplyHeightField(TRUE);
	pMapObject->SetAngle( pMapObject->m_RotateInfo.Angle.ToRad() );

	VECTOR3 Scale;
	Scale.x = Scale.y = Scale.z = fScale;
	pMapObject->m_EngineObject.SetScale(&Scale);
}

// magi82 - ������ �ٹ̱� ������Ʈ
void CAppearanceManager::SetDecorationAppearance(CMapObject* pMapObject, CPlayer* pPlayer, DWORD dwStreetStallKind)
{
	pMapObject->m_EngineObject.Release();
	DWORD dwItemIdx = pPlayer->GetShopItemStats()->dwStreetStallDecoration;
	if( 0 == dwItemIdx )
		return;

	ITEM_INFO* pInfo = ITEMMGR->GetItemInfo((WORD)dwItemIdx);
	if( !pInfo )
		return;
	char* strTemp = NULL;
	if(dwStreetStallKind == eSK_SELL)
	{
		strTemp = GAMERESRCMNGR->GetItemChxName(pInfo->Part3DModelNum);
	}
	else
	{
		strTemp = GAMERESRCMNGR->GetItemChxName((DWORD)pInfo->LifeRecoverRate);
	}
		
	pMapObject->m_EngineObject.Init( strTemp, NULL, eEngineObjectType_Npc);
	pMapObject->m_EngineObject.ApplyHeightField(TRUE);
	VECTOR3 pos = pPlayer->GetCurPosition();
	pos.x += 30;
	pos.z += 30;
	pMapObject->SetPosition( &pos );

	pMapObject->SetAngle(pPlayer->GetAngle());

	pMapObject->m_EngineObject.ChangeMotion(1);
}

void CAppearanceManager::SetCharacterAppearanceToEffect(CPlayer* pPlayer,CEngineEffect* pRtEffect)
{
	SetCharacterAppearanceToEngineObject(pPlayer,pRtEffect);
}

#define TITAN_EFFECT_MAINTAIN	3620
void CAppearanceManager::InitAppearance(CObject* pObject)
{
	BYTE objectKind = pObject->GetObjectKind();
	if(objectKind == eObjectKind_Player)
	{
		if( ((CPlayer*)pObject)->InTitan() || ((CPlayer*)pObject)->IsTitanPreView() )
		{
			SetTitanAppearance( (CPlayer*)pObject );

			/*if( FALSE == ((CPlayer*)pObject)->IsTitanPreView() )
			{
				OBJECTEFFECTDESC desc(TITAN_EFFECT_MAINTAIN);
				pObject->AddObjectEffect( TITAN_MAINTAIN_EFFECTID, &desc, 1 );
			}*/
		}
		else
		{
			SetCharacterAppearance( (CPlayer*)pObject );
			//pObject->RemoveObjectEffect( TITAN_MAINTAIN_EFFECTID );
		}
		//SetTitanAppearance( (CPlayer*)pObject );		
	}
	else if( (objectKind == eObjectKind_Monster) || (objectKind == eObjectKind_TitanMonster) )
	{
		SetMonsterAppearance((CMonster*)pObject);
	}
	else if(objectKind == eObjectKind_Pet)
	{
		SetPetAppearance((CPet*)pObject);
	}
	else if(objectKind == eObjectKind_Npc)
	{
		SetNpcAppearance((CNpc*)pObject);
	}
	else if(objectKind == eObjectKind_Tactic)
	{
	}
	else if(objectKind == eObjectKind_BossMonster)
	{
		SetBossMonsterAppearance((CBossMonster*)pObject);
	}
	else if(objectKind == eObjectKind_Mining)
	{
		SetMonsterAppearance((CMonster*)pObject);
	}
	else if(objectKind == eObjectKind_Collection)
	{
		SetMonsterAppearance((CMonster*)pObject);
	}
	else if(objectKind == eObjectKind_Hunt)
	{
		SetMonsterAppearance((CMonster*)pObject);
	}
}

void CAppearanceManager::SetScalebyGuageBar(DWORD PlayerID)
{
	CObject* pPlayer;	
	pPlayer = OBJECTMGR->GetObject(PlayerID);
	if(pPlayer == NULL)
	{
		ASSERT(0);
		return;
	}
	ASSERT(pPlayer->GetObjectKind() == eObjectKind_Player);

	((CPlayer*)pPlayer)->m_CharacterInfo.Width = CHARMAKEMGR->GetCharacterMakeInfo()->Width;
	((CPlayer*)pPlayer)->m_CharacterInfo.Height = CHARMAKEMGR->GetCharacterMakeInfo()->Height;
}

void CAppearanceManager::AddCharacterPartChange(DWORD PlayerID)
{
	m_PtrList.AddHead((void*)PlayerID);
}

void CAppearanceManager::ProcessAppearance()
{
	DWORD PlayerID= 0;
	CPlayer* pPlayer;
	while( PlayerID = (DWORD)m_PtrList.RemoveTail() )
	{
		pPlayer = (CPlayer*)OBJECTMGR->GetObject( PlayerID );
		if( pPlayer == NULL )
			continue;
		ASSERT( pPlayer->GetObjectKind() == eObjectKind_Player );

		if( pPlayer->InTitan() || pPlayer->IsTitanPreView() )
		{
			pPlayer->SetTitanMoveSpeed();
			SetTitanAppearance( pPlayer );
		}
		else
			SetCharacterAppearance( pPlayer );		
	}
}


void CAppearanceManager::ShowWeapon(CPlayer* pPlayer,CEngineObject* pEngineObject)
{
	WORD WeaponIdx = 0;
	MOD_LIST* pModList = NULL;
	BYTE gender = pPlayer->m_CharacterInfo.Gender;
	ASSERT(gender < 2);
	WORD EquipType = 0;
	int PartType = 0;
	int PartModelNum = 0;
	CEngineObject* pWeapon1 = NULL;
	CEngineObject* pWeapon2 = NULL;
	int n;  // 修复C2065: 在函数作用域声明n

	// Ÿ��ź ž�� �Ǵ� �̸������� ���
	if( pPlayer->InTitan() || pPlayer->IsTitanPreView() )		
	{
		WeaponIdx = pPlayer->GetTitanWearedItemIdx(eTitanWearedItem_Weapon);
		pModList = &GAMERESRCMNGR->m_TitanModFileList;

		HideWeapon(pPlayer,pEngineObject);

		EquipType = pPlayer->GetTitanWeaponEquipType();

		if( WeaponIdx )
		{
			PartType = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DType;
			PartModelNum = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DModelNum;
		}
		else
			return;

		if( PartType != -1 && PartType != 3 )
		{
			if( EquipType == WP_AMGI )
			{
				pWeapon1 = new CEngineObject;
				pWeapon1->Init(pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Titan);
				pEngineObject->AttachWeapon(pWeapon1,"Bip01 R Forearm");
			}
			else if( EquipType == WP_GWUN )
			{
				pWeapon1 = new CEngineObject;
				pWeapon2 = new CEngineObject;

				char temp[256],*pSrc;
				pSrc = pModList->ModFile[PartModelNum];
				int nLen = strlen(pSrc);

				for(int n=0;n<nLen;++n)
				{
					if( pSrc[n] == '.' )
					{
						temp[n] = '_';
						temp[n+1] = 'R';
						strcpy(&temp[n+2],&pSrc[n]);
						break;
					}
					else
						temp[n] = pSrc[n];
				}
				pWeapon1->Init(temp,NULL,eEngineObjectType_Titan);
				BOOL rt = pEngineObject->AttachWeapon(pWeapon1,"Bip01 R Forearm");

				for(n=0;n<nLen;++n)
				{
					if( pSrc[n] == '.')
					{
						temp[n] = '_';
						temp[n+1] = 'L';
						strcpy(&temp[n+2],&pSrc[n]);
						break;
					}
					else
						temp[n] = pSrc[n];
				}
					
				pWeapon2->Init(temp,NULL,eEngineObjectType_Titan);
				rt = pEngineObject->AttachWeapon(pWeapon2,"Bip01 L Forearm");
			}
			else
			{
				pWeapon1 = new CEngineObject;
				pWeapon1->Init(pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Titan);
				pEngineObject->AttachWeapon(pWeapon1,WEAPON_ATTACH_BONENAME_R);
			}
		}
	}
	// ĳ���� ������ ���
	else
	{
		WeaponIdx = pPlayer->GetWearedItemIdx(eWearedItem_Weapon);
		pModList = &GAMERESRCMNGR->m_ModFileList[gender];


		HideWeapon(pPlayer,pEngineObject);

		EquipType = pPlayer->GetWeaponEquipType();

		if( EquipType < WP_KEY && pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Gum+EquipType-1] > 1)
		{
			ITEM_INFO* pinfo = ITEMMGR->GetItemInfo( pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Gum+EquipType-1] );
			if( pinfo )
			{
				PartType = pinfo->Part3DType;
				PartModelNum = pinfo->Part3DModelNum;
			}
			else
			{
				PartType = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DType;
				PartModelNum = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DModelNum;
			}
		}
		else if( WeaponIdx )
		{
			PartType = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DType;
			PartModelNum = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DModelNum;
		}
		else
			return;

		// 2007. 10. 17. CBH - ���� ������ ������ �ƹ�Ÿ ���� üũ
		WORD wAvartarCheck = 0;
		switch(EquipType)
		{
		case WP_AMGI:	//�ϱ�
			{
				wAvartarCheck = pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Amgi];
			}
			break;
		case WP_GUNG:	//��
			{
				wAvartarCheck = pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Gung];
			}
			break;
		case WP_GWUN:	//��
			{
				wAvartarCheck = pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Gwun];
			}
			break;
		case WP_DO:	//��
			{
				wAvartarCheck = pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Do];
			}
			break;
		case WP_CHANG:	//â
			{
				wAvartarCheck = pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Chang];
			}
			break;
		case WP_GUM:	//��
			{
				wAvartarCheck = pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Gum];
			}
			break;
		}

		if( PartType != -1 && PartType != 3 )
		{
			if( EquipType == WP_AMGI )
			{				
				UNIQUE_ITEM_OPTION_INFO* pInfo = GAMERESRCMNGR->GetUniqueItemOptionList(WeaponIdx);
				if(pInfo && wAvartarCheck <= 1)
				{
					CEngineObject* pObject = NULL;
					char buf[MAX_CHXNAME_LENGTH+1] = { 0, };

					if(gender == 0)
						SafeStrCpy(buf, pInfo->strManEffectName, MAX_CHXNAME_LENGTH+1);
					else
						SafeStrCpy(buf, pInfo->strWomanEffectName, MAX_CHXNAME_LENGTH+1);

					if( strcmp(buf, "nofile") != 0 )
					{
						pObject = new CEngineObject;
						pObject->Init( buf, NULL, eEngineObjectType_Weapon, eAttachObjectType_withAni );
						pEngineObject->AttachWeapon(pObject,"Bip01 L Forearm");

						for( int i = 0; i < (int)strlen(buf); i++ )
						{
							if( buf[i] == '.' )
							{
								buf[i-1] = 'R';
								break;
							}
						}

						pObject = new CEngineObject;
						pObject->Init( buf, NULL, eEngineObjectType_Weapon, eAttachObjectType_withAni );
						pEngineObject->AttachWeapon(pObject,"Bip01 R Forearm");
					}
				}
				else
				{
					char temp[256],*pSrc;
					pSrc = pModList->ModFile[PartModelNum];
					int nLen = strlen(pSrc);

					for(int n=0;n<nLen;++n)
					{
						if( pSrc[n] == '.' )
						{
							temp[n] = '_';
							temp[n+1] = 'R';
							strcpy(&temp[n+2],&pSrc[n]);
							break;
						}
						else
							temp[n] = pSrc[n];
					}
					pWeapon1 = new CEngineObject;
					pWeapon1->Init(temp,NULL,eEngineObjectType_Weapon);
					BOOL rt = pEngineObject->AttachWeapon(pWeapon1,"Bip01 R Forearm");
					SetPlusItemEffect(pPlayer, pEngineObject, WeaponIdx, "Bip01 R Forearm");	//+10 �̻� ������ ����Ʈ ����

					for(n=0;n<nLen;++n)
					{
						if( pSrc[n] == '.')
						{
							temp[n] = '_';
							temp[n+1] = 'L';
							strcpy(&temp[n+2],&pSrc[n]);
							break;
						}
						else
							temp[n] = pSrc[n];
					}
					pWeapon2 = new CEngineObject;	
					pWeapon2->Init(temp,NULL,eEngineObjectType_Weapon);
					rt = pEngineObject->AttachWeapon(pWeapon2,"Bip01 L Forearm");
					SetPlusItemEffect(pPlayer, pEngineObject, WeaponIdx, "Bip01 L Forearm");	//+10 �̻� ������ ����Ʈ ����
				}
			}
			else if( EquipType == WP_GUNG )
			{				
				UNIQUE_ITEM_OPTION_INFO* pInfo = GAMERESRCMNGR->GetUniqueItemOptionList(WeaponIdx);
				if(pInfo && wAvartarCheck <= 1)
				{
					CEngineObject* pObject = NULL;
					char buf[MAX_CHXNAME_LENGTH+1] = { 0, };

					if(gender == 0)
						SafeStrCpy(buf, pInfo->strManEffectName, MAX_CHXNAME_LENGTH+1);
					else
						SafeStrCpy(buf, pInfo->strWomanEffectName, MAX_CHXNAME_LENGTH+1);

					if( strcmp(buf, "nofile") != 0 )
					{
						pObject = new CEngineObject;
						pObject->Init( buf, NULL, eEngineObjectType_Weapon, eAttachObjectType_withAni );
						pEngineObject->AttachWeapon(pObject,WEAPON_ATTACH_BONENAME_L);
					}
				}
				else
				{
					pWeapon1 = new CEngineObject;
					pWeapon1->Init(pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);
					pEngineObject->AttachWeapon(pWeapon1,WEAPON_ATTACH_BONENAME_L);					
					SetPlusItemEffect(pPlayer, pEngineObject, WeaponIdx, WEAPON_ATTACH_BONENAME_L);	//+10 �̻� ������ ����Ʈ ����
				}
			}
			else if( EquipType == WP_GWUN )
			{
				pWeapon1 = new CEngineObject;
				pWeapon1->Init(pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);
				pEngineObject->AttachWeapon(pWeapon1,WEAPON_ATTACH_BONENAME_R);
				SetPlusItemEffect(pPlayer, pEngineObject, WeaponIdx, "Bip01 R Forearm");	//+10 �̻� ������ ����Ʈ ����
				
				char temp[256],*pSrc;
				pSrc = pModList->ModFile[PartModelNum];
				for(DWORD n=0;n<strlen(pSrc);++n)
				{
					if(pSrc[n] == '.')
					{
						temp[n] = '_';
						temp[n+1] = 'L';
						strcpy(&temp[n+2],&pSrc[n]);
						break;
					}
					else
						temp[n] = pSrc[n];
				}
				pWeapon2 = new CEngineObject;
				pWeapon2->Init(temp,NULL,eEngineObjectType_Weapon);
				pEngineObject->AttachWeapon(pWeapon2,WEAPON_ATTACH_BONENAME_L);
				SetPlusItemEffect(pPlayer, pEngineObject, WeaponIdx, "Bip01 L Forearm");	//+10 �̻� ������ ����Ʈ ����
			}
			else
			{				
				UNIQUE_ITEM_OPTION_INFO* pInfo = GAMERESRCMNGR->GetUniqueItemOptionList(WeaponIdx);				
				if(pInfo && wAvartarCheck <= 1)
				{
					CEngineObject* pObject = NULL;
					char buf[MAX_CHXNAME_LENGTH+1] = { 0, };

					if(gender == 0)
						SafeStrCpy(buf, pInfo->strManEffectName, MAX_CHXNAME_LENGTH+1);
					else
						SafeStrCpy(buf, pInfo->strWomanEffectName, MAX_CHXNAME_LENGTH+1);

					if( strcmp(buf, "nofile") != 0 )
					{
						pObject = new CEngineObject;
						pObject->Init( buf, NULL, eEngineObjectType_Weapon, eAttachObjectType_withAni );
						pEngineObject->AttachWeapon(pObject,WEAPON_ATTACH_BONENAME_R);
						//hGxAttachObject = pObject->GetGXOHandle();
						//BOOL rt = g_pExecutive->GXOAttach( hGxAttachObject, hGxObject, "Bone_R" );            
					}
				}
				else
				{
					pWeapon1 = new CEngineObject;
					pWeapon1->Init(pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);
					pEngineObject->AttachWeapon(pWeapon1,WEAPON_ATTACH_BONENAME_R);
					SetPlusItemEffect(pPlayer, pEngineObject, WeaponIdx, WEAPON_ATTACH_BONENAME_R);	//+10 �̻� ������ ����Ʈ ����
				}
			}
		}
		else if( PartType == 3 && EquipType == WP_GWUN )
		{			
			UNIQUE_ITEM_OPTION_INFO* pInfo = GAMERESRCMNGR->GetUniqueItemOptionList(WeaponIdx);

			if(pInfo && wAvartarCheck <= 1)
			{
				char buf[MAX_CHXNAME_LENGTH+1] = { 0, };

				if(gender == 0)
					SafeStrCpy(buf, pInfo->strManEffectName, MAX_CHXNAME_LENGTH+1);
				else
					SafeStrCpy(buf, pInfo->strWomanEffectName, MAX_CHXNAME_LENGTH+1);

				if( strcmp(buf, "nofile") != 0 )
				{
					CEngineObject* pObject = new CEngineObject;
					pObject->Init(buf, NULL, eEngineObjectType_Weapon, eAttachObjectType_withAni);
					pEngineObject->AttachWeapon(pObject,"Bip01 L Forearm");

					for( int i = 0; i < (int)strlen(buf); i++ )
					{
						if( buf[i] == '.' )
						{
							buf[i-1] = 'R';
							break;
						}
					}

					CEngineObject* pObject2 = new CEngineObject;
					pObject2->Init(buf, NULL, eEngineObjectType_Weapon, eAttachObjectType_withAni);
					pEngineObject->AttachWeapon(pObject2,"Bip01 R Forearm");
				}				
			}
			else
			{
				SetPlusItemEffect(pPlayer, pEngineObject, WeaponIdx, "Bip01 R Forearm");	//+10 �̻� ������ ����Ʈ ����
				SetPlusItemEffect(pPlayer, pEngineObject, WeaponIdx, "Bip01 L Forearm");	//+10 �̻� ������ ����Ʈ ����
			}
		}
/*
		GXOBJECT_HANDLE hGxObject = pEngineObject->GetGXOHandle();
		GXOBJECT_HANDLE hGxAttachObject = NULL;
		CEngineObject* pEffect = NULL;
		char buf[MAX_CHXNAME_LENGTH] = { 0, };
		UNIQUE_ITEM_OPTION_INFO* pInfo = GAMERESRCMNGR->GetUniqueItemOptionList(WeaponIdx);

		if( pInfo )
		{
			if(gender == 0)
				SafeStrCpy(buf, pInfo->strManEffectName, MAX_CHXNAME_LENGTH+1);
			else
				SafeStrCpy(buf, pInfo->strWomanEffectName, MAX_CHXNAME_LENGTH+1);

			if( strcmp(buf, "nofile" ) == 0 )
			{
				pEffect = new CEngineObject;
				pEffect->Init( buf, NULL, eEngineObjectType_Effect, eAttachObjectType_withAni );
				hGxAttachObject = pEffect->GetGXOHandle();
				BOOL rt = g_pExecutive->GXOAttach( hGxAttachObject, hGxObject, "Bone_R" );            
			}
		}
*/
	}	
}

void CAppearanceManager::SetPlusItemEffect(CPlayer* pPlayer, CEngineObject* pEngineObject, DWORD dwItemIdx, char* pObjectName)
{
	ITEM_INFO* pItemInfo = ITEMMGR->GetItemInfo(dwItemIdx);
	if(pItemInfo == NULL)
		return;

	//2008. 6. 26. CBH - ������ ������ �������� �Ծ������� ����Ʈ�� ���� �ʴ´�
	for(int i = eAvatar_Weared_Gum ; i <= eAvatar_Weared_Amgi ; i++)
	{
		if(pPlayer->GetShopItemStats()->Avatar[i] > 1)
			return;
	}		

	//������ ����� 10 �̻��̸� +10 ������ �̻��̴�.
	if(pItemInfo->ItemGrade >= ePLUSITEM_EFFECT_10)
	{
		PLUSITEM_EFFECT_INFO* pPlusItemEffectInfo = GAMERESRCMNGR->GetPlusItemEffectInfo(pPlayer->GetWeaponEquipType());
		if(pPlusItemEffectInfo == NULL)
			return;

		//�迭�� 0���ͽ����̹Ƿ� ������ ��޿��� -10�� �Ѵ�.
		WORD wIndex = pItemInfo->ItemGrade - ePLUSITEM_EFFECT_10;
		if(wIndex < 0)
			return;

		//���� ���� ����Ʈ ������ �ٸ��⶧���� ó��
		char szCHXName[MAX_CHXNAME_LENGTH+1];
		memset(szCHXName, 0, sizeof(szCHXName));
		if(pPlayer->GetGender() == GENDER_MALE)	//����
			sprintf(szCHXName, "m_%s", pPlusItemEffectInfo->szCHXName[wIndex]);
		else //����
			sprintf(szCHXName, "w_%s", pPlusItemEffectInfo->szCHXName[wIndex]);

		CEngineObject* pEffectObject = new CEngineObject;		
		pEffectObject->Init(szCHXName, NULL, eEngineObjectType_Weapon);
		pEngineObject->AttachWeapon(pEffectObject, pObjectName);
	}			
}

/*
void CAppearanceManager::ShowWeapon(CPlayer* pPlayer,CEngineObject* pEngineObject)
{
	WORD WeaponIdx = 0;
	if( pPlayer->InTitan() || pPlayer->IsTitanPreView() )		
		WeaponIdx = pPlayer->GetTitanWearedItemIdx(eTitanWearedItem_Weapon);
	else
		WeaponIdx = pPlayer->GetWearedItemIdx(eWearedItem_Weapon);

	BYTE gender = pPlayer->m_CharacterInfo.Gender;
	ASSERT(gender < 2);

	MOD_LIST* pModList = NULL;
	if( pPlayer->InTitan() || pPlayer->IsTitanPreView() )
		pModList = &GAMERESRCMNGR->m_TitanModFileList;
	else
		pModList = &GAMERESRCMNGR->m_ModFileList[gender];

	HideWeapon(pPlayer,pEngineObject);


	WORD EquipType = 0;
	if( pPlayer->InTitan() || pPlayer->IsTitanPreView() )
		EquipType = pPlayer->GetTitanWeaponEquipType();
	else
		EquipType = pPlayer->GetWeaponEquipType();

	int PartType = 0;
	int PartModelNum = 0;

	if( EquipType < WP_KEY &&
		pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Gum+EquipType-1] > 1)
	{
		ITEM_INFO* pinfo = ITEMMGR->GetItemInfo( pPlayer->GetShopItemStats()->Avatar[eAvatar_Weared_Gum+EquipType-1] );
		if( pinfo )
		{
			PartType = pinfo->Part3DType;
			PartModelNum = pinfo->Part3DModelNum;
		}
		else
		{
			PartType = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DType;
			PartModelNum = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DModelNum;
		}
	}
	else if( WeaponIdx )
	{
		PartType = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DType;
		PartModelNum = ITEMMGR->GetItemInfo(WeaponIdx)->Part3DModelNum;
	}
	else
		return;

	if( PartType != -1 &&
		PartType != 3 )
	{
		CEngineObject* pWeapon1 = new CEngineObject;

		if( EquipType == WP_AMGI && !pPlayer->InTitan() && !pPlayer->IsTitanPreView() )
		{
			char temp[256],*pSrc;
			pSrc = pModList->ModFile[PartModelNum];
			int nLen = strlen(pSrc);

			for(int n=0;n<nLen;++n)
			{
				if(pSrc[n] == '_' || pSrc[n] == '.' )
				{
					temp[n] = '_';
					temp[n+1] = 'R';
					strcpy(&temp[n+2],&pSrc[n]);
					break;
				}
				else
					temp[n] = pSrc[n];
			}
			pWeapon1->Init(temp,NULL,eEngineObjectType_Weapon);
			BOOL rt = pEngineObject->AttachWeapon(pWeapon1,"Bip01 R Forearm");

			for(n=0;n<nLen;++n)
			{
				if(pSrc[n] == '_' || pSrc[n] == '.')
				{
					temp[n] = '_';
					temp[n+1] = 'L';
					strcpy(&temp[n+2],&pSrc[n]);
					break;
				}
				else
					temp[n] = pSrc[n];
			}
			CEngineObject* pWeapon2 = new CEngineObject;	
			pWeapon2->Init(temp,NULL,eEngineObjectType_Weapon);
			rt = pEngineObject->AttachWeapon(pWeapon2,"Bip01 L Forearm");
			return;
		}

		if( pPlayer->InTitan() || pPlayer->IsTitanPreView() )
			pWeapon1->Init(pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Titan);
		else
			pWeapon1->Init(pModList->ModFile[PartModelNum],NULL,eEngineObjectType_Weapon);

		if(EquipType == WP_GUNG && !pPlayer->InTitan() && !pPlayer->IsTitanPreView())
			pEngineObject->AttachWeapon(pWeapon1,WEAPON_ATTACH_BONENAME_L);
		else if( (EquipType == WP_GWUN || EquipType == WP_AMGI ) && (pPlayer->InTitan() || pPlayer->IsTitanPreView()) )
			pEngineObject->AttachWeapon(pWeapon1,"Bip01 R Forearm");
		else
			pEngineObject->AttachWeapon(pWeapon1,WEAPON_ATTACH_BONENAME_R);

		
		//CEngineObject* pEff = NULL;
		//pEff = new CEngineObject;
		//pEff->Init( "do_eff.chx", NULL, eEngineObjectType_Effect, eAttachObjectType_withAni );
		//hGxAttachObject = pEff->GetGXOHandle();
		//rt = g_pExecutive->GXOAttach( hGxAttachObject, hGxObject, "Bone_R" );

		if( EquipType == WP_GWUN )
		{
			CEngineObject* pWeapon2 = new CEngineObject;
			char temp[256],*pSrc;
			pSrc = pModList->ModFile[PartModelNum];
			for(DWORD n=0;n<strlen(pSrc);++n)
			{
				if(pSrc[n] == '.')
				{
					temp[n] = '_';
					temp[n+1] = 'L';
					strcpy(&temp[n+2],&pSrc[n]);
					break;
				}
				else
					temp[n] = pSrc[n];
			}
			if( pPlayer->InTitan() || pPlayer->IsTitanPreView() )
			{
				pWeapon2->Init(temp,NULL,eEngineObjectType_Titan);
				pEngineObject->AttachWeapon(pWeapon2,"Bip01 L Forearm");
			}
			else
			{
				pWeapon2->Init(temp,NULL,eEngineObjectType_Weapon);
				pEngineObject->AttachWeapon(pWeapon2,WEAPON_ATTACH_BONENAME_L);
			}			
		}
	}
}
*/


void CAppearanceManager::HideWeapon(CPlayer* pPlayer,CEngineObject* pEngineObject)
{
	if( !pPlayer->InTitan() )
		pEngineObject->RemoveAllAttachedWeapon();
}

void CAppearanceManager::SetAvatarItem( CPlayer* pPlayer, WORD ItemIdx, BOOL bAdd )
{
	WORD* pAvatarItem = pPlayer->GetShopItemStats()->Avatar;
	AVATARITEM* pAvatarUseInfo = GAMERESRCMNGR->m_AvatarEquipTable.GetData( ItemIdx );
	if( !pAvatarUseInfo )		return;
	
	if( bAdd )
	{
		for(int i=0; i<eAvatar_Max; i++)
		{
			if( i >= eAvatar_Weared_Hair )
			{
				if( !pAvatarUseInfo->Item[i] )
					pAvatarItem[i] = 0;
			}
			else if( i == pAvatarUseInfo->Position )
			{
				if( pAvatarItem[i] )				
				if( pPlayer->GetID() == HERO->GetID() )
				{
					MSG_WORD2 msg;
					msg.Category = MP_ITEM;
					msg.Protocol = MP_ITEM_SHOPITEM_AVATAR_CHANGE;
					msg.dwObjectID = HERO->GetID();
					msg.wData1 = i;						// �����ġ�� ����Ÿ����
					msg.wData2 = pAvatarItem[i];					
					NETWORK->Send( &msg, sizeof(msg) );
				}

				// ������ �������� ���������� �⺻�������� �ٽ� �������ش�.
				AVATARITEM* pTemp = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pAvatarItem[i] );
				if( !pTemp )		continue;
				for(int k=0; k<eAvatar_Max; k++)
				{
					if( k >= eAvatar_Weared_Hair )
					if( !pTemp->Item[k] )
						pAvatarItem[k] = 1;
				}

				pAvatarItem[i] = ItemIdx;
			}
			else if( !pAvatarUseInfo->Item[i] && pAvatarItem[i] )
			{				
				if( pPlayer->GetID() == HERO->GetID() )
				{
					MSG_WORD msg;
					msg.Category = MP_ITEM;
					msg.Protocol = MP_ITEM_SHOPITEM_AVATAR_TAKEOFF;
					msg.dwObjectID = HERO->GetID();
					msg.wData = ItemIdx;					// �����ġ�� ����Ÿ����					
					NETWORK->Send( &msg, sizeof(msg) );
				}				
				
				// ������ �������� ���������� �⺻�������� �ٽ� �������ش�.
				AVATARITEM* pAvatarUseInfo = GAMERESRCMNGR->m_AvatarEquipTable.GetData( pAvatarItem[i] );
				if( !pAvatarUseInfo )		continue;
				for(int k=0; k<eAvatar_Max; k++)
				{
					if( k >= eAvatar_Weared_Hair )
					if( !pAvatarUseInfo->Item[k] )
						pAvatarItem[k] = 1;					
				}

				pAvatarItem[i] = 0;
			}
		}
	}
	else
	{		
		for(int i=0; i<eAvatar_Max; i++)
		{
			if( i < eAvatar_Weared_Hair )
			{
				if( i == pAvatarUseInfo->Position )				
					pAvatarItem[i] = 0;
			}			
			else
			{
				if( !pAvatarUseInfo->Item[i] )
					pAvatarItem[i] = 1;
			}
		}		
	}
	
	AddCharacterPartChange( pPlayer->GetID() );
}



#ifndef _RESOURCE_WORK_NOT_APPLY_


VOID CAppearanceManager::CancelReservation(CObject* pObject)
{
	if (FALSE == m_bUseReservation)
	{
		return;
	}

	if (NULL == pObject)
	{
		return;
	}

	if (pObject->GetObjectKind() == eObjectKind_Player)
	{
		RESERVATION_ITEM_PLAYER* pReservItem = (RESERVATION_ITEM_PLAYER*)pObject->GetEngineObject()->GetCacheListData();

		if (NULL != pReservItem)
		{
			m_lstReserveToAddPlayerList.Remove((void*)pReservItem);
		}

		pObject->GetEngineObject()->SetCacheListData(NULL);
	}
}

VOID CAppearanceManager::CancelAlphaProcess(CObject* pObject)
{
	if (FALSE == m_bUseReservation)
	{
		return;
	}

	if (NULL == pObject)
	{
		return;
	}

	if (pObject->GetObjectKind() == eObjectKind_Player)
	{
		pObject->GetEngineObject()->SetAlpha(1.0f);
		m_lstAlphaProcessing.Remove(pObject);
	}
}

BOOL CAppearanceManager::ReserveToAppearObject(CObject* pObject, void* pMsg)
{
	if (FALSE == m_bUseReservation)
	{
		InitAppearance(pObject);
		return FALSE;
	}
		
	if(pObject->GetObjectKind() == eObjectKind_Player)
	{	

		RESERVATION_ITEM_PLAYER* pReservItem = AllocReservationItem();
		if (NULL != pReservItem)
		{
			pReservItem->pObject	= (CPlayer*)pObject;
			pReservItem->Msg		= *(SEND_CHARACTER_TOTALINFO*)pMsg;

			m_lstReserveToAddPlayerList.AddTail((void*)pReservItem);	
			
			pObject->GetEngineObject()->SetCacheListData((void*)pReservItem);			
		}
		else
		{
			pObject->GetEngineObject()->SetCacheListData(NULL);
		}
		
	}
	/*
	else if(pObject->GetObjectKind() == eObjectKind_Monster)
	{		
		m_lstReserveToAddMonsterList.AddTail((void*)pObject);		
	}
	*/
	else
	{
		InitAppearance(pObject);
	}
	
	return TRUE;
}


#define REACTION_PROCESS_RESERVATION_TIME	200
#define LOAD_OBJECT_NUMBER_PER_PROCESS		3


VOID CAppearanceManager::ProcessReservation(void)
{
	if (FALSE == m_bUseReservation)
	{		
		return;
	}	
	

	DWORD	dwCurrentTick = GetTickCount();
	int		iLoopLimit = 0;
	int		i = 0;

	if (dwCurrentTick >= m_dwLastProcessReservationTime + REACTION_PROCESS_RESERVATION_TIME)
	{	
		for(int i=0; i<LOAD_OBJECT_NUMBER_PER_PROCESS; ++i)
		{
			if (0 < m_lstReserveToAddPlayerList.GetCount())
			{
				RESERVATION_ITEM_PLAYER* pReservItem = NULL;
				pReservItem = (RESERVATION_ITEM_PLAYER*)m_lstReserveToAddPlayerList.RemoveHead();

				if (NULL != pReservItem)
				{
					CPlayer* pPlayer = pReservItem->pObject;

					if (NULL != pPlayer)
					{
						InitAppearance(pPlayer);
						
						PostProcessAddPlayer(pPlayer, &pReservItem->Msg);
						
						if( pPlayer->GetCharacterTotalInfo()->bVisible )
						{
							pPlayer->GetEngineObject()->SetAlpha(0.2f);
							m_lstAlphaProcessing.AddTail(pPlayer);
						}
					}

					FreeReservationItem(pReservItem);
					pReservItem = NULL;

					pPlayer->GetEngineObject()->SetCacheListData(NULL);

				}
			}
		}

		// player
		
		/*
		if ( LOAD_OBJECT_NUMBER_PER_PROCESS >= m_lstReserveToAddPlayerList.GetCount() )
		{
			iLoopLimit = m_lstReserveToAddPlayerList.GetCount();
		}
		else
		{
			iLoopLimit = LOAD_OBJECT_NUMBER_PER_PROCESS;
		}
		
		int i = 0;		
		RESERVATION_ITEM_PLAYER* pReservItem = NULL;
		for (i = 0; i < iLoopLimit; i++) 
		{
			pReservItem = (RESERVATION_ITEM_PLAYER*)m_lstReserveToAddPlayerList.RemoveHead();

			if (NULL != pReservItem)
			{
				CPlayer* pPlayer = pReservItem->pObject;

				if (NULL != pPlayer)
				{
					InitAppearance(pPlayer);		
					
					//pPlayer->GetEngineObject()->SetCompoundAlpha(50);			

					PostProcessAddPlayer(pPlayer, &pReservItem->Msg);				

					pPlayer->GetEngineObject()->SetAlpha(0.2f);
					m_lstAlphaProcessing.AddTail(pPlayer);
				}

				FreeReservationItem(pReservItem);
				pReservItem = NULL;


			}							
		}	
		*/

		// monster

		/*
		CMonster* pMonster = NULL;

		if ( LOAD_OBJECT_NUMBER_PER_PROCESS >= m_lstReserveToAddMonsterList.GetCount() )
		{
			iLoopLimit = m_lstReserveToAddMonsterList.GetCount();
		}
		for (i = 0; i < iLoopLimit; i++)
		{			
			pMonster = (CMonster*)m_lstReserveToAddMonsterList.RemoveHead();
			InitAppearance(pMonster);	
			
			
			//pMonster->GetEngineObject()->SetCompoundAlpha(100);
			//m_lstAlphaProcessing.AddTail(pMonster);
			
		}
		*/


		// npc

		m_dwLastProcessReservationTime = dwCurrentTick;
	}
}



RESERVATION_ITEM_PLAYER* CAppearanceManager::AllocReservationItem(VOID)
{

	if (m_dwUsedReserItemPlayerNum < MAX_RESERVATION_ITEM_PLAYER)
	{
		++m_dwUsedReserItemPlayerNum;
		return (RESERVATION_ITEM_PLAYER*)m_lstFreeReserItemPlayer.RemoveHead();
	}

	return NULL;
}

BOOL CAppearanceManager::FreeReservationItem(RESERVATION_ITEM_PLAYER* pItem)
{
	if (NULL == pItem)
	{
		return FALSE;
	}

	if (0 < m_dwUsedReserItemPlayerNum)
	{
		--m_dwUsedReserItemPlayerNum;
		m_lstFreeReserItemPlayer.AddTail((void*)pItem);
	}

	return TRUE;
}


VOID CAppearanceManager::ProcessAlphaAppearance(VOID)
{		
	float fAlpha = 0.0f;

	cPtrList removelist;

	PTRLISTPOS pos = m_lstAlphaProcessing.GetHeadPosition();
	
	while( pos )
	{
		CObject* pObject = (CObject*)m_lstAlphaProcessing.GetNext(pos);
		
		if( pObject )
		{
			fAlpha = pObject->GetEngineObject()->GetAlpha();			
			
			if (0.9f <= fAlpha)
			{
				pObject->GetEngineObject()->SetAlpha(1.0f);					
				
				//m_lstAlphaProcessing.GetNext(pos);
				//m_lstAlphaProcessing.Remove(pObject);
				removelist.AddTail(pObject);
			}
			else
			{
				fAlpha += 0.025f;
				
				pObject->GetEngineObject()->SetAlpha(fAlpha);			
			}
		}
	}

	pos = removelist.GetHeadPosition();
	while( pos )
	{
		CObject* pObject = (CObject*)removelist.GetNext(pos);
		if( pObject )
			m_lstAlphaProcessing.Remove( pObject );
	}
	removelist.RemoveAll();
}



DWORD ConfirmAddPlayer( CPlayer* pPlayer, SEND_CHARACTER_TOTALINFO* pmsg )
{
	if( !pPlayer || !pmsg )		return eConfirmAddPlayer_NoData;

	WORD TournamentTeam = 0;

	CAddableInfoIterator iter(&pmsg->AddableInfo);
	BYTE AddInfoKind;
	while((AddInfoKind = iter.GetInfoKind()) != CAddableInfoList::None)
	{
		ySWITCH(AddInfoKind)			
			yCASE(CAddableInfoList::GTournament)
				iter.GetInfoData( &TournamentTeam );
		yENDSWITCH
		
		iter.ShiftToNextData();
	}

	if( TournamentTeam == 2 )			return eConfirmAddPlayer_DontAddPlayer;

	return eConfirmAddPlayer_NoErr;
}

void PostProcessAddPlayer(CPlayer* pPlayer, SEND_CHARACTER_TOTALINFO* pmsg)
{
	if (NULL == pPlayer)
	{
		return;
	}

	if (NULL == pmsg)
	{
		return;
	}


	OBJECTSTATEMGR->InitObjectState(pPlayer);
	OBJECTMGR->ApplyOverInfoOption( pPlayer );
	OBJECTMGR->ApplyShadowOption( pPlayer );

	///


	BOOL bVimu = FALSE;
	BOOL bPartyWar = FALSE;
	BOOL bGTournament = FALSE;
	BOOL bSiegeWar = FALSE;
	int wPartyTeam = 0;
	int wGTournamentTeam = 0;
	WORD StallKind = 0;
	SEIGEWAR_CHARADDINFO SiegeWarInfo;
	COMPRESSEDPOS VimuPos;
	static char StreetStallTitle[MAX_STREETSTALL_TITLELEN+1] = {0,};
	BOOL bInTitan = FALSE;
	TITAN_APPEARANCEINFO titanAppearance;

	CAddableInfoIterator iter(&pmsg->AddableInfo);
	BYTE AddInfoKind;
	while((AddInfoKind = iter.GetInfoKind()) != CAddableInfoList::None)
	{
		ySWITCH(AddInfoKind)			
			yCASE(CAddableInfoList::VimuInfo)
				bVimu = TRUE;
				iter.GetInfoData(&VimuPos);
			yCASE(CAddableInfoList::StreetStall)
				StallKind = eSK_SELL;
				iter.GetInfoData(StreetStallTitle);
			yCASE(CAddableInfoList::StreetBuyStall)
				StallKind = eSK_BUY;
				iter.GetInfoData(StreetStallTitle);
			yCASE(CAddableInfoList::PartyWar)
				bPartyWar = TRUE;
				iter.GetInfoData( &wPartyTeam );
			yCASE(CAddableInfoList::GTournament)
				bGTournament = TRUE;
				iter.GetInfoData( &wGTournamentTeam );
			yCASE(CAddableInfoList::SiegeWar)
				bSiegeWar = TRUE;
				iter.GetInfoData( &SiegeWarInfo );	
			yCASE(CAddableInfoList::TitanAppearanceInfo)
				bInTitan = TRUE;
				iter.GetInfoData( &titanAppearance );
		yENDSWITCH
		
		iter.ShiftToNextData();
	}

	if( bInTitan = pmsg->bInTitan )
	{
		pPlayer->RidingTitan(bInTitan);
		pPlayer->SetTitanAppearanceInfo(&titanAppearance);
		APPEARANCEMGR->InitAppearance(pPlayer);	// ��ġ ��¿ �� ����.
	}

	if( pmsg->TotalInfo.bPKMode )
	{
		if( pPlayer->InTitan() )
		{
			OBJECTEFFECTDESC desc(FindEffectNum("t_maintain_PK_S.beff"));
			pPlayer->AddObjectEffect( PK_EFFECT_ID, &desc, 1 );						
		}
		else
		{
			OBJECTEFFECTDESC desc(FindEffectNum("maintain_PK_S.beff"));
			pPlayer->AddObjectEffect( PK_EFFECT_ID, &desc, 1 );
		}
	}

	//SW051112 ���ָ��
	if((pPlayer->GetCharacterTotalInfo()->bVisible == FALSE)
		||(!pPlayer->GetSingleSpecialState(eSingleSpecialState_Hide)))
	{
		if( pmsg->TotalInfo.bMussangMode )
		{
#ifdef _JAPAN_LOCAL_
			OBJECTEFFECTDESC desc(FindEffectNum("maintain_mussang.beff"));
			pPlayer->AddObjectEffect( MUSSANG_EFFECT_ID, &desc, 1, pPlayer);
#else
			// 06. 03. �������� - �̿���
			switch(pPlayer->GetStage())
			{
			case eStage_Normal:	
				{
					OBJECTEFFECTDESC desc(1035);
					pPlayer->AddObjectEffect( MUSSANG_EFFECT_ID1, &desc, 1, pPlayer);
				}
				break;
			case eStage_Hwa:		
			case eStage_Hyun:
				{
					OBJECTEFFECTDESC desc(888);
					pPlayer->AddObjectEffect( MUSSANG_EFFECT_ID2, &desc, 1, pPlayer);
				}
				break;
			case eStage_Geuk:
			case eStage_Tal:
				{
					OBJECTEFFECTDESC desc(889);
					pPlayer->AddObjectEffect( MUSSANG_EFFECT_ID3, &desc, 1, pPlayer);
				}
				break;
			}
#endif			
		}

	}
	if(pmsg->bLogin)
	{
		TARGETSET set;
		set.pTarget = pPlayer;
		EFFECTMGR->StartEffectProcess(eEffect_NewCharacter,pPlayer,&set,0,pPlayer->GetID());
		pPlayer->ChangeBaseMotion( pPlayer->GetStandardMotion() );
	}
	else
	{
		// ���õ� �������� ���� �ٲ��� by Stiner(8)
		DWORD	dwObjectIndex = GAMEIN->GetStallFindDlg()->GetSelectedObjectIndex();
		DWORD	dwPrevType = GAMEIN->GetStallFindDlg()->GetPrevSelectedType();
		//--
		if( pmsg->BaseObjectInfo.ObjectState == eObjectState_StreetStall_Owner )
		{
			if(StallKind == eSK_SELL)
				pPlayer->ShowStreetStallTitle( TRUE, StreetStallTitle );
			else if(StallKind == eSK_BUY)
				pPlayer->ShowStreetBuyStallTitle( TRUE, StreetStallTitle );

			// magi82 - ������ �ٹ̱� ������Ʈ
			// �׸��� ������ �÷��̾ ���� �Ǿ����� �� �÷��̾ ���������̰� ������ �ٹ̱� �������� ��� �ߴٸ� �ٹ̱⸦ Add ���ش�.
			if( pPlayer->GetShopItemStats()->dwStreetStallDecoration )
				STREETSTALLMGR->AddStreetStallDecoration(pPlayer, StallKind);

			// ������ ���� �ٲ��� by Stiner(8)
			if( dwObjectIndex == pPlayer->GetID() )
			{
				if( StallKind == dwPrevType )
                    pPlayer->SetStreetStallBalloonImage( dwPrevType, TRUE );
			}
		}
		else
		{ 
			if( dwObjectIndex == pPlayer->GetID() )
			{
				pPlayer->SetStreetStallBalloonImage( dwPrevType, FALSE );
				GAMEIN->GetStallFindDlg()->SetSelectedObjectIndex(0);
				GAMEIN->GetStallFindDlg()->SetPrevSelectedType(0);
			}
			//--
		}
	}

	if( bVimu )
	{
		if( pmsg->BaseObjectInfo.BattleTeam == eBattleTeam1 )
		{
			if( pPlayer->InTitan() )
			{
				OBJECTEFFECTDESC desc(FindEffectNum("t_eff_bmu_A_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );	//SW HERO�� �ɼ� ���� �����ϰ� ����Ʈ ������ ����..����
			}
			else
			{
				OBJECTEFFECTDESC desc(FindEffectNum("eff_bmu_A_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );	//SW HERO�� �ɼ� ���� �����ϰ� ����Ʈ ������ ����..����
			}
		}
		else if( pmsg->BaseObjectInfo.BattleTeam == eBattleTeam2 )
		{
			if( pPlayer->InTitan() )
			{
				OBJECTEFFECTDESC desc(FindEffectNum("t_eff_bmu_B_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
			else
			{
				OBJECTEFFECTDESC desc(FindEffectNum("eff_bmu_B_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
		}
	}

	if( bPartyWar )
	{
		if( wPartyTeam == 1 )
		{
			if( pPlayer->InTitan() )
			{
				OBJECTEFFECTDESC desc(FindEffectNum("t_eff_bmu_A_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
			else
			{
				OBJECTEFFECTDESC desc(FindEffectNum("eff_bmu_A_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
		}
		else if( wPartyTeam == 2 )
		{
			if( pPlayer->InTitan() )
			{
				OBJECTEFFECTDESC desc(FindEffectNum("t_eff_bmu_B_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
			else
			{
				OBJECTEFFECTDESC desc(FindEffectNum("eff_bmu_B_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
		}
	}
	
	if( bGTournament )
	{
		if( wGTournamentTeam == 0 )
		{
			if( pPlayer->InTitan() )
			{
				OBJECTEFFECTDESC desc(FindEffectNum("t_eff_bmu_A_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
			else
			{
				OBJECTEFFECTDESC desc(FindEffectNum("eff_bmu_A_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
			pPlayer->GetCharacterTotalInfo()->bVisible = true;
		}
		else if( wGTournamentTeam == 1 )
		{
			if( pPlayer->InTitan() )
			{
				OBJECTEFFECTDESC desc(FindEffectNum("t_eff_bmu_B_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}
			else
			{
				OBJECTEFFECTDESC desc(FindEffectNum("eff_bmu_B_s.beff"));
				pPlayer->AddObjectEffect( BATTLE_TEAMEFFECT_ID, &desc, 1, HERO );
			}

			pPlayer->GetCharacterTotalInfo()->bVisible = true;
		}
		else if( wGTournamentTeam == 2 )
		{
			// Observer
			pPlayer->GetCharacterTotalInfo()->bVisible = false;
			if( MAINGAME->GetUserLevel() <= eUSERLEVEL_GM && CAMERA->GetCameraMode() != eCM_EyeView )
				pPlayer->GetEngineObject()->SetAlpha( 0.3f );
			else
				pPlayer->GetEngineObject()->HideWithScheduling();
		}		
	}

	if( bSiegeWar && HERO )
	{
		((CBattle_SiegeWar*)BATTLESYSTEM->GetBattle(HERO))->SetSiegeWarName( pPlayer, SiegeWarInfo.Team, pPlayer->GetGuildIdx() );

		if( SiegeWarInfo.bEngrave )
		{
			OBJECTSTATEMGR->StartObjectState(pPlayer, eObjectState_Engrave);
			pPlayer->ChangeMotion(329, TRUE);

			OBJECTEFFECTDESC desc(FindEffectNum("eff_gakin_s.beff"));
			pPlayer->AddObjectEffect( ENGRAVE_EFFECTID, &desc, 1, HERO );			
		}

		if( pPlayer->IsRestraintMode() )
		{
			pPlayer->SetRestraintMode( TRUE );
			OBJECTMGR->ApplyOverInfoOption( pPlayer );

			if( pPlayer->InTitan() )
			{
				OBJECTEFFECTDESC desc(FindEffectNum("t_maintain_PK_S.beff"));
				pPlayer->AddObjectEffect( PK_EFFECT_ID, &desc, 1 );						
			}
			else
			{
				OBJECTEFFECTDESC desc(FindEffectNum("maintain_PK_S.beff"));
				pPlayer->AddObjectEffect( PK_EFFECT_ID, &desc, 1 );
			}
		}
	}

	if( pPlayer->GetState() == eObjectState_Engrave )
	{
		OBJECTEFFECTDESC desc(FindEffectNum("eff_gakin_s.beff"));
		pPlayer->AddObjectEffect( ENGRAVE_EFFECTID, &desc, 1, HERO );
	}
}




#endif

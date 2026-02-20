#ifndef __COMMONSTRUCT_H__
#define __COMMONSTRUCT_H__

// VS2022: 添加YHLibrary路径
#include "D:\mxym\[Lib]YHLibrary\hashtable.h"

#include "CommonDefine.h"
#include "CommonGameDefine.h"
#include "CommonGameStruct.h"
// 仅在服务器构建时包含 ServerGameDefine.h
#ifndef _MHCLIENT_
#include "ServerGameDefine.h"
#else
#include "ClientGameDefine.h"
#endif
#include "vector.h"
#include "protocol.h"
#include "..\[CC]Ability\AbilityCommonHeader.h"
#include "AddableInfo.h"


#pragma pack(push,1)
/**********************************************************************************/
/*			�Ʒ��� Ŭ�������� ���Ǵ� DEFINE			 						  */
/**********************************************************************************/
#define MAX_NETWORK_MSG	256

typedef WORD MAPTYPE;
typedef INT64 EXPTYPE;
typedef WORD LEVELTYPE;

typedef DWORD DURTYPE;
typedef WORD POSTYPE;
typedef DWORD ITEMPARAM;

typedef int FAMETYPE;
typedef DWORD WANTEDTYPE;

typedef DWORD MARKNAMETYPE;

typedef DWORD			MONEYTYPE;
typedef double			DOUBLE;		// 添加DOUBLE类型定义
#define MAXMONEY		MAXULONG_PTR		// 0xffffffff(4294967295)

struct MSGROOT
{
	BYTE CheckSum;
#ifdef _CRYPTCHECK_ 
	MSGROOT():Code(0){}
	char Code;
#endif
	BYTE Category;
	BYTE Protocol;
};
struct MSGBASE :public MSGROOT
{
	DWORD dwObjectID;
};

struct MAPSERVER_TRANSDATAONOFF : public MSGROOT // ON/OFF ���� ��Ŷ
{
	int MapServerGroupNum;
	int MapServerNum;
	int HardNum;
};

struct MAPSERVER_TRANSDATA : public MSGROOT // �Ϲ� ���� ��Ŷ
{
	int MapServerGroupNum;
	int MapServerNum;
	int HardNum;
};

struct MSGBASEDATA :public MSGROOT //�޽��� ���ۿ�
{
	int MapServerGroupNum;
	int MapServerNum;
	char Messagebuff[1028];
};

struct MSGCRYPTDATA : public MSGROOT
{
#ifdef _CRYPTCHECK_ 
	HselInit eninit;
	HselInit deinit;
#endif
};

struct COMPRESSEDPOS
{
	WORD wx;
	WORD wz;
	void Compress(VECTOR3* pPos)
	{
		wx = WORD(pPos->x);
		wz = WORD(pPos->z);
	}
	void Decompress(VECTOR3* pRtPos)
	{
		pRtPos->x = wx;
		pRtPos->z = wz;
		pRtPos->y = 0;
	}
};
struct COMPRESSEDTARGETPOS
{
	void Init()
	{
		PosNum = 0;
		memset( pos, 0, sizeof(COMPRESSEDPOS)*MAX_CHARTARGETPOSBUF_SIZE );
//		memset( this, 0, sizeof(COMPRESSEDTARGETPOS) );
	}
	void AddTargetPos(VECTOR3 * pPos)
	{
		if( PosNum >= MAX_CHARTARGETPOSBUF_SIZE )
		{
			ASSERT(0);
			return;
		}
		pos[PosNum++].Compress(pPos);
	}
	int GetSize()
	{
		return sizeof(COMPRESSEDTARGETPOS) - sizeof(COMPRESSEDPOS)*(MAX_CHARTARGETPOSBUF_SIZE-PosNum);
	}
	int GetSurplusSize()
	{
		return sizeof(COMPRESSEDPOS)*(MAX_CHARTARGETPOSBUF_SIZE-PosNum);
	}
	BYTE PosNum;
	COMPRESSEDPOS pos[MAX_CHARTARGETPOSBUF_SIZE];
};

struct SEND_MOVEINFO
{
	COMPRESSEDPOS CurPos;
	bool MoveMode;
	WORD KyungGongIdx;
	WORD AbilityKyungGongLevel;
	VECTOR2	Move_Direction;	//SW071129 ���� �߰�. ĳ���� add �� ��� ���� ���� ���� �ִ� ����� ����! .y�� .z���̴�.
};

struct BASEMOVE_INFO
{
	VECTOR3 CurPosition;
private:
	BYTE CurTargetPositionIdx;
	BYTE MaxTargetPositionIdx;
	VECTOR3 TargetPositions[MAX_CHARTARGETPOSBUF_SIZE];
public:
	bool bMoving;
	bool MoveMode;				// 0:�ٱ� 1:�ȱ�

	WORD KyungGongIdx;			// ���Idx 0: ������� �ƴϴ�  !0: ������� �����ȣ
	WORD AbilityKyungGongLevel;	// ��Ź� ����
	
	void SetFrom(SEND_MOVEINFO* pInfo)
	{
		bMoving = FALSE;
		pInfo->CurPos.Decompress(&CurPosition);
		MoveMode = pInfo->MoveMode;
		KyungGongIdx = pInfo->KyungGongIdx;
		AbilityKyungGongLevel = pInfo->AbilityKyungGongLevel;
	}
	BYTE GetCurTargetPosIdx() { return CurTargetPositionIdx; }
	BYTE GetMaxTargetPosIdx() { return MaxTargetPositionIdx; }
	BYTE & GetMaxTargetPosIdxRef() { return MaxTargetPositionIdx; }
	void SetCurTargetPosIdx(BYTE idx) { CurTargetPositionIdx=idx; }
	void SetMaxTargetPosIdx(BYTE idx) { MaxTargetPositionIdx=idx; }

	VECTOR3 * GetTargetPosition(BYTE idx) { return &TargetPositions[idx]; }
	VECTOR3 * GetTargetPositionArray() { return TargetPositions; }
	void InitTargetPosition()
	{
		CurTargetPositionIdx = 0;
		MaxTargetPositionIdx = 0;
		memset( TargetPositions, 0, sizeof( VECTOR3 ) * MAX_CHARTARGETPOSBUF_SIZE );
	}
	void SetTargetPosition( VECTOR3* pos )	{ memcpy( TargetPositions, pos, sizeof(VECTOR3)*MAX_CHARTARGETPOSBUF_SIZE );	}
	void SetTargetPosition( BYTE idx, VECTOR3 pos )	{ TargetPositions[idx] = pos;	}
};

struct MOVE_INFO : public BASEMOVE_INFO
{
	BOOL m_bLastMoving; // Ŭ���̾�Ʈ������ ����
	BOOL m_bEffectMoving;

	DWORD m_dwMoveCalculateTime;
	BOOL m_bMoveCalculateTime;

	// ���������� ����� �ð�.
	// ���������� ����� �ð��� �󸶵��� �ʾ����� �Ǵٽ� ������� �ʴ´�
	DWORD Move_LastCalcTime;

	DWORD Move_StartTime;
	VECTOR3 Move_StartPosition;
	VECTOR3 Move_Direction;

	float Move_EstimateMoveTime;

};

struct STATE_INFO
{
	int State_Start_Motion;
	int State_Ing_Motion;
	int State_End_Motion;
	DWORD State_End_MotionTime;
	DWORD State_End_Time;
	DWORD State_Start_Time;
	BYTE BeforeState;
	BYTE bEndState;
};


struct HERO_TOTALINFO
{
	WORD	wGenGol;						// �ٰ�
	WORD	wMinChub;						// ��ø
	WORD	wCheRyuk;						// ü��
	WORD	wSimMek;						// �ɸ�

	DWORD	wNaeRyuk;						// ����
	DWORD	wMaxNaeRyuk;					// �ִ� ����

	FAMETYPE	Fame;						// ����
	WORD	wKarma;							// ���ǵ�(��ġ��)

	EXPTYPE	ExpPoint;						// ����ġ
	DWORD	MaxMussangPoint;				// ���� �ִ�ġ
	LEVELTYPE	LevelUpPoint;				// ����������Ʈ		//??????
	MONEYTYPE	Money;						// ��

	DWORD	PartyID;						// ��Ƽ �ε���
	char	MunpaName[MAX_MUNPA_NAME+1];	// ���� �̸�
	WORD	KyungGongGrade;					// ��� ����
	
	DWORD	AbilityExp;						// Ư��ġ

	DWORD	Playtime;						// Play Time
	DWORD	LastPKModeEndTime;				// ������ ����� off�ð�
	LEVELTYPE MaxLevel;						// �ö����� �ְ� ����
	char	MunpaCanEntryDate[11];			// can create guild/be member left time 
	BYTE	ExpFlag;						// magi82 Levelup ����ȭ ���� �÷���
};

struct PET_TOTALINFO	//������
{
	DWORD	PetMasterUserID;		// MasterID
	DWORD	PetDBIdx;			// DB Index
	DWORD	PetSummonItemDBIdx;	// Item
	WORD	PetKind;		// ����
	WORD	PetGrade;		// ��ȭ�ܰ�
	DWORD	PetFriendly;	// ģ�е�
	DWORD	PetStamina;		// ���׹̳�
	BYTE	bAlive;			// ���� ����(ģ�е�0->����)
	BYTE	bSummonning;	// ��ȯ��	//���̵��� ��ȯ���̾��°�
	BYTE	bRest;			// �޽���	//���̵��� �޽����̾��°�
	//MAPTYPE MapNum;			// �ʿ�?
};

//SW070127 Ÿ��ź
struct TITAN_TOTALINFO
{
	TITAN_TOTALINFO() {	memset(this,0,sizeof(TITAN_TOTALINFO));	}

	DWORD	TitanMasterUserID;	// MasterUserID
	DWORD	TitanDBIdx;		
	DWORD	TitanCallItemDBIdx;
	WORD	TitanKind;
	WORD	TitanGrade;
	DWORD	Fuel;			// ����
	DWORD	Spell;			// ����
	DWORD	RecallTime;		// ���� ������ ���ȯ ���� �ð�.
	DWORD	RegisterCharacterIdx;
	BOOL	bRiding;		// ž�� ����	// �� �̵��� ���.
	//BOOL	bRegistered;	// ���� ���� -->RegisterCharacterIdx
	WORD	Scale;
	DWORD	MaintainTime;	// magi82(23)
};

//SW070127 Ÿ��ź
struct TITAN_ENDURANCE_ITEMINFO
{
	TITAN_ENDURANCE_ITEMINFO() { memset(this, 0, sizeof(TITAN_ENDURANCE_ITEMINFO)); }

	DWORD	CharacterID;
	DWORD	ItemDBIdx;		// �������� ���� ITEM�� DBIndex
	DWORD	ItemIdx;		// list������ idx	// �����ۺ� ������ ������ ���� �ʿ�.
	DWORD	Endurance;		// ���� ������ ��ġ
	DWORD	UserIdx;		// �ش� UserIdx
};

//struct TITAN_WEARINFO
//{
//	WORD	WearedItemIdx[eTitanWearedItem_Max];
//};

struct TITAN_APPEARANCEINFO
{
	WORD	TitanKind;
	WORD	TitanGrade;
	WORD	TitanScale;
	WORD	WearedItemIdx[eTitanWearedItem_Max];
};

struct SEND_TITAN_APPEARANCEINFO: public MSGBASE
{
	DWORD OwnerID;
	TITAN_APPEARANCEINFO titanAppearanceInfo;
};

struct MONSTER_TOTALINFO
{
	DWORD Life;
	DWORD Shield;
	WORD MonsterKind;
	WORD Group;	
	MAPTYPE MapNum;
	//char Name;					//�̰� �ߺ���..BASEOBJECT_INFO��  taiyo
};

struct NPC_TOTALINFO
{
	WORD NpcKind;
	WORD Group;
	MAPTYPE MapNum;
	WORD NpcUniqueIdx;
	WORD NpcJob;				// NPC_ROLE�� �ٲ�� ��! : taiyo
	
};

struct STATIC_NPCINFO
{
	MAPTYPE	MapNum;
	WORD	wNpcJob;
	WORD	wNpcUniqueIdx;
	VECTOR3	vPos;
	char NpcName[MAX_NAME_LENGTH+1];
};

struct QUESTNPCINFO
{
	DWORD	dwObjIdx;
	MAPTYPE	wMapNum;
	WORD	wNpcKind;
	char	sName[MAX_NAME_LENGTH+1];
	WORD	wNpcIdx;
	VECTOR3	vPos;
	float	fDir;	
};

//////////////////////////////////////////////////////////////////////////
// taiyo : ITEMBASE�� MUGONGBASE�� ���� Ŭ������ ����
struct ICONBASE
{
	DWORD	dwDBIdx;
	WORD	wIconIdx;		// ������->itemIdx, ����->mugongIdx
	POSTYPE Position;		// 
};

struct QUICKBASE
{
	POSTYPE Position;
};

//����������������������������������������������������������������������������
//�� ITEM_OPTION_INFO - Start												��
//����������������������������������������������������������������������������
class ITEM_OPTION_INFO
{
public:
	DWORD	dwOptionIdx;
	DWORD	dwItemDBIdx;
	WORD	GenGol;					//		�ٰ�+(����)	����� ���� �����ۿ� ������
	WORD	MinChub;				//		��ø+(����)	����� ���� �����ۿ� ������
	WORD	CheRyuk;				//		ü��+(����)	����� ���� �����ۿ� ������
	WORD	SimMek;					//		�ɸ�+(����)	����� ���� �����ۿ� ������
	DWORD	Life;					//		������+(����)	����� ���� �����ۿ� ������
	WORD	NaeRyuk;				//		������+(����)	����� ���� �����ۿ� ������
	DWORD	Shield;					//		ȣ�Ű���+(����)	����� ���� �����ۿ� ������
	ATTRIBUTEREGIST AttrRegist;		//		ȭ �Ӽ� ���׷�+(����)	����� ���� �����ۿ� ������
	WORD	PhyAttack;
	WORD	CriticalPercent;
	ATTRIBUTEATTACK AttrAttack;		//				ȭ�Ӽ����ݷ�+%
	WORD	PhyDefense;				//				���� ����+ 

	inline DWORD GetOptionType();
	inline float GetOptionValue( DWORD Type );
};
inline DWORD ITEM_OPTION_INFO::GetOptionType()
{
	DWORD type = 0;
	
	if( GenGol )
	{
		if( type )			return 0;
		type = eRJP_GENGOL;
	}
	if( MinChub )
	{
		if( type )			return 0;
		type = eRJP_MINCHUB;
	}
	if( CheRyuk )
	{
		if( type )			return 0;
		type = eRJP_CHERYUK;
	}
	if( SimMek )
	{
		if( type )			return 0;
		type = eRJP_SIMMEK;
	}
	if( Life )
	{
		if( type )			return 0;
		type = eRJP_LIFE;
	}
	if( NaeRyuk )
	{
		if( type )			return 0;
		type = eRJP_NAERYUK;
	}
	if( Shield )
	{
		if( type )			return 0;
		type = eRJP_SHIELD;
	}
	if( PhyAttack )
	{
		if( type )			return 0;
		type = eRJP_PHYATTACK;
	}
	if( CriticalPercent )
	{
		if( type )			return 0;
		type = eRJP_CRITICAL;
	}
	if( PhyDefense )
	{
		if( type )			return 0;
		type = eRJP_PHYDEFENSE;
	}
	for(int i=0; i<ATTR_MAX; ++i)
		if( AttrRegist.GetElement_Val( ATTR_FIRE+i ) )
		{
			if( type )			return 0;
			type = eRJP_FIREREGIST+i;
		}
	for(int i=0; i<ATTR_MAX; ++i)
		if( AttrAttack.GetElement_Val( ATTR_FIRE+i ) )
		{
			if( type )			return 0;
			type = eRJP_FIREATTACK+i;
		}

	return type;
}
inline float ITEM_OPTION_INFO::GetOptionValue( DWORD Type )
{
	float value = 0;

	switch( Type )
	{
	case eRJP_GENGOL:
		value = GenGol;
		break;
	case eRJP_MINCHUB:
		value = MinChub;
		break;
	case eRJP_CHERYUK:
		value = CheRyuk;
		break;
	case eRJP_SIMMEK:
		value = SimMek;
		break;
	case eRJP_LIFE:
		value = (float)Life;
		break;
	case eRJP_NAERYUK:
		value = NaeRyuk;
		break;
	case eRJP_SHIELD:
		value = (float)Shield;
		break;
	case eRJP_FIREREGIST:
		value = AttrRegist.GetElement_Val( ATTR_FIRE );
		break;
	case eRJP_WATERREGIST:
		value = AttrRegist.GetElement_Val( ATTR_WATER );
		break;
	case eRJP_TREEREGIST:
		value = AttrRegist.GetElement_Val( ATTR_TREE );
		break;
	case eRJP_GOLDREGIST:
		value = AttrRegist.GetElement_Val( ATTR_IRON );
		break;
	case eRJP_EARTHREGIST:
		value = AttrRegist.GetElement_Val( ATTR_EARTH );
		break;
	case eRJP_PHYATTACK:
		value = PhyAttack;
		break;
	case eRJP_CRITICAL:
		value = CriticalPercent;
		break;
	case eRJP_FIREATTACK:
		value = AttrAttack.GetElement_Val( ATTR_FIRE );
		break;
	case eRJP_WATERATTACK:
		value = AttrAttack.GetElement_Val( ATTR_WATER );
		break;
	case eRJP_TREEATTACK:
		value = AttrAttack.GetElement_Val( ATTR_TREE );
		break;
	case eRJP_GOLDATTACK:
		value = AttrAttack.GetElement_Val( ATTR_IRON );
		break;
	case eRJP_EARTHATTACK:
		value = AttrAttack.GetElement_Val( ATTR_EARTH );
		break;
	case eRJP_PHYDEFENSE:
		value = PhyDefense;
		break;
	}

	return value;
}
//����������������������������������������������������������������������������
//�� ITEM_OPTION_INFO - End													��
//����������������������������������������������������������������������������

//SW050909
//����������������������������������������������������������������������������
//�� ITEM_RARE_OPTION_INFO - End											��
//����������������������������������������������������������������������������
struct ITEM_RARE_OPTION_INFO
{
	//ITEM_RARE_OPTION_INFO():dwOptionIdx(0),dwItemDBIdx(0),GenGol(0),MinChub(0),
	//	CheRyuk(0),SimMek(0),Life(0),NaeRyuk(0),Shield(0),PhyAttack(0),CriticalPercent(0),PhyDefense(0){}
	ITEM_RARE_OPTION_INFO() { memset(this, 0, sizeof(ITEM_RARE_OPTION_INFO)); }
	DWORD	dwRareOptionIdx;
	DWORD	dwItemDBIdx;
	WORD	GenGol;
	WORD	MinChub;
	WORD	CheRyuk;
	WORD	SimMek;
	DWORD	Life;
	WORD	NaeRyuk;
	DWORD	Shield;
	WORD	PhyAttack;
//	WORD	CriticalPercent;
	WORD	PhyDefense;
	ATTRIBUTEREGIST AttrRegist;
	ATTRIBUTEATTACK AttrAttack;
};
//����������������������������������������������������������������������������
//�� ITEM_OPTION_INFO - End													��
//����������������������������������������������������������������������������

//////////////////////////////////////////////////////////////////////////
struct MUGONGBASE : public ICONBASE
{
	DWORD	ExpPoint;		// ��������ġ
	BYTE	Sung;			// ��������
	BYTE	bWear;			// ������Ʈ �ؾ� �ϴ����� ����
	POSTYPE	QuickPosition;	// ���� �����ܿ� ������ ��ġ

	//////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ���� ��ȯ �߰�
	WORD	OptionIndex;	// ���� ��ȯ �ɼ� �ε���
	//////////////////////////////////////////////////////////////////////////

};

struct MUGONG_TOTALINFO
{
	MUGONGBASE	mugong[SLOT_MUGONGTOTAL_NUM];		// ����+����
	//MUGONGBASE	Titanmugong[SLOT_MUGONGTOTAL_NUM];		// magi82 - Titan(070611) Ÿ��ź ������ȯ �ּ�ó��
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
struct ITEMBASE : public ICONBASE
{
//	DWORD DBIdx;
//	WORD ItemIdx;
	DURTYPE Durability;
	DWORD RareIdx;
//	POSTYPE bPosition;
	POSTYPE	QuickPosition;	// ���� �����ܿ� ������ ��ġ
	//BYTE MapNum;
	ITEMPARAM ItemParam;	
};
struct SLOTINFO
{
	BOOL	bLock;
	WORD	wPassword;
	WORD	wState;
};

struct ITEM_TOTALINFO
{
	ITEMBASE Inventory[SLOT_INVENTORY_NUM];
	ITEMBASE WearedItem[eWearedItem_Max];
	ITEMBASE ShopInventory[TABCELL_SHOPINVEN_NUM];
	ITEMBASE PetWearedItem[SLOT_PETWEAR_NUM];
	ITEMBASE TitanWearedItem[SLOT_TITANWEAR_NUM];	// magi82 - Titan(070209)
	ITEMBASE TitanShopItem[SLOT_TITANSHOPITEM_NUM];	// magi82 - Titan(070227)
	//WORD	QuickItem[MAX_QUICKITEM_NUM];	// ��������� 4
};

struct PET_INFO
{
	PET_TOTALINFO PetInfo[SLOT_INVENTORY_NUM + SLOT_SHOPINVEN_NUM];
};

struct TITAN_INFO
{
	TITAN_TOTALINFO TitanInfo[SLOT_INVENTORY_NUM];
};

struct TITAN_ENDURANCEINFO
{
	TITAN_ENDURANCE_ITEMINFO TitanEnduranceInfo[SLOT_INVENTORY_NUM + SLOT_SHOPINVEN_NUM];
};

struct PYOGUK_ITEM
{
	ITEMBASE Pyoguk[SLOT_PYOGUK_NUM];
};

struct PETINVEN_ITEM
{
	ITEMBASE PetInven[SLOT_PETINVEN_NUM];
};

struct PETACC_ITEM
{
	ITEMBASE PetAcc[SLOT_PETWEAR_NUM];
};

//////////////////////////////////////////////////////////////////////////


//------ ��Ƽ���� ����ϴ� ����ü -------------------------------
struct SEND_PARTY_GAUGE : public MSGBASE
{
	DWORD PlayerID;
	float GaugePercent;
};

struct PARTY_MEMBER
{
	DWORD dwMemberID;
	BOOL bLogged;
	BYTE LifePercent;
	BYTE ShieldPercent;
	BYTE NaeRyukPercent;
	char Name[MAX_NAME_LENGTH+1];
	LEVELTYPE Level;	
	WORD	posX;
	WORD	posZ;
};


//2008. 5. 21. CBH - ���� �ɼ� ����ü �߰�
struct PARTY_ADDOPTION : public MSGBASE
{
	DWORD dwPartyIndex;				//��Ƽ �ε���
	char szTheme[MAX_PARTY_NAME+1];	//���� ����
	LEVELTYPE wMinLevel;			//�ּҷ���
	LEVELTYPE wMaxLevel;			//�ִ뷹��
	BYTE bPublic;					//����, ����� ����
	WORD wLimitCount;				//�����ο� ����
	BYTE bOption;					//�й���
};

struct PARTY_MATCHINGLIST_INFO
{
	DWORD dwMemberID[MAX_PARTY_LISTNUM];
	char szMasterName[MAX_NAME_LENGTH+1];
	PARTY_ADDOPTION PartyAddOption;	//��Ƽ �߰� �ɼ�
};

struct PARTY_INFO : public MSGBASE 
{
	DWORD PartyDBIdx;
	BYTE Option;
	PARTY_MEMBER Member[MAX_PARTY_LISTNUM];

	PARTY_ADDOPTION PartyAddOption;	//��Ƽ �߰� �ɼ�
};

struct SEND_PARTY_MEMBER_POS : public MSGBASE
{
	DWORD	dwMemberID;
	WORD	posX;
	WORD	posZ;
};

struct SEND_PARTY_MEMBER_INFO : public MSGBASE 
{
	PARTY_MEMBER MemberInfo;
	DWORD PartyID;
};

struct SEND_CHANGE_PARTY_MEMBER_INFO : public MSGBASE
{
	DWORD PartyID;
	char Name[MAX_NAME_LENGTH+1];
	LEVELTYPE Level;
};

//------ ��Ƽ���� ����ϴ� ����ü -------------------------------

//////////////////////////////////////////////////////////////////////////



struct MAPSELECT_PARAM			// �ɸ��� ���鶧 ���ο��� ���̴� ����ü
{
	/*
	MAPSELECT_PARAM()
	{
		*Name=0;
		JobType =  0;
		SexType =  0;
		BodyType = 0;
		HairType = 0;
		FaceType = 0;
		StartArea = 0;
		bDuplCheck=FALSE;
	}
	*/
	char Name[MAX_NAME_LENGTH+1];			// ĳ���� �̸�
	BYTE	JobType;
	BYTE	SexType;
	BYTE	BodyType;
	BYTE	HairType;
	BYTE	FaceType;
	BYTE	StartArea;
	BOOL	bDuplCheck;
};

struct CHARACTERMAKEINFO : public MSGBASE
{
	char Name[MAX_NAME_LENGTH+1];			// ĳ���� �̸�
	DWORD	UserID;
	BYTE	SexType;
	BYTE	BodyType;
	BYTE	HairType;
	BYTE	FaceType;
	BYTE	StartArea;
	BOOL	bDuplCheck;
	WORD	WearedItemIdx[eWearedItem_Max];	// ������ �����Ƶ��� ����
	BYTE    StandingArrayNum;				// pjs �ɸ����� ��ġ ���� [5/22/2003]
	
	float	Height;
	float	Width;

#ifdef _JAPAN_LOCAL_
	WORD	wCharAttr;						// ĳ���� �Ӽ�
#endif
};

//////////////////////////////////////////////////////////////////////////
// taiyo : ���������� ����ϳ�?
struct CALCCHARACTER
{
	WORD wAttack;
	WORD wDefense;
	WORD wHit;
	WORD wDisengage;
	WORD wMove;
	WORD wLife;
	WORD wInner;

	WORD wHwa;
	WORD wSu;
	WORD wMok;
	WORD wKum;
	WORD wTo;
	WORD wDok;
};
//////////////////////////////////////////////////////////////////////////
// �Խ��� ����
struct AUCTION_BOARDTITLE
{
	DWORD BidIdx;		// ��Ź�ȣ(�ʼ������� �����ϴ� PrimaryKey)
	WORD ItemIdx;
	WORD ItemNum;
	DWORD EndDate;
	DWORD EndTime;
	DWORD CurrentPrice;
	DWORD ImmediatePrice;
};
//////////////////////////////////////////////////////////////////////////

// Weather System
struct stWeatherTime
{
	WORD	Index;
	WORD	StartDay;
	WORD	StartHour;
	WORD	StartMinute;
	WORD	EndDay;
	WORD	EndHour;
	WORD	EndMinute;
	WORD	State;
};

inline struct MSG_WEATHER : public MSGBASE
{
	WORD MapNum;
	stWeatherTime WeatherTime;
};

inline struct MSG_INT : public MSGBASE
{
	int nData;
};

inline struct MSG_INT2 : public MSGBASE
{
	int nData1;
	int nData2;
};

inline struct MSG_DWORD : public MSGBASE
{
	DWORD dwData;
};

inline struct MSG_DWORD2 : public MSGBASE
{
	DWORD dwData1;
	DWORD dwData2;
};

inline struct MSG_DWORDBYTE : public MSGBASE
{
	DWORD dwData;
	BYTE bData;
};

inline struct MSG_DWORD2BYTE : public MSGBASE
{
	DWORD dwData1;
	DWORD dwData2;
	BYTE bData;
};

inline struct MSG_DWORD3 : public MSGBASE
{
	DWORD dwData1;
	DWORD dwData2;
	DWORD dwData3;
};

inline struct MSG_DWORD4 : public MSGBASE
{
	DWORD dwData1;
	DWORD dwData2;
	DWORD dwData3;
	DWORD dwData4;
};

inline struct MSG_DWORD3BYTE2 : public MSGBASE
{
	DWORD	dwData1;
	DWORD	dwData2;
	DWORD	dwData3;
	BYTE	bData1;
	BYTE	bData2;
};

inline struct MSG_WORD : public MSGBASE
{
	WORD wData;
};

inline struct MSG_WORD2 : public MSGBASE
{
	WORD wData1;
	WORD wData2;
};
inline struct MSG_WORD3 : public MSGBASE
{
	WORD wData1;
	WORD wData2;
	WORD wData3;
};
inline struct MSG_WORD4 : public MSGBASE
{
	WORD wData1;
	WORD wData2;
	WORD wData3;
	WORD wData4;
};

inline struct MSG_DWORD_WORD : public MSGBASE
{
	DWORD dwData;
	WORD wData;
};

inline struct MSG_DWORD_WORD2 : public MSGBASE
{
	DWORD dwData;
	WORD wData1;
	WORD wData2;
};

inline struct MSG_BYTE : public MSGBASE
{
	BYTE bData;
};

inline struct MSG_TITAN_RIDEIN : public MSGBASE
{
	DWORD OwnerID;
	DWORD MaintainTime;	// magi82(23)
	TITAN_TOTALINFO			TitanTotalInfo;
	TITAN_APPEARANCEINFO	AppearanceInfo;
};

inline struct MSG_GUILDNOTICE : public MSGBASE
{
	DWORD dwGuildId;
	char Msg[MAX_GUILD_NOTICE+1];
	int GetMsgLength() { return sizeof(MSG_GUILDNOTICE) - MAX_GUILD_NOTICE + strlen(Msg); }
};

struct TESTMSG : public MSGBASE
{
	char Msg[MAX_CHAT_LENGTH+1];
	int GetMsgLength() { return sizeof(TESTMSG) - MAX_CHAT_LENGTH + strlen(Msg); }
};

struct TESTMSGID : public MSGBASE
{
	DWORD dwSenderID;
	char Msg[MAX_CHAT_LENGTH+1];
	int GetMsgLength() { return sizeof(TESTMSGID) - MAX_CHAT_LENGTH + strlen(Msg); }
};

//struct MSG_STR : public MSGBASE
//{
//	char Data[256];
//};

inline struct MSG_EVENT_MONSTERREGEN : public MSGBASE
{
	WORD	MonsterKind;
	BYTE	cbMobCount;		//mob count
	WORD	wMap;			//map
	BYTE	cbChannel;		//channel	= 0:all
	VECTOR3 Pos;
	WORD	wRadius;
	WORD	ItemID;
	DWORD	dwDropRatio;	//item drop ratio
//	BYTE	bBoss;			//�ʿ��Ѱ�?
};


struct MSG_FAME : public MSGBASE
{
	FAMETYPE Fame;
};


struct MSG_LOGIN_SYN : public MSGBASE
{
	DWORD AuthKey;
	char id[MAX_NAME_LENGTH+1];
	char pw[MAX_NAME_LENGTH+1];
	char pn[MAX_NAME_LENGTH+1];	//2008. 3. 18. CBH - ���ȹ�ȣ �߰�
	char Version[16];

#ifdef _KOR_LOCAL_
	BYTE gate;			// 0-����, 1-¯����
#endif
};


struct MSG_LOGIN_SYN_BUDDY : public MSG_LOGIN_SYN
{
    char AuthCode[256];
};

struct MSG_USE_DYNAMIC_SYN : public MSGBASE
{
	char id[MAX_NAME_LENGTH+1];
};

struct MSG_USE_DYNAMIC_ACK : public MSGBASE
{
	char mat[32];
};

struct MSG_LOGIN_DYNAMIC_SYN : public MSGBASE
{
	DWORD AuthKey;
	char id[MAX_NAME_LENGTH+1];
	char pw[MAX_NAME_LENGTH+1];
	char Version[16];
};

struct MSG_LOGIN_ACK : public MSGBASE
{
	char agentip[16];
	WORD agentport;
	DWORD userIdx;
	BYTE cbUserLevel;
};

struct MOVE_ONETARGETPOS : public MSGBASE
{
	DWORD			dwMoverID;
	COMPRESSEDPOS	sPos;
	COMPRESSEDPOS	tPos;

	void SetStartPos( VECTOR3 * pos )
	{
		sPos.Compress(pos);
	}
	void SetTargetPos( VECTOR3* pos )
	{
		tPos.Compress( pos );
	}
	void GetStartPos( VECTOR3 * pos )
	{
		sPos.Decompress( pos );
	}
	void GetTargetInfo(MOVE_INFO * pMoveInfo)
	{
		pMoveInfo->SetCurTargetPosIdx(0);
		pMoveInfo->SetMaxTargetPosIdx(1);
		tPos.Decompress(pMoveInfo->GetTargetPosition(0));
	}
	int GetSize()
	{
		return sizeof(MOVE_ONETARGETPOS);
	}
};

struct MOVE_ONETARGETPOS_FROMSERVER : public MSGBASE
{
	COMPRESSEDPOS	sPos;
	COMPRESSEDPOS	tPos;

	void SetStartPos( VECTOR3 * pos )
	{
		sPos.Compress(pos);
	}
	void GetStartPos( VECTOR3 * pos )
	{
		sPos.Decompress( pos );
	}
	void SetTargetPos( VECTOR3* pos )
	{
		tPos.Compress( pos );
	}
	void GetTargetInfo(MOVE_INFO * pMoveInfo)
	{
		pMoveInfo->SetCurTargetPosIdx(0);
		pMoveInfo->SetMaxTargetPosIdx(1);
		tPos.Decompress(pMoveInfo->GetTargetPosition(0));
	}
	int GetSize()
	{
		return sizeof(MOVE_ONETARGETPOS_FROMSERVER);
	}
};

struct MOVE_TARGETPOS : public MSGBASE
{
	DWORD dwMoverID;
	COMPRESSEDPOS spos;
	COMPRESSEDTARGETPOS tpos;
	void Init()
	{
		tpos.Init();
	}
	void SetStartPos(VECTOR3 * pos)
	{
		spos.Compress(pos);
	}
	void AddTargetPos(VECTOR3 * pos)
	{
		tpos.AddTargetPos(pos);
	}
	void GetStartPos(VECTOR3 * pos)
	{
		spos.Decompress(pos);
	}
	void GetTargetInfo(MOVE_INFO * pMoveInfo)
	{
		pMoveInfo->SetCurTargetPosIdx(0);//CurTargetPositionIdx = 0;
		pMoveInfo->SetMaxTargetPosIdx(tpos.PosNum);//pMoveInfo->MaxTargetPositionIdx = tpos.PosNum;
		ASSERT( tpos.PosNum <= MAX_CHARTARGETPOSBUF_SIZE );
		for(int i = 0 ; i < tpos.PosNum ; ++i)
			tpos.pos[i].Decompress(pMoveInfo->GetTargetPosition(i));//&pMoveInfo->TargetPositions[i]);
	}
	int GetSize()
	{
		return sizeof(MOVE_TARGETPOS) - tpos.GetSurplusSize();
	}
};


struct MOVE_POS : public MSGBASE
{
	DWORD dwMoverID;
	COMPRESSEDPOS cpos;
};


struct MOVE_POS_USER : public MSGBASE
{
	char	Name[MAX_NAME_LENGTH+1];
	COMPRESSEDPOS cpos;
};


struct MSG_WHERE_INFO : public MSGBASE
{
	WORD	wMapNum;		//�ʹ�ȣ
	BYTE	bChannel;		//ä�ι�ȣ
	COMPRESSEDPOS cpos;		//��ġ
};


struct BASEOBJECT_INFO
{
	BASEOBJECT_INFO()
	{
		BattleID = 0;
		BattleTeam = 0;

		memset(SingleSpecialState, 0, sizeof(bool) * eSingleSpecialState_Max);
	}
	DWORD dwObjectID;
	DWORD dwUserID;
	char ObjectName[MAX_NAME_LENGTH+1];
	DWORD BattleID;
	BYTE BattleTeam;
	BYTE ObjectState;

	
	////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ����/����
	bool	SingleSpecialState[eSingleSpecialState_Max];	// ���� ��ȣ
	////////////////////////////
};

struct CHARACTER_TOTALINFO
{
	DWORD	Life;							// ������			//?
	DWORD	MaxLife;						// �ִ� ������		//?
	
	DWORD	Shield;							// ȣ�Ű���			//?
	DWORD	MaxShield;						// �ִ� ȣ�Ű���	//?

	BYTE	Gender;							// ����
	BYTE	FaceType;						// �󱼹�ȣ
	BYTE	HairType;						// �Ӹ���� ��ȣ

	WORD	WearedItemIdx[eWearedItem_Max];	// ������ �����Ƶ��� ����

	BYTE	Stage;							// ����
	LEVELTYPE	Level;						// ����				//?
	MAPTYPE	CurMapNum;						// �����			//?
	MAPTYPE	LoginMapNum;					// �α��θ�			//?

	bool	bPeace;							// ��/��ȭ��� FALSE �� ���� TRUE �� ��ȭ	//?
	WORD	MapChangePoint_Index;			// ����ȯ�� ���	//?
	WORD	LoginPoint_Index;				// �α��ν� ���	//?

	DWORD	MunpaID;						// ���� �ε���
	BYTE	PositionInMunpa;				// ���� ���� 
	MARKNAMETYPE MarkName;					// guild mark name	
	bool	bVisible;						// ī�޶� ��带 ���� ����
	bool	bPKMode;						//PK��� ����
	BOOL	bMussangMode;					//���ָ�� ����
	
	FAMETYPE	BadFame;					//�Ǹ�(PK��ġ)

	float	Height;							// Ű
	float	Width;							// ü��

	char	NickName[MAX_GUILD_NICKNAME+1]; // ��忡���� ȣĪ
	char	GuildName[MAX_GUILD_NAME+1];	// ���� �̸�
	
	DWORD	dwGuildUnionIdx;					// ����
	char	sGuildUnionName[MAX_GUILD_NAME+1];	// �����̸�
	DWORD	dwGuildUnionMarkIdx;				// ���͸�ũ
	
	bool	bRestraint;						// ����
	BYTE	EventIndex;						// �߼� �̺�Ʈ �ε���
	bool	bNoAvatarView;

#ifdef _JAPAN_LOCAL_
	int		nMainAttr;				// ĳ���� �ּӼ�
	int		nSubAttr;				// ĳ���� �μӼ�
//	DWORD	dwCharacterAttr;		// ĳ���� �Ӽ�&�μӼ�( LOWORD = �ּӼ�, HIWROD = �μӼ� )
	char	ExtraInvenSlot;			// �߰������� �κ��丮 ����
	char	ExtraPyogukSlot;		// �߰������� ����â�� ����
	char	ExtraMugongSlot;		// �߰������� ����â ����
#endif

#ifdef _HK_LOCAL_
	char	ExtraInvenSlot;			// �߰������� �κ��丮 ����
	char	ExtraPyogukSlot;		// �߰������� ����â�� ����
	char	ExtraMugongSlot;		// �߰������� ����â ����
#endif

#ifdef _TL_LOCAL_
	char	ExtraInvenSlot;			// �߰������� �κ��丮 ����
	char	ExtraPyogukSlot;		// �߰������� ����â�� ����
	char	ExtraMugongSlot;		// �߰������� ����â ����
#endif

};



struct SHOPITEMOPTION
{
	WORD		Avatar[eAvatar_Max];

	WORD		Gengol;				// �ٰ�ġ
	WORD		Minchub;			// ��øġ
	WORD		Cheryuk;			// ü��ġ
	WORD		Simmek;				// �ɸ�ġ

	WORD		Life;				// �ִ�ü������ġ
	WORD		Shield;				// �ִ�ȣ�Ű�������ġ
	WORD		Naeryuk;			// �ִ볻������ġ

	WORD		AddExp;				// �߰� ����ġ			100% => 100
	WORD		AddItemDrop;		// �߰� �����۵����	100% => 100
	char		ExpPeneltyPoint;	// ���� ����ġ �г�Ƽ����Ʈ
	char		MoneyPeneltyPoint;	// ���� �� �г�Ƽ����Ʈ

	WORD		AddSung;			// Ưġ��
	char		NeagongDamage;		// ����������		100% => 100
	char		WoigongDamage;		// �ܰ�������		100% => 100
	char		ComboDamage;		// �⺻�޺� ������  100% => 100

	char		RecoverRate;		// ����ȸ����		100% => 100

	WORD		Critical;			// ũ��Ƽ��
	char		StunByCri;			// ũ��Ƽ�ý� ���ϰɸ� Ȯ��  100% => 100
	WORD		Decisive;			// ���� ����(�ϰ�)

	char		ItemMixSuccess;		// ������ ���� ����Ȯ��		100% => 100
	WORD		StatePoint;			// �� �� �ִ� ��������Ʈ ��
	WORD		UseStatePoint;		// �̹� ���� ���� �� �ִ� ��������Ʈ ��
	
	char		RegistPhys;			// �������� ����ġ	100% => 100
	char		RegistAttr;			// �Ӽ����� ����ġ	100% => 100
	char		NeaRyukSpend;		// ���¼Ҹ� ����ġ		100% => 100 ����

	DWORD		SkillPoint;			// �� �� �ִ� ����Ʈ
	DWORD		UseSkillPoint;		// �̹� ���� �� �� �ִ� ����Ʈ
	
	char		ProtectCount;		// ����ġ+����ȣ�ֹ����� ���� Ƚ��.
	
	WORD		AddAbility;			// Ư��ġ ����ġ		100% => 100
	WORD		AddMugongExp;		// ��������ġ ����ġ	100% => 100

	char		PlustimeExp;		// �÷���Ÿ�� ����ġ
	char		PlustimeAbil;		// �÷���Ÿ�� Ư��ġ
	char		PlustimeNaeruyk;	// �÷���Ÿ�� ���¼Ҹ�
	
	BYTE		bKyungGong;			// ���
	BYTE		KyungGongSpeed;		// ����ӵ�
	
	BYTE		EquipLevelFree;		// ��� �������� ����
	BYTE		ReinforceAmp;		// ��ȭ���� ��ġ		100% => 100

	BYTE		bStreetStall;		// �������� ��밡������ ����

	WORD		wSkinItem[eSkinItem_Max];	// ��Ų ������ ����

//	WORD		BuyStallInven;		// ���Գ��� �߰� �κ� ��

//	char		ReviveLife;			// ��Ȱ�ֹ��� ���� Life ȭ������		100% => 100
//	char		ReviveExp;			// ��Ȱ�ֹ��� ���� ����ġ ȸ������	100% => 100
//	char		ReviveShield;		// ��Ȱ�ֹ��� ���� ȣ�Ű��� ȸ������	100% => 100
//	char		ReviveNaeryuk;		// ��Ȱ�ֹ��� ���� ���� ȸ������		100% => 100	
	DWORD		dwStreetStallDecoration;	// ������ �ٹ̱�(�ش� �������� �ε����� ����)
};

struct AVATARITEMOPTION
{
	WORD		Life;				// �ִ������
	WORD		Shield;				// �ִ�ȣ�Ű���
	WORD		Naeruyk;			// �ִ볻��

	BYTE		Attack;				// ���ݷ�
	BYTE		Critical;			// ũ��Ƽ��
	BYTE		Decisive;			// ���� �ϰ�(����)

	BYTE		Gengol;				// �ٰ�
	BYTE		Minchub;			// ��ø
	BYTE		Cheryuk;			// ü��
	BYTE		Simmek;				// �ɸ�

	WORD		CounterPercent;		// ī���� Ȯ��
	WORD		CounterDamage;		// ī���ͽ� ��������

	BYTE		bKyungGong;			// ���
	
	BYTE		NeaRyukSpend;		// ���¼Ҹ𰨼�ġ
	
	WORD		NeagongDamage;		// ����������		100% => 100
	WORD		WoigongDamage;		// �ܰ�������		100% => 100

	WORD		TargetPhyDefDown;	// ���� �������� ����
	WORD		TargetAttrDefDown;	// ���� �Ӽ����׷� ����
	WORD		TargetAtkDown;		// ������ ���ݷ� ����

	WORD		RecoverRate;		// ȸ����
	WORD		KyunggongSpeed;		// ����ӵ�

	WORD		MussangCharge;		// ��ȥ������ �����ӵ�
	BYTE		NaeruykspendbyKG;	// ������ ���¼Ҹ� ����
    WORD		ShieldRecoverRate;	// ����ȸ���� �߰��� ȣ�Ű��� ȸ����
	BYTE		MussangDamage;		// ��ȥ�ߵ��� ������������	100% -> 100
};

struct SEND_MONSTER_TOTALINFO	:	public MSGBASE
{
	BASEOBJECT_INFO BaseObjectInfo;
	MONSTER_TOTALINFO TotalInfo;
	SEND_MOVEINFO MoveInfo;

	BYTE bLogin;
	
	CAddableInfoList AddableInfo;

	WORD GetMsgLength()	{	return sizeof(SEND_MONSTER_TOTALINFO) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();	}
};

struct SEND_NPC_TOTALINFO	:	public MSGBASE
{
	BASEOBJECT_INFO BaseObjectInfo;
	NPC_TOTALINFO TotalInfo;
	SEND_MOVEINFO MoveInfo;
	
	float Angle;
	BYTE bLogin;

	CAddableInfoList AddableInfo;

	WORD GetMsgLength()	{	return sizeof(SEND_NPC_TOTALINFO) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();	}
};

struct SEND_PET_TOTALINFO	:	public MSGBASE
{
	BASEOBJECT_INFO	BaseObjectInfo;
	PET_TOTALINFO	TotalInfo;
	SEND_MOVEINFO	MoveInfo;
	char MasterName[MAX_NAME_LENGTH+1];
	DWORD			MasterID;

	BYTE	bLogin;

	CAddableInfoList	AddableInfo;

	WORD	GetMsgLength() {	return sizeof(SEND_PET_TOTALINFO) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();	}
};

struct SEND_CHARACTER_TOTALINFO : public MSGBASE
{
	BASEOBJECT_INFO BaseObjectInfo;
	CHARACTER_TOTALINFO TotalInfo;	
	SEND_MOVEINFO MoveInfo;
	SHOPITEMOPTION	ShopItemOption;
	
	BOOL	bInTitan;
	BYTE bLogin;
	
	CAddableInfoList AddableInfo;

	WORD GetMsgLength()	{	return sizeof(SEND_CHARACTER_TOTALINFO) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();	}
};

struct ABILITY_TOTALINFO
{
	char AbilityDataArray[eAGK_Max][MAX_ABILITY_NUM_PER_GROUP+1];
	char AbilityQuickPositionArray[eAGK_Max][MAX_ABILITY_NUM_PER_GROUP+1];
};

struct SEND_HERO_TOTALINFO : public MSGBASE
{
	BASEOBJECT_INFO BaseObjectInfo;
	CHARACTER_TOTALINFO ChrTotalInfo;
	HERO_TOTALINFO HeroTotalInfo;
	SEND_MOVEINFO SendMoveInfo;
	DWORD UniqueIDinAgent;

	SHOPITEMOPTION	ShopItemOption;
	MUGONG_TOTALINFO MugongTotalInfo;
	ABILITY_TOTALINFO AbilityInfo;
	ITEM_TOTALINFO ItemTotalInfo;

	WORD	OptionNum;
	WORD	RareOptionNum;
	WORD	PetNum;
	WORD	TitanNum;
	WORD	TitanEndrncNum;

	SYSTEMTIME		ServerTime;			//ĳ���� ���� �α��� �ð�

//	ITEM_OPTION_INFO OptionInfo[MAX_ITEM_OPTION_NUM];
//	ITEM_RARE_OPTION_INFO RareOptionInfo[MAX_ITEM_OPTION_NUM];
	//SW051004 ����
	CAddableInfoList AddableInfo;

	WORD GetMsgLength()
	{
		return sizeof( SEND_HERO_TOTALINFO ) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();
	}

	/*SW051004
	void InitOptionInfo()
	{
		OptionNum = 0;
	}
	void AddOptionInfo(ITEM_OPTION_INFO * pInfo)
	{
		memcpy(&OptionInfo[OptionNum], pInfo, sizeof(ITEM_OPTION_INFO));
		++OptionNum;
	}
	void InitRareOptionInfo()
	{
		RareOptionNum = 0;
	}
	void AddRareOptionInfo(ITEM_RARE_OPTION_INFO* pRareInfo)
	{
		memcpy(&RareOptionInfo[RareOptionNum], pRareInfo, sizeof(ITEM_RARE_OPTION_INFO));
		++RareOptionNum;
	}*/
};

struct SEND_CHARSELECT_INFO : public MSGBASE
{
#ifdef _CRYPTCHECK_ 
	HselInit eninit;
	HselInit deinit;
#endif
	int CharNum;
	WORD StandingArrayNum[MAX_CHARACTER_NUM];				// pjs �ɸ����� ��ġ ���� [5/22/2003]
	BASEOBJECT_INFO BaseObjectInfo[MAX_CHARACTER_NUM];
	CHARACTER_TOTALINFO ChrTotalInfo[MAX_CHARACTER_NUM];
};

struct SEND_ADDPET_FROMITEM : public MSGBASE
{
	PET_TOTALINFO Info;
};

//SW070127 Ÿ��ź
struct SEND_ADDTITAN_FROMITEM : public MSGBASE
{
	TITAN_TOTALINFO Info;
};

struct SEND_ADDTITANEQUIP_FROMITEM : public MSGBASE
{
	TITAN_ENDURANCE_ITEMINFO Info;
};

struct SEND_TITANSTATINFO : public MSGBASE
{
	titan_calc_stats Info;
};
/*
struct SEND_CHARMAKE_INFO : public MSGBASE
{
	DWORD	UserID;
	char	Name[MAX_NAME_LENGTH+1];
	BYTE	Job;
	BYTE	FaceType;
	BYTE	HeadType;
	MAPTYPE	Map;
	BYTE	Gender;
	
	char Name[MAX_NAME_LENGTH+1];			// ĳ���� �̸�
	DWORD	UserID;
	BYTE	JobType;
	BYTE	SexType;
	BYTE	BodyType;
	BYTE	HairType;
	BYTE	FaceType;
	BYTE	StartArea;
	BOOL	bDuplCheck;
};
*/

struct MSG_NAME : public MSGBASE
{
	char Name[MAX_NAME_LENGTH+1];
};

struct MSG_NAME2 : public MSGBASE
{
	char str1[MAX_NAME_LENGTH+1];
	char str2[MAX_NAME_LENGTH+1];
};

struct MSG_NAME_DWORD : public MSGBASE
{
	char Name[MAX_NAME_LENGTH+1];
	DWORD dwData;
};

struct MSG_NAME_WORD : public MSGBASE
{
	char Name[MAX_NAME_LENGTH+1];
	WORD wData;
};

struct MSG_NAME_DWORD2 : public MSGBASE
{
	char Name[MAX_NAME_LENGTH+1];
	DWORD dwData1;
	DWORD dwData2;
};

//ÆÑ µ¥?ÌÅÍ °ú·Ã ////////////////////////////////////////////////
struct MSG_NAME_DWORD3 : public MSGBASE
{
	char	Name[MAX_NAME_LENGTH+1];
	DWORD	dwData1;
	DWORD	dwData2;
	DWORD	dwData3;
};

struct MSG_NAME2_DWORD : public MSGBASE
{
	char	Name1[MAX_NAME_LENGTH+1];
	char	Name2[MAX_NAME_LENGTH+1];
	DWORD	dwData;
};

//�� ������ ���� ////////////////////////////////////////////////
struct SEND_PACKED_DATA : public MSGBASE
{
	WORD wRealDataSize;
	WORD wReceiverNum;
	char Data[MAX_PACKEDDATA_SIZE];
};
struct SEND_PACKED_TOMAPSERVER_DATA : public MSGBASE
{
	MAPTYPE ToMapNum;
	WORD wRealDataSize;
	char Data[MAX_PACKEDDATA_SIZE];
	WORD GetMsgLength()
	{
		return sizeof(SEND_PACKED_TOMAPSERVER_DATA) - MAX_PACKEDDATA_SIZE + wRealDataSize;
	}
};
// ������ ���� ��Ʈ�p �޽��� ////////////////////////////////////

struct MSG_ITEM : public MSGBASE
{
	ITEMBASE ItemInfo;
};

struct MSG_ITEM_WITH_OPTION : public MSGBASE
{
	BOOL IsOptionItem;
	BOOL IsRareOptionItem;
	ITEMBASE ItemInfo;
	ITEM_OPTION_INFO OptionInfo;
	ITEM_RARE_OPTION_INFO RareOptionInfo;

	MSG_ITEM_WITH_OPTION() : IsOptionItem(0), IsRareOptionItem(0) {}
	WORD GetMsgLength()
	{
		if(!IsRareOptionItem)
		{
			return sizeof(MSG_ITEM_WITH_OPTION) - sizeof(ITEM_OPTION_INFO)*(!IsOptionItem);
		}
		else
		{
			return sizeof(MSG_ITEM_WITH_OPTION);
		}
	}
};


// RaMa �����ʸ�Ʈ �̺�Ʈ 
struct MSG_GT_EVENTSTART : public MSGBASE
{
	char GuildName1[MAX_GUILD_NAME+1];
	char GuildName2[MAX_GUILD_NAME+1];
	DWORD SenderID;

	MSG_GT_EVENTSTART()
	{
		memset( GuildName1, 0, sizeof(char)*(MAX_GUILD_NAME+1) );
		memset( GuildName2, 0, sizeof(char)*(MAX_GUILD_NAME+1) );
		SenderID = 0;
	}
};

//���� �۾�
struct ITEMOBTAINARRAY : public MSGBASE
{
	ITEMOBTAINARRAY()
	{
		ItemNum = 0;
	}
	void Init( BYTE cg, BYTE pt, DWORD dwID )
	{
		Category	= cg;
		Protocol	= pt;
		dwObjectID	= dwID;
	}
	void AddItem( DWORD DBIdx, WORD ItemIdx, DURTYPE Durability, POSTYPE bPosition, POSTYPE QuickPosition, ITEMPARAM Param, DWORD RareIdx = 0 )
	{
		ItemInfo[ItemNum].dwDBIdx		= DBIdx;
		ItemInfo[ItemNum].wIconIdx		= ItemIdx;
		ItemInfo[ItemNum].Durability	= Durability;
		ItemInfo[ItemNum].Position		= bPosition;
		ItemInfo[ItemNum].QuickPosition	= QuickPosition;
		ItemInfo[ItemNum].ItemParam		= Param;
		//SW050920 RareItem
		ItemInfo[ItemNum].RareIdx		= RareIdx;
		ItemNum++;
	}
	void AddItem( WORD idx, ITEMBASE * item)
	{
		memcpy( &ItemInfo[idx], item, sizeof( ITEMBASE ) );
		ItemNum++;
	}
	ITEMBASE * GetItem( WORD idx ) { return &ItemInfo[idx]; }
	int GetSize()
	{
		return ( sizeof( ITEMOBTAINARRAY ) - ( sizeof( ITEMBASE ) * ( SLOT_INVENTORY_NUM - ItemNum ) ) );
	}
	void Clear()
	{
		memset( this, 0, sizeof( ITEMOBTAINARRAY ) );
		ItemNum = 0;
	}
	BYTE		ItemNum;
	WORD		wObtainCount;
	ITEMBASE	ItemInfo[SLOT_INVENTORY_NUM];
};

typedef void (*DBResult)(void * pPlayer, WORD ArrayID);
typedef void (*DBResultEx)(void* pPlayer, WORD ArrayID, void* pRareOptionInfo);
struct ITEMOBTAINARRAYINFO
{
	DWORD			wObtainArrayID;
	DBResult		CallBack;
	DBResultEx		CallBackEx;
	DWORD			dwFurnisherIdx;
	WORD			wType;

	ITEMOBTAINARRAY	ItemArray;
};

struct MSG_ITEM_ERROR : public MSGBASE
{
	int ECode;
};

struct MSG_ITEM_MOVE_SYN : public MSGBASE
{
	POSTYPE FromPos;
	WORD wFromItemIdx;
	POSTYPE ToPos;
	WORD wToItemIdx;
};

struct MSG_ITEM_MOVE_ACK : public MSG_ITEM_MOVE_SYN
{
};
struct MSG_ITEM_COMBINE_SYN : public MSGBASE
{
	WORD wItemIdx;
	POSTYPE FromPos;
	POSTYPE ToPos;
	DURTYPE FromDur;
	DURTYPE ToDur;
};
struct MSG_ITEM_COMBINE_ACK : public MSG_ITEM_COMBINE_SYN
{
};
struct MSG_ITEM_DIVIDE_SYN : public MSGBASE
{
	WORD wItemIdx;
	POSTYPE FromPos;
	POSTYPE ToPos;
	DURTYPE FromDur;
	DURTYPE ToDur;
};

struct MSG_ITEM_DISCARD_SYN : public MSGBASE
{
	POSTYPE TargetPos;
	WORD wItemIdx;
	DURTYPE ItemNum;
};
struct MSG_ITEM_DISCARD_ACK : public MSG_ITEM_DISCARD_SYN
{
};

//SW070626 ���λ�NPC �����Ͽ� ����
/*
struct MSG_ITEM_BUY_SYN : public MSGBASE
{
	WORD wBuyItemIdx;
	WORD BuyItemNum;
	WORD wDealerIdx;
};*/
struct DEMAND_ITEM	// ���� ���Ž� �� �ܿ� �Һ�Ǵ� ������
{
	DEMAND_ITEM() : demandItemIdx(0), demandItemPos(0), demandItemDur(0) {}
	WORD demandItemIdx;
	POSTYPE demandItemPos;
	DURTYPE demandItemDur;
};
struct MSG_ITEM_BUY_SYN : public MSGBASE
{
	MSG_ITEM_BUY_SYN() : slotNum(0) {}
	WORD wBuyItemIdx;
	WORD BuyItemNum;
	WORD wDealerIdx;

	WORD slotNum;
	DEMAND_ITEM demandItem[SLOT_INVENTORY_NUM];

	void AddDemandSlot(WORD itemIdx, POSTYPE itemPos, DURTYPE itemDur)
	{
		demandItem[slotNum].demandItemIdx = itemIdx;
		demandItem[slotNum].demandItemPos = itemPos;
		demandItem[slotNum].demandItemDur = itemDur;
		++slotNum;
	}

	int GetSize()
	{
		int length = sizeof(MSG_ITEM_BUY_SYN) - (sizeof(DEMAND_ITEM) * (SLOT_INVENTORY_NUM - slotNum));
		return length;
	}
};

struct MSG_ITEM_SELL_SYN : public MSGBASE
{
	POSTYPE TargetPos;
	WORD	wSellItemIdx;
	WORD	SellItemNum;
	WORD	wDealerIdx;
};
struct MSG_ITEM_SELL_ACK : public MSG_ITEM_SELL_SYN
{
};
struct MSG_ITEM_UPGRADE_SYN : public MSGBASE
{
	POSTYPE ItemPos;
	WORD	wItemIdx;

	POSTYPE MaterialItemPos;
	WORD	wMaterialItemIdx;
};
struct MSG_ITEM_UPGRADE_ACK : public MSG_ITEM_UPGRADE_SYN
{
};

struct MATERIAL_ARRAY
{
	WORD	wItemIdx;
	POSTYPE ItemPos;
	DURTYPE Dur;
};
struct MSG_ITEM_MIX_SYN : public MSGBASE
{
	WORD	wBasicItemIdx;
	POSTYPE BasicItemPos;
	WORD	wResultItemIdx;
	DWORD	dwRareIdx;
	POSTYPE ResultIndex;
	WORD	ShopItemIdx;
	WORD	ShopItemPos;
	WORD	wMaterialNum;
	MATERIAL_ARRAY Material[MAX_MIX_MATERIAL];
	void Init()
	{
		wMaterialNum = 0;
		ShopItemIdx = 0;
		ShopItemPos = 0;
		dwRareIdx	= 0;
	}
	void AddMaterial(WORD wItemIdx, POSTYPE ItemPos, DURTYPE Dur)
	{
		Material[wMaterialNum].wItemIdx = wItemIdx;
		Material[wMaterialNum].ItemPos = ItemPos;
		Material[wMaterialNum++].Dur = Dur;
	}
	int GetSize()
	{
		return sizeof(MSG_ITEM_MIX_SYN) - (MAX_MIX_MATERIAL - wMaterialNum)*sizeof(MATERIAL_ARRAY);
	}
};
struct MSG_ITEM_MIX_ACK : public MSG_ITEM_MIX_SYN
{
};
struct MSG_ITEM_MIX_RARE_ACK : public MSG_ITEM_MIX_SYN
{
	ITEM_RARE_OPTION_INFO RareOptionInfo;
};

struct MSG_ITEM_USE_SYN : public MSGBASE
{
	POSTYPE TargetPos;
	WORD	wItemIdx;
};
struct MSG_ITEM_USE_ACK : public MSG_ITEM_USE_SYN
{
};
/// ������ �ɼ� ���� //////////////////////////////////
struct ITEM_JEWEL_POS
{
	WORD	wItemIdx;
	POSTYPE pos;
};
struct ITEM_JEWEL_POS_EX
{
	WORD	wItemIdx;
	POSTYPE pos;
	DURTYPE Dur;
};
struct MSG_ITEM_DISSOLVE_SYN : public MSGBASE
{
	WORD	wTargetItemIdx;
	POSTYPE TargetPos;
	DURTYPE	wOptionIdx;
};
struct MSG_ITEM_REINFORCE_SYN : public MSGBASE
{
	void Init( BYTE cg, BYTE pt, DWORD dwID )
	{
		Category	= cg;
		Protocol	= pt;
		dwObjectID	= dwID;
		wJewelUnit = 0;
	}
	void AddJewelInfo(WORD JewelItemIdx, POSTYPE Pos, DURTYPE Dur )
	{
		JewelWhich[wJewelUnit].wItemIdx = JewelItemIdx;
		JewelWhich[wJewelUnit].pos = Pos;
		JewelWhich[wJewelUnit].Dur = Dur;
		++wJewelUnit;
	}
	int GetSize()
	{
		return sizeof(MSG_ITEM_REINFORCE_SYN) - (MAX_ITEM_OPTION_NUM - wJewelUnit)*sizeof(ITEM_JEWEL_POS_EX);
	}
	WORD	wTargetItemIdx;
	POSTYPE TargetPos;
	WORD	wJewelUnit;
	ITEM_JEWEL_POS_EX	JewelWhich[MAX_ITEM_OPTION_NUM];
};
struct MSG_ITEM_REINFORCE_WITHSHOPITEM_SYN : public MSGBASE
{
	void Init( BYTE cg, BYTE pt, DWORD dwID )
	{
		Category	= cg;
		Protocol	= pt;
		dwObjectID	= dwID;
		wJewelUnit = 0;
	}
	void AddJewelInfo(WORD JewelItemIdx, POSTYPE Pos, DURTYPE Dur )
	{
		JewelWhich[wJewelUnit].wItemIdx = JewelItemIdx;
		JewelWhich[wJewelUnit].pos = Pos;
		JewelWhich[wJewelUnit].Dur = Dur;
		++wJewelUnit;
	}
	int GetSize()
	{
		return sizeof(MSG_ITEM_REINFORCE_WITHSHOPITEM_SYN) - (MAX_ITEM_OPTION_NUM - wJewelUnit)*sizeof(ITEM_JEWEL_POS_EX);
	}
	WORD	wTargetItemIdx;
	POSTYPE TargetPos;
	WORD	wShopItemIdx;
	POSTYPE ShopItemPos;
	WORD	wJewelUnit;
	ITEM_JEWEL_POS_EX	JewelWhich[MAX_ITEM_OPTION_NUM];
};
struct MSG_ITEM_REINFORCE_ACK : public MSGBASE
{
	WORD	wTargetItemIdx;
	POSTYPE TargetPos;
	ITEM_OPTION_INFO OptionInfo;
};

struct MSG_ITEM_RAREITEM_GET : public MSGBASE
{
	WORD		wTargetItemIdx;
	ITEMBASE	RareItemBase;
	ITEM_RARE_OPTION_INFO RareInfo;
};
///////////////////////////////////////////////////////
// ����â
struct MSG_QUICK_SET_SYN : public MSGBASE
{
	POSTYPE QuickPos;
	POSTYPE SrcPos;
	WORD	SrcItemIdx;
};
struct MSG_QUICK_SET_ACK : public MSG_QUICK_SET_SYN
{
};

struct MSG_QUICK_ADD_SYN : public MSGBASE
{
	POSTYPE QuickPos;
	POSTYPE SrcPos;
	WORD	SrcItemIdx;
	POSTYPE OldSrcPos;
	WORD	OldSrcItemIdx;
	
	// 06. 01 �̿��� - ����â ����
	POSTYPE SrcQuickPos;
	POSTYPE OldSrcQuickPos;
};
struct MSG_QUICK_ADD_ACK : public MSG_QUICK_ADD_SYN
{
};
struct MSG_QUICK_REM_SYN : public MSGBASE
{
	POSTYPE SrcPos;
	POSTYPE SrcItemIdx;

	// 06. 01 �̿��� - ����â ����
	POSTYPE QuickPos;
	POSTYPE SrcQuickPos;
};
struct MSG_QUICK_REM_ACK : public MSG_QUICK_REM_SYN
{
};
struct MSG_QUICK_MOVE_SYN : public MSGBASE
{
	POSTYPE FromSrcPos;
	WORD wFromSrcItemIdx;
	POSTYPE FromQuickPos;
	POSTYPE ToSrcPos;
	WORD wToSrcItemIdx;
	POSTYPE ToQuickPos;

	// 06. 01 �̿��� - ����â ����
	POSTYPE FromSrcQuickPos;
	POSTYPE ToSrcQuickPos;
	bool	DeleteToSrcQuickPos;
	
};
struct MSG_QUICK_MOVE_ACK : public MSG_QUICK_MOVE_SYN
{
};
struct MSG_QUICK_ERROR : public MSGBASE
{
	int ECode;
};

//////////// ���� ���� ��Ʈ�p �޽��� /////////////////////////////
//-------------- ����(��ų) �̵� (in ����â) ---------------------

struct MSG_MUGONG_MOVE_SYN : public MSGBASE
{
	POSTYPE FromPos;
	POSTYPE ToPos;
};

struct MSG_MUGONG_MOVE_ACK : public MSG_MUGONG_MOVE_SYN
{
};

struct MSG_MUGONG_REM_SYN : public MSGBASE
{
	WORD	wMugongIdx;
	POSTYPE TargetPos;
};

struct MSG_MUGONG_REM_ACK : public MSG_MUGONG_REM_SYN
{
};

struct MSG_MUGONG_ADD_ACK : public MSGBASE
{
	MUGONGBASE MugongBase;
};

//KES
struct MSG_MUGONG_DESTROY : public MSGBASE
{
	WORD		wMugongIdx;
	POSTYPE		TargetPos;
	BYTE		cbReason;	//�����ı� ����
};




/*

struct MSG_MUGONG_ADD_NACK : public MSGBASE
{
};
struct MSG_MUGONG_MOVEINFO : public MSGBASE
{
	DWORD		dwMugongDBIdx;
	POSTYPE		wToPosition;
	POSTYPE		wFromPosition;
};
struct MSG_MUGONG_DELINFO : public MSGBASE
{
	DWORD		dwMugongDBIdx;
	POSTYPE		wPosition;
};


struct MSG_MUGONG_ADDINVDEL_SYN : public MSGBASE
{
	BYTE	bMugongType;				// 0,1,2
	DWORD	dwItemDBIdx;				// ������ db index
	POSTYPE	wItemPosition;				// ������ pos
};
struct MSG_MUGONG_ADDINVDEL_ACK : public MSGBASE
{
	BYTE		bMugongType;				// 0,1,2
	POSTYPE		wItemPosition;				// ������ pos
	MUGONGBASE	mugong;
};
struct MSG_MUGONG_DELINVADD_SYN : public MSGBASE
{
	BYTE	bMugongType;				// 0,1,2
	WORD 	wMugongIdx;					
	//WORD	wItemIdx;
	POSTYPE	wItemInvPosition;
};
struct MSG_MUGONG_DELINVADD_ACK : public MSGBASE
{
	BYTE		bMugongType;				// 0,1,2
	ITEMBASE	itemBase;			
};
struct MSG_MUGONG_DELGRDADD_SYN : public MSGBASE
{
	BYTE		bMugongType;				// 0,1,2
	WORD 		wMugongIdx;					
	//WORD		wItemIdx;
	VECTOR3		vItemGroundPos;
};
struct MSG_MUGONG_DELGRDADD_ACK : public MSGBASE
{
	BYTE		bMugongType;				// 0,1,2
};*/

//////////////////////////////////////////////////////////////////
///////// ����â ���� ��Ʈ�� �޽��� /////////////////////////////////
/*
struct MSG_QUICK_ADD_SYN :  public MSGBASE
{
	WORD	wIconIdx;
	POSTYPE	wQuickAbsPosition;
	POSTYPE	wSrcAbsPosition;
};
struct MSG_QUICK_ADD_ACK : public MSGBASE
{
	WORD	wIconIdx;
	POSTYPE	wSrcAbsPosition;
	POSTYPE	wQuickAbsPosition;
};
struct MSG_QUICK_REM_SYN : public MSG_ITEM_REM_SYN
{
};
struct MSG_QUICK_REM_ACK : public MSGBASE
{
	POSTYPE	wQuickAbsPosition;
	POSTYPE	wSrcAbsPosition;
};

struct MSG_QUICK_USE_SYN :  public MSGBASE
{
	POSTYPE	wQuickRelPosition;
	POSTYPE	wSrcPosition;
};
struct MSG_QUICK_MOVE_SYN : public MSGBASE
{
	POSTYPE		wToAbsPosition;
	POSTYPE		wFromAbsPosition;
};
struct MSG_QUICK_MOVE_ACK : public MSGBASE
{
	POSTYPE		wToAbsPosition;
	POSTYPE		wFromAbsPosition;
};
struct MSG_QUICK_UPD_ACK : public MSGBASE
{
	POSTYPE		wAbsPosition;
	POSTYPE		wSrcPosition;
};
*/


//KES EXCHANGE 030923
////////////////////
//��ȯâ ������ ����
struct MSG_LINKITEM : public MSG_ITEM
{
	POSTYPE		wAbsPosition;
};

//SW050920 Rare
struct MSG_LINKITEM_ADD : public MSG_LINKITEM	//��ȯ�� �ɼ������� �Ѱ��ٶ� ����.
{
	BYTE				bPetInfo;
	BYTE				bOptionItem;
	BYTE				bRareOptionItem;
	PET_TOTALINFO		sPetTotalInfo;
	ITEM_OPTION_INFO	sOptionInfo;
	ITEM_RARE_OPTION_INFO sRareOptionInfo;

	MSG_LINKITEM_ADD() : bPetInfo(0), bOptionItem(0), bRareOptionItem(0) {}
	int GetSize()
	{
		if(!bRareOptionItem)
		{
			return sizeof(MSG_LINKITEM_ADD) - sizeof(ITEM_OPTION_INFO)*(1-(bOptionItem));
		}
		else
		{
			return sizeof(MSG_LINKITEM_ADD);
		}
	}
};

struct MSG_LINKITEMEX : public MSG_LINKITEM
{
	MONEYTYPE	dwMoney;
	WORD		wVolume;
};

//SW070308 �����ۺΰ��ɼǵ� ����
struct MSG_LINKITEMOPTIONS : public MSG_LINKITEMEX
{
	MSG_LINKITEMOPTIONS() : eOptionKind(0) {}
	int					eOptionKind;

	CAddableInfoList	AddableInfo;

	WORD	GetSize()
	{
		return sizeof( MSG_LINKITEMOPTIONS ) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();
	}
};

struct MSG_LINKBUYITEMEX : public MSG_LINKITEMEX
{
	WORD wVolume;
};

struct MSG_ITEMEX : public MSG_ITEM
{
	BYTE count;
	BYTE RareCount;
	BYTE PetInfoCount;
	PET_TOTALINFO		sPetInfo;
	ITEM_OPTION_INFO	sOptionInfo;
	ITEM_RARE_OPTION_INFO sRareOptionInfo;

	MSG_ITEMEX() {memset(this, 0, sizeof(MSG_ITEMEX));}

	int GetSize()
	{
		if(!RareCount)
		{
			return sizeof(MSG_ITEMEX) - sizeof(ITEM_OPTION_INFO)*(1-(count));
		}
		else
		{
			return sizeof(MSG_ITEMEX);
		}
	}
};

/*
struct MSG_BUYITEM : public MSG_ITEMEX
{
	DWORD dwData;
};*/
//SW070308 �����ۺΰ��ɼǵ� ����
struct MSG_BUYITEM : MSG_LINKITEMOPTIONS
{};


struct MSG_LOOTINGIEM : public MSG_LINKITEM
{
	DWORD				dwDiePlayerIdx;
	BYTE				bOptionItem;
	BYTE				bRareOptionItem;
	BYTE				bPetSummonItem;
	PET_TOTALINFO		sPetInfo;
	ITEM_OPTION_INFO	sOptionInfo;
	ITEM_RARE_OPTION_INFO	sRareOptionInfo;
	
	MSG_LOOTINGIEM() : bOptionItem(0), bRareOptionItem(0), bPetSummonItem(0) {}
	int GetSize()
	{
		if(!bRareOptionItem)
		{
			return sizeof(MSG_LOOTINGIEM) - sizeof(ITEM_OPTION_INFO)*(!bOptionItem);
		}
		else
		{
			return sizeof(MSG_LOOTINGIEM);
		}
	}
};
//SW070509 MSG_LOOTINGIEM ������ :p ö������
struct MSG_LOOTINGITEM : MSG_LINKITEMOPTIONS
{};

/*
struct SEND_LINKITEM_TOTALINFO : public MSG_LINKITEMEX
{
	BYTE count;
	BYTE RareCount;
	BYTE PetInfoCount;
	PET_TOTALINFO sPetTotalInfo;
	ITEM_OPTION_INFO sOptionInfo;
	ITEM_RARE_OPTION_INFO sRareOptionInfo;

	int GetSize()
	{
		if(!RareCount)
		{
			return sizeof(SEND_LINKITEM_TOTALINFO) - sizeof(ITEM_OPTION_INFO)*(!count);
		}
		else
		{
			return sizeof(SEND_LINKITEM_TOTALINFO);
		}
	}
};*/
//SW070308 �����ۺΰ��ɼǵ� ����
struct SEND_LINKITEM_TOTALINFO : public MSG_LINKITEMOPTIONS
{};


struct SEND_LINKBUYITEM_TOTALINFO : public SEND_LINKITEM_TOTALINFO
{
	//WORD wVolume;	// move to MSG_LINKITEMEX	//SW070308 �����ۺΰ��ɼǵ� ����
};

struct MSG_EXCHANGE_REMOVEITEM : public MSGBASE
{
	POSTYPE		wAbsPosition;
};

struct MSG_ITEM_DESTROY : public MSGBASE
{
	POSTYPE		wAbsPosition;
	BYTE		cbReason;
};


//////////////////////////////////////

struct REGIST_MAP : public MSGBASE
{
	WORD mapServerPort;
	MAPTYPE mapnum;
};


struct REGIST_BASEECONOMY
{
	MAPTYPE MapNum;
	WORD OriginNum;
	WORD OriginPrice;
	WORD OriginAmount;
	WORD RequireNum;
	WORD RequirePrice;
	WORD RequireAmount;
	BYTE BuyRates;
	BYTE SellRates;
};
//////////////////////////////////////////////////////////////////////////

struct AUCTION_LISTFIELD
{
	WORD ItemIdx;
	WORD SellNum;
	WORD SellerID;
	WORD EndDate;
	WORD EndTime;
	DWORD StartPrice;
	DWORD ImmediatePrice;
	char Memo[256];
};

struct AUCTION_ONPAGELIST : public MSGBASE
{
	AUCTION_LISTFIELD Auction_List[10];
};

struct AUCTION_NEW_BOARDCONTENTS : public MSGBASE
{	
	WORD ItemIdx;
	WORD ItemNum;
	DWORD SellerID;
	DWORD EndDate;
	DWORD EndTime;
	DWORD StartPrice;
	DWORD ImmediatePrice;
	WORD MemoLength;
	char Memo[256];

	WORD GetMsgLength()	{	return sizeof(AUCTION_NEW_BOARDCONTENTS) - 256 + MemoLength + 1;	}
};
struct AUCTION_BOARDCONTENTS_INFO : public MSGBASE
{
	DWORD BidIdx;		// ��Ź�ȣ(�ʼ������� �����ϴ� PrimaryKey)
	WORD ItemIdx;
	WORD ItemNum;
	DWORD SellerID;
	DWORD EndDate;
	DWORD EndTime;
	DWORD StartPrice;
	DWORD ImmediatePrice;

	DWORD CurrentPrice;
	DWORD BidderNum;
	DWORD CurrentBidder;

	WORD MemoLength;
	char Memo[256];
};

struct AUCTION_BOARDPAGE_INFO : public MSGBASE
{
	WORD wCurPage;
	WORD wTotalPageNum;
	AUCTION_BOARDTITLE Title[MAX_BOARDTITLE_NUM];
};

// LBS ������ ����..
struct MSG_STREETSTALL_TITLE : public MSGBASE
{
	char Title[MAX_STREETSTALL_TITLELEN+1];
	WORD StallKind;
};

struct MSG_STREETSTALL_ITEMSTATUS : public MSG_ITEM
{
	POSTYPE		wAbsPosition;
	MONEYTYPE		dwMoney;
	WORD	wVolume;
};


struct STREETSTALLITEM
{
	WORD		wVolume; // ���ų����� ����
	WORD		wIconIdx;
	DWORD		dwDBIdx;
	DURTYPE		Durability;
	DWORD		dwRareIdx;
	MONEYTYPE		money;
	char		Locked;			// ����
	char		Fill;

	ITEMPARAM ItemParam;
};

struct STREETSTALL_INFO : public MSGBASE
{
	DWORD StallOwnerID;
	char Title[MAX_STREETSTALL_TITLELEN+1];
	STREETSTALLITEM Item[SLOT_STREETSTALL_NUM];
	WORD StallKind;

	WORD count;
	WORD RareCount;
	WORD PetItemCount;
	WORD TitanItemCount;
	WORD TitanEquipItemCount;
	
	CAddableInfoList AddableInfo;

	WORD GetSize()
	{
		return sizeof( STREETSTALL_INFO ) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();
	}
};

struct STREETSTALLTITLE_INFO : public MSGBASE
{
	char Title[MAX_STREETSTALL_TITLELEN+1];
};

struct STREETSTALL_BUYINFO : public MSG_ITEM
{
	DWORD	StallOwnerID;
	POSTYPE StallPos;
	DWORD	dwBuyPrice;
};

// LBS StreetStall 03.11.27
struct MSG_REGISTITEM : public MSG_ITEM
{
	DWORD dwData;
};

struct MSG_REGISTITEMEX : public MSG_REGISTITEM
{
	MONEYTYPE dwMoney;
};

struct MSG_REGISTBUYITEMEX : public MSG_REGISTITEMEX
{
	WORD wVolume;
	WORD wAbsPosition;
};

struct MSG_SELLITEM : public MSG_REGISTITEM
{
	DWORD count;
};

struct MSG_STALLMSG	: public MSGBASE
{
	DWORD	SellerID;
	DWORD	BuyerID;
	WORD	ItemIdx;
	WORD	count;
	DWORD	money;
};

struct ITEM_SELLACK_INFO : public MSGBASE
{
	//DWORD ItemDBIdx;			// check�� ����  �ӽ� ����
	MONEYTYPE Money;
	POSTYPE Pos;
};

struct APPEARANCE_INFO : public MSGBASE
{
	DWORD PlayerID;
	WORD WearedItem[eWearedItem_Max];
};
//pjs
//��� �˻� ���.
struct SEARCHLIST : public MSGBASE
{
	SEARCH_TYPE search_type;
	SORT_TYPE sort_type;
	char name[15];
	int pagenum;
};
//��� ���� ���.
struct SORTLIST : public MSGBASE
{
	SORT_TYPE sort_type;
	char name[15];
	int pagenum;
};

//��� ���, ���� ��� 
struct AUCTIONCANCEL : public MSGBASE
{
	CANCEL_TYPE cancel_type;
	int regIndex;
	char name[15];
	
};

//��� ��� ���� 
struct  REGISTERAUCTION : public MSGBASE
{
	int	 PRO_Index;
	int  PRO_Amount;
	char BID_DUEDATE[10];
	int  BID_HighPrice;
	int  BID_ImmediatePrice;
	char Auctioneer[15];
};

//��� ���� ���� 
struct JOINAUCTION : public MSGBASE
{
	int REG_Index;
	int BID_Price;
	char JOIN_Name[15];	
};

//��� ���� ������ ������ ����ü
struct REAUCTIONSUCCESS 
{
	char PRO_Name[15];
	int PRO_Amount;
	char Auctioneer[15];
	int PRO_Index;
	
};

//�޾ƿ� ������ ���� ����ü
struct AUCTIONSUCCESS : public MSGBASE
{	//���߿� ��ġ�� ����Ʈ�� 
	REAUCTIONSUCCESS resultlist[10];	
};

//����Ʈ ������ �޾ƿ� ����ü
struct REBASELIST 
{
	int REG_Index;
	char PRO_Name[15];
	int  PRO_Amount;
	int  BID_HighPrice;
	int  BID_ImmediatePrice;
	DWORD BID_DUEDATE[10];
	char BID_HighName[15];
	int  JOIN_Amount;
	DWORD  BID_LeftTime;
	char Auctioneer[15];
	int	 PRO_Index;
};
//�޾ƿ� ������ ���� ����ü
struct BASELIST : public MSGBASE
{
	REBASELIST imfolist[10];
};




///////////////////////////////////////

struct MSG_LEVEL : public MSGBASE
{
	enum EXPPOINTKIND
	{
		EXPPOINTKIND_ACQUIRE,
		EXPPOINTKIND_DIE,
	};
	LEVELTYPE Level;
	EXPTYPE CurExpPoint;
	EXPTYPE MaxExpPoint;
};
struct MSG_EXPPOINT : public MSGBASE
{
	enum EXPPOINTKIND
	{
		EXPPOINTKIND_ACQUIRE,
		EXPPOINTKIND_DIE,
		EXPPOINTKIND_ABILITYUSE,
	};
	EXPTYPE ExpPoint;
	BYTE ExpKind;
};
struct MSG_ABILLITY_EXPPOINT : public MSGBASE
{
	enum EXPPOINTKIND
	{
		EXPPOINTKIND_ACQUIRE,
		EXPPOINTKIND_DIE,
		EXPPOINTKIND_ABILITYUSE,
	};
	DWORD ExpPoint;
	BYTE ExpKind;
};
struct MSG_MUGONGEXP : public MSGBASE
{
	DWORD ExpPoint;
	DWORD absPos;
};
struct M2C_DAMAGE_MSG : public MSGBASE
{
	DWORD dwDamage;
};

//////////////////////////////////////////////////////////////////////////
// ���� ����
struct TACTICSTART_INFO 
{
	WORD TacticID;
	char Name[MAX_NAME_LENGTH+1];
	BYTE NeedMemberNum; //�����α�
	WORD WaitingTime; //���ð�
	WORD WaitingNaeryuk;//��⳻�� (����/��)
	WORD SuccessNaeryuk; //�����ϸ� �ٴ� ����
	int TacticStartEffect;
	WORD SkillNum;
	float PosX[MAX_PARTY_LISTNUM-1];
	float PosZ[MAX_PARTY_LISTNUM-1];
};

struct TACTIC_TOTALINFO
{
	VECTOR3 Pos;
	DWORD StartTime;
	DIRINDEX Direction;
	DWORD JoinedPlayerIDs[MAX_PARTY_LISTNUM];
	WORD TacticId;
};

struct SEND_TACTIC_TOTALINFO : public MSGBASE 
{
	BASEOBJECT_INFO baseinfo;
	TACTIC_TOTALINFO tacticinfo;
};

struct SEND_TACTIC_START : public MSGBASE
{
	DWORD OperatorID;
	WORD TacticID;
	DIRINDEX Direction;
};

struct SEND_TACTIC_JOIN : public MSGBASE
{
	DWORD TacticObjectID;
	DWORD JoinedPlayerID;
	BYTE Position;
};

struct TATIC_ABILITY_INFO
{
	WORD	wTypeAttack[MAX_TATIC_ABILITY_NUM];
	WORD	wTypeRecover[MAX_TATIC_ABILITY_NUM];
	float	fTypeBuffRate[MAX_TATIC_ABILITY_NUM];
	WORD	wTypeBuff[MAX_TATIC_ABILITY_NUM];
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Skill ����
struct MAINTARGET
{
	enum
	{
		MAINTARGETKIND_OBJECTID,
		MAINTARGETKIND_POS,
	};
	BYTE MainTargetKind;
	union {
		DWORD dwMainTargetID;		
		COMPRESSEDPOS cpTargetPos;
	};
	void SetMainTarget(DWORD id)
	{	MainTargetKind = MAINTARGETKIND_OBJECTID;	dwMainTargetID = id;	}
	void SetMainTarget(VECTOR3* pp)
	{	MainTargetKind = MAINTARGETKIND_POS;	cpTargetPos.Compress(pp);	}
	MAINTARGET()	{}
	MAINTARGET(DWORD id)	{	SetMainTarget(id);	}
	MAINTARGET(VECTOR3* pp){	SetMainTarget(pp);	}
};

struct SKILLOBJECT_INFO
{
	DWORD SkillObjectIdx;
	WORD SkillIdx;

	VECTOR3 Pos;
	DWORD StartTime;
	DIRINDEX Direction;
	BYTE SkillLevel;
	DWORD Operator;

	MAINTARGET MainTarget;

	DWORD BattleID;
	BYTE BattleKind;
	WORD Option;
};

enum SKILLKIND
{
	SKILLKIND_COMBO = 0,
	SKILLKIND_OUTERMUGONG,
	SKILLKIND_INNERMUGONG,
	SKILLKIND_SIMBUB,
	SKILLKIND_JINBUB,	
	SKILLKIND_MINING,
	SKILLKIND_COLLECTION,
	SKILLKIND_HUNT,
	SKILLKIND_TITAN,	// magi82 - Titan(070911) Ÿ��ź ����������Ʈ
	SKILLKIND_MAX
};

struct SKILLINFO
{
	WORD SkillIdx;	//���� index
	char SkillName[MAX_NAME_LENGTH+1];	//���� �̸�
	WORD SkillTooltipIdx;	//���� ���� �ε���
	LEVELTYPE RestrictLevel;		// ���� ����
	int	LowImage;	//������ �̹���
	int HighImage;	//������ �̹���
	
	WORD SkillKind;	//���� ����
	
	WORD WeaponKind;	//���� ����
	
	WORD SkillRange;	//���� ��Ÿ�

	WORD TargetKind;	//Ÿ��		0 : Ÿ��, 1 : �ڽ�
	WORD TargetRange;	//���� ����  => 0 �̸� �׸�����, �����̸� ���� ���� ������ 
	WORD TargetAreaIdx;	//���� ǥ�� �׸�����
	WORD TargetAreaPivot;	//������ �߽�		=>0 : ��ǥ����,  1 : �ڽ�
	WORD TargetAreaFix;	//ȿ�� ��� ���� ����		0 : ȿ�� ����� ���� (ó�� ������ ��� ����)	1 : ȿ�� ����� ������ (������ ���� ���ŵ�)		EX> �ߵ� : 0 , ���̾�� : 1
	
	WORD MoveTargetArea;	//�̵�����
	WORD MoveTargetAreaDirection;	//�̵�����		0 : ����	1 : Ÿ��������	2 : �߻����	3 : ����	*.�̵����ΰ� 1(�����ӵ��� �̵�) ���� ����
	float MoveTargetAreaVelocity;	//�ӵ�

	DWORD Duration;	//���ӽð�
	WORD Interval;	//ȿ�� ����
	WORD DelaySingleEffect;	// ����ȿ��������
	
	WORD ComboNum;	//���� Ÿ�Ա���		=>1~6 : �޺��ܰ� / 100 : ����
	
	WORD Life;	//���� ������
	
	WORD BindOperator;	//���� ��� �ӹ� ����	*. ������ �ַ� ���Ǵ� ������ ������ ����ϴ� ���� ĳ���Ͱ� �����Ҵ��� ��.	0 : �������	1 : �ӹڵ�
	
	int EffectStartTime;	//���� ����ð�
	int EffectStart;	//���� ����
	int EffectUse;		//��� ����
	int EffectSelf;	//�ڽ� ����
	
	int EffectMapObjectCreate;	//�� ���� ������Ʈ ����
	int EffectMineOperate;	//���� ����Ʈ

	DWORD DelayTime;	//���� ������ Ÿ��
	
	WORD FatalDamage;	//���� ����
	
	DWORD NeedExp[12];	//����ġ(12��)
	WORD NeedNaeRyuk[12];	//���� �Ҹ�(12��)
	
	WORD Attrib;	//���� �Ӽ�		0 : ����	1 : ȭ	2 : ��	3 : ��	4 : ��	5 : ��	6 : ���Ӽ�
	WORD NegativeResultTargetType;	//����ȿ�� Ÿ�� Ÿ��	���� ���ݺ��� �������� negative�� ȿ���� ����Ǵ� �ʵ��� ����Ÿ��
								//0 : Ÿ�� (��, ���͸�)	1 : ������ �Ʊ���	2 : �ڽŸ�	3 : ��/�Ʊ��� ������ ����  4 : ���� �ƴ� Ÿ��

	WORD TieUpType;	//��� Ÿ��		<=��� Ÿ�� ( 0 : �̵�,   1 : ����(�޺�+����)   2 : ���� )
	
	WORD ChangeSpeedType;	//�ӵ���ȭ Ÿ��		<=�ӵ���ȭ Ÿ�� ( 0 : ����,  1 : �̵�,  2 : ����������(�޺�) Ÿ�� ����)
	WORD ChangeSpeedRate;	//�ӵ���ȭ ����		<=�ӵ� ��ȭ�� ���� 50�̸� �̵��ӵ��� ��� 50% ���ϵ����� Ÿ���� ��� 50% ����� ȿ��

	WORD Dispel;	//����ȿ�� ����
	WORD PositiveResultTargetType;	//���ȿ�� Ÿ�� Ÿ��	*. ���� ���ݺ��� �������� positive �� ȿ���� ����Ǵ� �ʵ��� ����Ÿ��
								//0 : Ÿ�� (��, ���͸�)	1 : ������ �Ʊ���	2 : �ڽŸ�	3 : ��/�Ʊ��� ������ ����

	WORD Immune;	//�������

	WORD AIObject;	//AI ������Ʈ ��ȯ

	WORD MineCheckRange;	//����üũ �Ÿ�
	WORD MineCheckStartTime;	//���ڽ��۽ð�
	
	WORD CounterDodgeKind;		// 0:���� 1:�ݰ� 2:ȸ�� 3:���
	int CounterEffect;			// �ݰ� ����Ʈ

//041213 KES decrease skill power
	WORD DamageDecreaseForDist;
//---
	float CounterDodgeRate[12];	

	WORD FirstRecoverLife[12];	//������ ȸ��(12��)
	WORD FirstRecoverNaeRyuk[12];	//����ȸ��(12��)

	WORD ContinueRecoverLife[12];	//������ ���� ȸ��(12��)
	WORD ContinueRecoverNaeRyuk[12];	//�������� ȸ��(12��)
	WORD ContinueRecoverShield[12];	//ȣ�Ű��� ���� ȸ��(12��)
	
	float CounterPhyAttack[12];	//���� �ݰ� (12��)
	float CounterAttAttack[12];	//�Ӽ� �ݰ� (12��)
	float CriticalRate[12];
	float StunRate[12];
	WORD StunTime[12];
	
	float FirstPhyAttack[12];	//���� ����(12��)
	float FirstAttAttack[12];	//�Ӽ� ����(12��)
	
	WORD FirstAttAttackMin[12];	//�Ӽ������ּ�(12��)
	WORD FirstAttAttackMax[12];	//�Ӽ������ִ�(12��)
	WORD ContinueAttAttack[12];	//�Ӽ�����������(12��)		���� �Ӽ� �������� �ּ��ִ����
	float ContinueAttAttackRate[12];	//�Ӽ�����������(12��)		���ӼӼ�����Rate

	WORD AmplifiedPowerPhy[12];
	WORD AmplifiedPowerAtt[12];
	float AmplifiedPowerAttRate[12];
	
	float VampiricLife[12];
	float VampiricNaeryuk[12];
	float RecoverStateAbnormal[12];	// �����̻� ȸ��
	
	float DispelAttackFeelRate[12]; // �����ǿ� ���
	float ChangeSpeedProbability[12];	//�̵��ӵ� ���� Ȯ��
	
	WORD UpMaxLife[12];	//�ִ������ ���(12��)
	WORD UpMaxNaeRyuk[12];	//�ִ� ���� ���(12��)
	WORD UpMaxShield[12];	//�ִ� ȣ�Ű��� ���(12��)
	float UpPhyDefence[12];	//���� ��� ��� (12��)
	float UpAttDefence[12];	//�Ӽ� ��� ��� (12��)
	float UpPhyAttack[12];		//���� ���� ���(12��)
	
	WORD DownMaxLife[12];	//�ִ������ ����(12��)
	WORD DownMaxNaeRyuk[12];	//�ִ� ���� ����(12��)
	WORD DownMaxShield[12];	//�ִ� ȣ�Ű��� ����(12��)
	float DownPhyDefence[12];	//���� ��� ���� (12��)
	float DownAttDefence[12];	//�Ӽ� ��� ���� (12��)
	float DownPhyAttack[12];		//���� ���� ����(12��)
	
	DWORD SkillAdditionalTime[12];	//��ų �߰� �ð�(12��)
	WORD UpAttAttack[12];		//�Ӽ� ������(12��)
	
	float DamageRate[12];		//��������(12��)
	float AttackRate[12];			//�߰� ���ݷ�(12��)
	float UpCriticalRate[12];		//�ϰ� Ȯ��(12��

	float AttackLifeRate[12];		//ü�¾�ȭ
	float AttackShieldRate[12];		//ȣ�ž�ȭ

	float AttackSuccessRate[12];	//���� ���� Ȯ��

	float VampiricReverseLife[12];		//���ݴ��ҽ� ������ ����
	float VampiricReverseNaeryuk[12];	//���ݴ��ҽ� ���� ����

	DWORD AttackPhyLastUp[12];		//�����ܰ����� + Damage
	DWORD AttackAttLastUp[12];		//�ֳ��������� + Damage

	////////////////////////////////////////////////////////
	//06. 06 2�� ���� - �̿���
	WORD SkipEffect;	//����Ʈ ����(����)
	BOOL CanSkipEffect;	//����Ʈ ����(����) ���� ����
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ����/����
	// ����� ���� On/Off ������ �ڽ��� ���� ��ȭ��
	WORD SpecialState;
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ���� ��ȯ
	// ��ȯ ���� - 0: ��ȯ�Ұ�, 1: ������, 2:������, 3:��ΰ���
	WORD ChangeKind;
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// 06. 08. 2�� ���� - �̿���
	// �߰� ȸ�� ��
	// 2�� ���� ��ų�� �����ϴ� ��󺸴� ���� Ʋ���� ������ ���ؾ� �ϴ� ��ų�� ���� �߰�
	// �Ϲ� ���͵� ��밡��
	int AddDegree;
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// 06. 08. 2�� ���� - �̿���
	// �����Ÿ�
	// �����Ÿ� ������ Ÿ���� �ƴϴ�
	WORD SafeRange;
	////////////////////////////////////////////////////////

	//SW070127 Ÿ��ź
	WORD LinkSkillIdx;
};

//////////////////////////////////////////////////////////////////////////
// 06. 06. 2�� ���� - �̿���
// ���� ��ȯ �߰�
struct SKILLOPTION{
	WORD Index;				// �ɼ� Index
	WORD SkillKind;			// �������� ��ų ����
	WORD OptionKind;		// �ɼ� ����
	WORD OptionGrade;		// �ɼ� ���
	WORD ItemIndex;			// ������ Index

	int Life;				// ������
	int NaeRyuk;			// ����
	int Shield;				// ȣ�Ű���
	float PhyDef;			// ��������
	float AttDef;			// �Ӽ�����

	int Range;				// �����Ÿ�
	float ReduceNaeRyuk;	// ���¼Ҹ�
	float PhyAtk;			// ��������
	float BaseAtk;			// �⺻��������
	float AttAtk;			// �Ӽ�����

	long Duration;			// ���ӽð�
};

struct SKILLSTATSOPTION		// ���ݿ� ����Ǵ� �ɼǵ�
{
	int Life;				// ������
	int NaeRyuk;			// ����
	int Shield;				// ȣ�Ű���
	float PhyDef;			// ��������
	float AttDef;			// �Ӽ�����

	int Range;				// �����Ÿ�
	float ReduceNaeRyuk;	// ���¼Ҹ�
	float PhyAtk;			// ��������
	float BaseAtk;			// �⺻��������
	float AttAtk;			// �Ӽ�����
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// 06. 06. 2�� ���� - �̿���
// ����/����
// Ư�����¿� ����ü
struct SPECIAL_STATE_INFO{
	WORD	Index;

	DWORD	TickTime;		// �����ð�
	WORD	NaeRyuk;		// �����ð��� �Ҹ𳻷�

	DWORD	DelayTime;		// ���� ��� �ð�
	DWORD	DurationTime;	// ���ӽð�

	WORD	IconIdx;		// ���� ������ ��ȣ
};
//////////////////////////////////////////////////////////////////////////

#include ".\TargetList\TargetList.h"
#include ".\TargetList\TargetListIterator.h"
struct MSG_SKILL_START_SYN : public MSGBASE
{
	WORD SkillIdx;
	DWORD Operator;
	DIRINDEX SkillDir;
	MAINTARGET MainTarget;
	CTargetList TargetList;

	void InitMsg(WORD wSkillIdx,MAINTARGET* pMainTarget,float AngleDeg,DWORD pOperator)
	{
		Category = MP_SKILL;
		Protocol = MP_SKILL_START_SYN;
		SkillIdx = wSkillIdx;
		TargetList.Clear();
		SkillDir = DEGTODIR(AngleDeg);
        Operator = pOperator;
		memcpy(&MainTarget,pMainTarget,sizeof(MAINTARGET));
		dwObjectID = pOperator;
	}
	WORD GetMsgLength()
	{
		return sizeof(MSG_SKILL_START_SYN) - sizeof(CTargetList) + TargetList.GetTotalMsgLen();
	}
};

struct MSG_SKILL_OPERATE : public MSGBASE
{
	DWORD SkillObjectID;
	DWORD RequestorID;
	MAINTARGET MainTarget;
	CTargetList TargetList;

	void InitMsg(BYTE bProtocol,MAINTARGET* pMainTarget)
	{
		Category = MP_SKILL;
		Protocol = bProtocol;
		TargetList.Clear();
		memcpy(&MainTarget,pMainTarget,sizeof(MAINTARGET));
	}
	WORD GetMsgLength()
	{
		return sizeof(MSG_SKILL_OPERATE) - sizeof(CTargetList) + TargetList.GetTotalMsgLen();
	}
};

struct MSG_SKILLOBJECT_ADD : public MSGBASE
{
	SKILLOBJECT_INFO SkillObjectInfo;
	bool bCreate;
	CTargetList TargetList;

	void InitMsg(SKILLOBJECT_INFO* pSkillObjectInfo,bool Create)
	{
		Category = MP_SKILL;
		Protocol = MP_SKILL_SKILLOBJECT_ADD;
		memcpy(&SkillObjectInfo,pSkillObjectInfo,sizeof(SkillObjectInfo));
		SkillObjectInfo.StartTime = gCurTime - SkillObjectInfo.StartTime;
		TargetList.Clear();
		bCreate = Create;
	}
	WORD GetMsgLength()
	{
		return sizeof(MSG_SKILLOBJECT_ADD) - sizeof(CTargetList) + TargetList.GetTotalMsgLen();
	}
};

struct MSG_SKILL_SINGLE_RESULT : public MSGBASE
{
	DWORD SkillObjectID;
	DWORD SingleUnitNum;
	CTargetList TargetList;

	void InitMsg(DWORD SOID,DWORD SUNum)
	{
		Category = MP_SKILL;
		Protocol = MP_SKILL_SKILL_SINGLE_RESULT;
		SkillObjectID = SOID;
		SingleUnitNum = SUNum;		
		TargetList.Clear();
	}
	WORD GetMsgLength()
	{
		return sizeof(MSG_SKILL_SINGLE_RESULT) - sizeof(CTargetList) + TargetList.GetTotalMsgLen();
	}

};

struct SKILL_CHANGE_INFO
{
	WORD wMugongIdx; // ���� ��ȣ
	WORD wChangeRate; // ���� ȹ�� Ȯ��
	WORD wTargetMugongIdx; // ���� ���� ��ȣ
};


struct MSG_SKILL_DAMAGE_TARGETLIST : public MSGBASE
{
	DWORD SkillObjectID;
	DWORD RequestorID;
	MAINTARGET MainTarget;
	CTargetList TargetList;
	
	void InitMsg(BYTE bProtocol,MAINTARGET* pMainTarget)
	{
		Category = MP_SKILL;
		Protocol = bProtocol;
		TargetList.Clear();
		memcpy(&MainTarget,pMainTarget,sizeof(MAINTARGET));
	}
	WORD GetMsgLength()
	{
		return sizeof(MSG_SKILL_DAMAGE_TARGETLIST) - sizeof(CTargetList) + TargetList.GetTotalMsgLen();
	}
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ��� ����
struct KYUNGGONG_INFO
{
	WORD KyungGongIdx;		//���� index
	char KyungGongName[MAX_NAME_LENGTH+1];		//����̸�
	WORD NeedNaeRyuk;		//�Ҹ� ����
	WORD MoveType;			//�̵� Ÿ��
	float Speed;			//�̵� �ӵ�
	WORD ChangeTime;		//��� ��� ��ȯ �ð�
	int StartEffect;		//��� ��� ����
	int IngEffect;			//��� ���� ����
	int EndEffect;			//��� �� ����
};
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//���� ����
struct MUNPA_BOARD_INFO
{
	char MunpaBoardName[MAX_MUNPA_BOARD_NAME+1];
	DWORD RSubjectRank;
	DWORD RContentRank;
	DWORD WContentRank;
	DWORD DContentRank;
	DWORD BoardIDX;
};

struct MUNPA_BATTLERECORD_INFO 
{
	DWORD Win;
	DWORD Loss;
	DWORD Total;
}; 

struct MUNPA_HOME_INFO : public MSGBASE
{
	char IntroMsg[MAX_MUNPA_INTROMSG+1];
	DWORD MunpaID;
	DWORD BoardNum;
	MUNPA_BOARD_INFO BoardInfo[MAX_MUNPA_BOARD_NUM];
	BYTE Kind;
	
	char MunpaName[MAX_MUNPA_NAME+1];
	char MasterName[MAX_NAME_LENGTH+1];
	FAMETYPE MunpaFamous;
	BYTE MemberNum;
	char CreateDay[16];
	MUNPA_BATTLERECORD_INFO MunpaBattleRecordInfo[MAX_MUNPA_BATTLEKIND];
	BOOL bIsInMap;
};

struct MUNPALISTINFO_MAP : public MSGBASE
{
	DWORD MunpaID;
	DWORD MasterID;
	char MunpaName[MAX_MUNPA_NAME+1];
	FAMETYPE Famous;
	BYTE MemberNum;
	DWORD MunpaRank;
	MONEYTYPE MunpaMoney;
};

struct MUNPALISTINFO
{
	DWORD MunpaID;
	char MunpaMasterName[MAX_NAME_LENGTH+1];
	char MunpaName[MAX_MUNPA_NAME+1];
	FAMETYPE Famous;
	BYTE MemberNum;
	BYTE Kind;
};

struct SEND_MUNPACCEPT_INFO : public MSGBASE
{
	DWORD MunpaID;
	DWORD TargetID;
	BOOL bLast;
	char MunpaName[MAX_MUNPA_NAME+1];
};

struct SEND_MUNPA_ACCEPT_TO_MAP : public MSGBASE
{
	DWORD MunpaID;
	char MunpaName[MAX_MUNPA_NAME+1];
};

struct SEND_MUNPA_INFO : public MSGBASE
{
	MUNPALISTINFO Munpa[MAX_MUNPA_LIST];
	DWORD TotalResultNum;
};

struct SEND_MUNPAMEMBER_RANK_CHANGE : public MSGBASE
{
	DWORD TargetID;
	DWORD MunpaID;
	DWORD Rank;
};

struct SEND_MUNPAMEMBER_RANK_CHANGE_ACK : public MSGBASE
{
	DWORD MunpaID;
	BYTE Rank;
	FAMETYPE Fame;
};

struct SEND_MUNPALIST_SYN : public MSGBASE
{
	DWORD GotoPage;
	char OrderType[16];
};

struct MUNPA_SEARCH_SYN : public MSGBASE
{
	char SearchKey[32];
	DWORD GotoPage;
};

struct MUNPA_CREATE_INFO : public MSGBASE
{
	char MunpaName[MAX_MUNPA_NAME+1];
	char MunpaIntro[MAX_MUNPA_INTROMSG+1];
	BYTE Kind;
};

struct MUNPA_MODIFY_INTRO : public MSGBASE
{
	char MunpaIntro[MAX_MUNPA_INTROMSG+1];
	DWORD MunpaID;
};

struct MUNPA_SEMIMEMBER_INFO
{
	DWORD id;
	char Name[MAX_NAME_LENGTH+1];
	LEVELTYPE Level;
	FAMETYPE Famous;
	char RequestJoinDay[16];
};

struct SEND_MUNPA_SEMIMEMBER_LIST : public MSGBASE
{
	MUNPA_SEMIMEMBER_INFO MunpaMemberList[MAX_MUNPA_SEMIMEMBERLIST_NUM];
	DWORD TotalResultNum;
	DWORD MunpaID;
};

struct MUNPA_REGULARMEMBER_INFO
{
	DWORD id;
	char Name[MAX_NAME_LENGTH+1];
	DWORD Rank;
	LEVELTYPE Level;
	FAMETYPE Famous;
	BOOL bLogin;
	char lastvisitdate[16];
	char entrydate[16];
};

struct SEND_MUNPA_REGULARMEMBER_LIST : public MSGBASE
{
	MUNPA_REGULARMEMBER_INFO MunpaRegularMemberList[MAX_MUNPA_SEMIMEMBERLIST_NUM];
	DWORD TotalResultNum;
	DWORD MunpaID;
};

struct MUNPA_BOARD_NAME_NUM 
{
	char BoardName[MAX_MUNPA_BOARD_NAME+1];
	DWORD WritingsNum;
};

struct SEND_MUNPA_BOARD_INFO : public MSGBASE
{
	DWORD CurMunpaBoardNum;
	DWORD BoardIDX[MAX_MUNPA_BOARD_NUM];
	MUNPA_BOARD_NAME_NUM BoardNameNum[MAX_MUNPA_BOARD_NUM];
};

struct SEND_MUNPA_CREATE_INFO : public MSGBASE
{
	char MunpaName[MAX_MUNPA_NAME+1];
	DWORD dwData;
};

struct SEND_MUNPA_BOARD_RANK_INFO : public MSGBASE
{
	char MunpaBoardName[MAX_MUNPA_BOARD_NAME+1];
	DWORD RSubjectRank;
	DWORD RContentRank;
	DWORD WContentRank;
	DWORD DContentRank;
	DWORD ID; // C->S : MUNPAIDX , S->C : BOARDIDX but modify�� c->s : BOARDIDX
};

struct SEND_BOARD_CONTENT_INFO : public MSGBASE
{
	DWORD BoardIDX;
	DWORD RootContentsID;
	char Subject[MAX_MUNPA_BOARD_SUBJECT+1];
	char Content[MAX_MUNPA_BOARD_CONTENT+1];
	WORD GetMsgLength()
	{
		return sizeof(SEND_BOARD_CONTENT_INFO) - (MAX_MUNPA_BOARD_CONTENT+1) + strlen(Content)+1;
	}
};

struct BOARD_LIST_INFO
{
	char WriterName[MAX_NAME_LENGTH+1];
	char RegDate[32];
	int Count; //��ȸ��
	char Subject[MAX_MUNPA_BOARD_SUBJECT+1];
	int  Seq; //�۹�ȣ
	DWORD WritingsIDX;
	BYTE Depth;
};

struct SEND_MUNPA_BOARD_LIST_INFO : public MSGBASE
{
	BOARD_LIST_INFO BoardListInfo[MAX_MUNPA_BOARD_LIST];
	DWORD TotalPage;
};

struct SEND_MUNPA_BOARD_CONTENT_INFO : public MSGBASE
{
	BOARD_LIST_INFO BoardListInfo;
	BYTE DCRank;
	BYTE WCRank;
	char Content[MAX_MUNPA_BOARD_CONTENT+1];
	WORD GetMsgLength()
	{
		return sizeof(SEND_MUNPA_BOARD_CONTENT_INFO) - (MAX_MUNPA_BOARD_CONTENT+1) + strlen(Content)+1;
	}
};

struct MUNPA_WAREHOUSE_ITEM_INFO
{
//	ITEMBASE WarehouseItem[MAX_MUNPA_WAREITEM_PERTAB_NUM];
	ITEMBASE WarehouseItem[TABCELL_GUILDWAREHOUSE_NUM];
	MONEYTYPE money;
};

struct SEND_MUNPA_WAREHOUSE_ITEM_INFO : public MSGBASE
{
	MUNPA_WAREHOUSE_ITEM_INFO info;
	WORD wOptionCount;
	WORD wRareOptionCount;
	WORD wPetInfoCount;
	BYTE bTabNum;

	CAddableInfoList AddableInfo;

	WORD GetSize()
	{
		return sizeof( SEND_MUNPA_WAREHOUSE_ITEM_INFO ) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();
	}
};

struct PYOGUKLIST
{
	DWORD MaxCellNum;
	MONEYTYPE MaxMoney;
	MONEYTYPE BuyPrice;
};

struct SEND_PYOGUK_ITEM_INFO : public MSGBASE
{
	MONEYTYPE money;
	ITEMBASE PyogukItem[SLOT_PYOGUK_NUM];

	WORD wOptionCount;
	WORD wRareOptionCount;
	WORD wPetInfoCount;
	WORD wTitanItemCount;
	WORD wTitanEquipItemCount;
//	ITEM_OPTION_INFO PyogukItemOption[SLOT_PYOGUK_NUM];
//	ITEM_RARE_OPTION_INFO PyogukItemRareOption[SLOT_PYOGUK_NUM];
//	
//	//!����ȭ �ʿ�
//	int GetSize()
//	{
//		if(!wRareOptionCount)
//		{
//			return sizeof(SEND_PYOGUK_ITEM_INFO) - sizeof(ITEM_OPTION_INFO)*(SLOT_PYOGUK_NUM - wOptionCount);
//		}
//		else
//		{
//			return sizeof(SEND_PYOGUK_ITEM_INFO);
//		}
//		
//	}

	CAddableInfoList AddableInfo;

	WORD GetSize()
	{
		return sizeof( SEND_PYOGUK_ITEM_INFO ) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();
	}
	
};

struct SEND_PETINVEN_ITEM_INFO : public MSGBASE
{
	ITEMBASE PetInvenItem[SLOT_PETINVEN_NUM];

	WORD wOptionCount;
	WORD wRareOptionCount;

	CAddableInfoList AddableInfo;

	WORD GetSize()
	{
		return sizeof( SEND_PETINVEN_ITEM_INFO ) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();
	}
};

struct MSG_FRIEND_MEMBER_ADDDELETEID : public MSGBASE
{
	DWORD PlayerID;
	char Name[MAX_NAME_LENGTH+1];
};

struct FRIEND
{
	char Name[MAX_NAME_LENGTH+1];
	DWORD Id;
	BOOL IsLoggIn;
};

struct FRIEND_LIST
{
	FRIEND Friend[MAX_FRIEND_LIST];
	int totalnum;
};

struct MSG_FRIEND_LIST_DLG : public MSGBASE
{
	BYTE count;
	FRIEND FriendList[MAX_FRIEND_NUM];
	WORD GetMsgLength()
	{
		return sizeof(MSG_FRIEND_LIST_DLG) - (sizeof(FRIEND) * (MAX_FRIEND_NUM - count));
	}
};

struct MSG_FRIEND_SEND_NOTE : public MSGBASE
{
	DWORD FromId;
	char FromName[MAX_NAME_LENGTH+1];
	char ToName[MAX_NAME_LENGTH+1];
	char Note[MAX_NOTE_LENGTH+1];
	WORD GetMsgLength()
	{
		return sizeof(MSG_FRIEND_SEND_NOTE) - (MAX_NOTE_LENGTH+1) + strlen(Note)+1;
	}
};

struct MSG_GUILD_SEND_NOTE : public MSGBASE
{
	DWORD FromId;
	char FromName[MAX_NAME_LENGTH+1];
	char Note[MAX_NOTE_LENGTH+1];
	WORD GetMsgLength()
	{
		return sizeof(MSG_GUILD_SEND_NOTE) - (MAX_NOTE_LENGTH+1) + strlen(Note)+1;
	}
};

struct MSG_FRIEND_SEND_NOTE_ID : public MSGBASE
{
	DWORD TargetID;
	char FromName[MAX_NAME_LENGTH+1];
	char Note[MAX_NOTE_LENGTH+1];
	WORD GetMsgLength()
	{
		return sizeof(MSG_FRIEND_SEND_NOTE_ID) - (MAX_NOTE_LENGTH+1) + strlen(Note)+1;
	}
};

struct FRIEND_NOTE
{
	//note�� ���� �ҷ��´�. 
	char FromName[MAX_NAME_LENGTH+1];
	DWORD NoteID;
	char SendDate[16];
	BYTE bIsRead;
};

struct MSG_FRIEND_NOTE_LIST : public MSGBASE
{
	FRIEND_NOTE NoteList[NOTENUM_PER_PAGE];
	BYTE TotalPage;
};

struct MSG_FRIEND_DEL_NOTE : public MSGBASE
{
	DWORD NoteID;
	BOOL bLast;
};

struct MSG_FRIEND_READ_NOTE : public MSGBASE
{
	char FromName[MAX_NAME_LENGTH+1];
	
	DWORD NoteID;
	WORD ItemIdx;
	char Note[MAX_NOTE_LENGTH+1];
	WORD GetMsgLength()
	{
		return sizeof(MSG_FRIEND_READ_NOTE) - (MAX_NOTE_LENGTH+1) + strlen(Note)+1;
	}
};

//////////////////////////////////////////////////////////////////////////
//ä��
struct MSG_CHANNEL_INFO : public MSGBASE
{
	WORD PlayerNum[MAX_CHANNEL_NUM];
#ifdef _KOR_LOCAL_
	char ChannelName[MAX_CHANNEL_NAME+1];
#else
	bool bBattleChannel[MAX_CHANNEL_NUM];
    WORD wMoveMapNum;
	DWORD dwChangeMapState;
	char ChannelName[MAX_CHANNEL_NAME+1];
#endif
	DWORD dwUniqueIDinAgent;
	BYTE Count;
};

struct MSG_CHANNEL_INFO_MORNITOR : public MSGBASE
{
	MSG_CHANNEL_INFO	ChannelInfo;
	char				sServerIP[MAX_IPADDRESS_SIZE];
	WORD				wServerPort;
};
//////////////////////////////////////////////////////////////////////////


//------------ AUTOPATCH ���� �޽��� ----------------------
typedef struct tagServerTraffic : public MSGROOT
{
	LONG UserNum;
	int Returnkey;
	
}ServerTraffic;


//////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// ä�ð��ø޼��� //////////////////////////////////////

struct MSG_CHAT : public MSGBASE
{
	char	Name[MAX_NAME_LENGTH+1];
	char	Msg[MAX_CHAT_LENGTH+1];
	int		GetMsgLength() { return sizeof(MSG_CHAT) - MAX_CHAT_LENGTH + strlen(Msg); }
};

struct MSG_CHAT_WITH_SENDERID : public MSGBASE
{
	DWORD	dwSenderID;
	char	Name[MAX_NAME_LENGTH+1];
	char	Msg[MAX_CHAT_LENGTH+1];
	int		GetMsgLength() { return sizeof(MSG_CHAT_WITH_SENDERID) - MAX_CHAT_LENGTH + strlen(Msg); }
};

struct MSG_WHISPER : public MSGBASE
{
	DWORD	dwReceiverID;
	char	SenderName[MAX_NAME_LENGTH+1];
	char	ReceiverName[MAX_NAME_LENGTH+1];
	char	Msg[MAX_CHAT_LENGTH+1];
	int		GetMsgLength() { return sizeof(MSG_WHISPER) - MAX_CHAT_LENGTH + strlen(Msg); }
};

struct SEND_PARTY_CHAT : public MSGBASE
{
	DWORD	MemberID[MAX_PARTY_LISTNUM];
	BYTE	MemberNum;

	char	Name[MAX_NAME_LENGTH+1];
	char	Msg[MAX_CHAT_LENGTH+1];
	int		GetMsgLength() { return sizeof(SEND_PARTY_CHAT) - MAX_CHAT_LENGTH + strlen(Msg); }
};

struct MSG_CHAT_WORD : public MSGBASE
{
	WORD	wData;
	char	Msg[MAX_CHAT_LENGTH+1];
	int		GetMsgLength() { return sizeof(MSG_CHAT_WORD) - MAX_CHAT_LENGTH + strlen(Msg); }
};

/*
struct SEND_PARTY_CHAT : public MSG_CHAT
{
	DWORD MemberID[MAX_PARTY_LISTNUM];
	BYTE MemberNum;
};
*/


enum eChatError
{
	CHATERR_ERROR,
	CHATERR_NO_NAME,
	CHATERR_NOT_CONNECTED,
	CHATERR_OPTION_NOWHISPER,
};


//KES EXCHANGE 030922
//struct MSG_EXCHANGE_APPLY : public MSGBASE
//{
//	BYTE bAccept;	
//};


//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Battle System 

struct BATTLE_TEAMMEMBER_INFO
{
	DWORD MemberID;
	char MemberName[MAX_NAME_LENGTH+1];
	void set(DWORD id,char* name)
	{
		MemberID = id;
//		strcpy(MemberName,name);
		strncpy( MemberName, name, MAX_NAME_LENGTH );
		MemberName[MAX_NAME_LENGTH] = 0;

		MemberKillNum = 0;
		MemberDieNum = 0;
	}
	BYTE MemberKillNum;
	BYTE MemberDieNum;
};

struct BATTLE_INFO_BASE
{
	DWORD BattleID;
	BYTE BattleKind;
	BYTE BattleState;
	DWORD BattleTime;
};

struct MUNPAFIELD_TEAMINFO
{
	BYTE TotalKill;
	BYTE TotalDied;
	BATTLE_TEAMMEMBER_INFO TeamMember[MAX_MUNPAFIELD_MEMBERNUM];

	BATTLE_TEAMMEMBER_INFO* FindMember(DWORD id)
	{
		for(int n=0;n<MAX_MUNPAFIELD_MEMBERNUM;++n)
		{
			if(TeamMember[n].MemberID == id)
				return &TeamMember[n];
		}
		return NULL;
	}
	void AddTeamMember(DWORD id)
	{		
		for(int n=0;n<MAX_MUNPAFIELD_MEMBERNUM;++n)
		{
			if(TeamMember[n].MemberID == 0)
			{
				TeamMember[n].MemberID = id;
				break;
			}
		}
	}
};

struct MSG_BATTLESETTING_SYN_MUNPAFIELD : public BATTLE_INFO_BASE
{
	MUNPAFIELD_TEAMINFO TeamInfo[2];
};

//---------------------------------------------------------------------------------------
// Murim Field
struct MURIMFIELD_TEAMINFO
{
	BYTE TotalKill;
	BYTE TotalDied;
	BATTLE_TEAMMEMBER_INFO TeamMember[MAX_MURIMFIELD_MEMBERNUM];

	BATTLE_TEAMMEMBER_INFO* FindMember(DWORD id)
	{
		for(int n=0;n<MAX_MURIMFIELD_MEMBERNUM;++n)
		{
			if(TeamMember[n].MemberID == id)
				return &TeamMember[n];
		}
		return NULL;
	}
	void AddTeamMember(DWORD id)
	{		
		for(int n=0;n<MAX_MURIMFIELD_MEMBERNUM;++n)
		{
			if(TeamMember[n].MemberID == 0)
			{
				TeamMember[n].MemberID = id;
				break;
			}
		}
	}
};

struct MURIMFIELD_BATTLE_INFO
{
	WORD	wMurimBattleKind;	// ���� ���� ���� ����
	WORD	wMurimBattleReward;	// ���� ���� ����
};

struct MSG_BATTLESETTING_SYN_MURIMFIELD : public BATTLE_INFO_BASE
{
	MURIMFIELD_BATTLE_INFO	MurimBattleInfo;
	MURIMFIELD_TEAMINFO		TeamInfo[3];
};
//---------------------------------------------------------------------------------------


#define MAX_BATTLE_INFO_LENGTH	2048
struct MSG_BATTLE_INFO : public MSGBASE
{
	BYTE BattleKind;
	char info[MAX_BATTLE_INFO_LENGTH];
	WORD msglen;

	int GetMsgLength()
	{
		return sizeof(MSGBASE) + msglen + sizeof(BYTE);
	}
};

struct MSG_BATTLE_TEAMMEMBER_ADDDELETE : public MSGBASE

{
	BATTLE_TEAMMEMBER_INFO Member;
	BYTE Team;
};


//==============
//For VimuStreet
struct MSG_BATTLE_VIMU_CREATESTAGE : public MSGBASE
{
	DWORD	dwBattleID;
	VECTOR3 vPosStage;		//�̰� ������. �Ʒ��� �ٲ���.
//	COMPRESSEDPOS vPosStage
};
//===============


struct MSG_MONEY : public MSGBASE
{
	MONEYTYPE dwTotalMoney;		// ��ü �ݾ�
	BYTE bFlag;					// MsgFlag
};

//////////////////
//---for MURIM NET
struct MNPLAYER_BASEINFO
{
	DWORD		dwObjectID;
	WORD		wRankPoint;
	char		strPlayerName[MAX_NAME_LENGTH];
	LEVELTYPE	Level;
	char		strNick[MAX_NAME_LENGTH];
	WORD		wPlayCount;
	WORD		wWin;
	WORD		wLose;
	char		strMunpa[MAX_NAME_LENGTH];
	char		cbPositionInMunpa;
	char		cbTeam;
};

struct MSG_MNPLAYER_BASEINFO : public MSGBASE
{
	MNPLAYER_BASEINFO	Info;
};

struct MSG_MNPLAYER_BASEINFOLIST : public MSGBASE
{
	DWORD				dwTotalPlayerNum;

	MNPLAYER_BASEINFO	PlayerInfo[MAXPLAYER_IN_CHANNEL];

	DWORD GetMsgLength()
	{
		return sizeof(MSG_MNPLAYER_BASEINFOLIST)
				- sizeof(MNPLAYER_BASEINFO)*(MAXPLAYER_IN_CHANNEL - dwTotalPlayerNum);
	}	
};

struct CHANNEL_BASEINFO
{
	DWORD		dwChannelIndex;
	char		strChannelTitle[MAX_CHANNELTITLE_LENGTH];
	char		cbChannelKind;
	WORD		wMaxPlayer;
	WORD		wPlayerNum;
};

struct MSG_CHANNEL_BASEINFO : public MSGBASE
{
	CHANNEL_BASEINFO	Info;
};

struct MSG_CHANNEL_BASEINFOLIST : public MSGBASE
{
	DWORD				dwTotalChannelNum;

	CHANNEL_BASEINFO	ChannelInfo[MAXCHANNEL_IN_MURIMNET];

	DWORD GetMsgLength()
	{
		return sizeof(MSG_CHANNEL_BASEINFOLIST)
				- sizeof(CHANNEL_BASEINFO)*(MAXCHANNEL_IN_MURIMNET - dwTotalChannelNum);
	}
};

struct PLAYROOM_BASEINFO
{
	DWORD		dwPlayRoomIndex;
	char		strPlayRoomTitle[MAX_PLAYROOMTITLE_LENGTH];
	char		cbPlayRoomKind;
	WORD		wMaxObserver;
	WORD		wMaxPlayerPerTeam;
	MONEYTYPE	MoneyForPlay;
	WORD		wPlayerNum;
	char		cbStart;
	MAPTYPE		wMapNum;
};

struct MSG_PLAYROOM_BASEINFO : public MSGBASE
{
	PLAYROOM_BASEINFO	Info;
};

struct MSG_PLAYROOM_BASEINFOLIST : public MSGBASE
{
	DWORD				dwTotalPlayRoomNum;

	PLAYROOM_BASEINFO	PlayRoomInfo[MAXPLAYROOM_IN_MURIMNET];

	DWORD GetMsgLength()
	{
		return sizeof(MSG_PLAYROOM_BASEINFOLIST)
				- sizeof(PLAYROOM_BASEINFO)*(MAXPLAYROOM_IN_MURIMNET - dwTotalPlayRoomNum);
	}		
};

struct MSG_MNTEAMCHANGE : public MSGBASE
{
	DWORD	dwMoverID;
	BYTE	cbFromTeam;
	BYTE	cbToTeam;
};


//---������ (to Murim Server)
struct MSPLAYER_INFO
{
	DWORD	dwObjectID;
	BYTE	cbTeam;
};
struct MSG_MNPLAYROOM_INFO : public MSGBASE
{
	int		nIndex;
	int		nGameKind;
	int		nTotalPlayerNum;
	MAPTYPE wMapNum;

	MSPLAYER_INFO PlayerInfo[100];

	WORD GetMsgLength()
	{
		return sizeof(MSG_MNPLAYROOM_INFO) - sizeof(MSPLAYER_INFO)*(100 - nTotalPlayerNum);
	}
};

struct WANTEDINFO
{
	WANTEDTYPE WantedIDX;
	MONEYTYPE TotalPrize;
	DWORD WantedChrIDX;
	char WantedChrName[MAX_NAME_LENGTH+1];
	DWORD RegistChrIDX;
	int TotalVolunteerNum;
};

struct WANTEDINFO_LOAD
{
	WANTEDINFO m_WantedInfo[MAX_QUERY_RESULT];
	int count;
};

struct SEND_WANTEDINFO_TO_SERVER : public MSGBASE
{
	WANTEDINFO Info;
};

struct WANTEDLIST
{
	WANTEDTYPE WantedIDX;
	DWORD WantedChrID;
	char WantedName[MAX_NAME_LENGTH+1];
	char RegistDate[11];
};

struct SEND_WANTEDLIST_TO_CLIENT : public MSGBASE
{
	WANTEDLIST List;
};

struct SEND_WANTEDLIST : public MSGBASE
{
//	WORD WantedIdx; // himself
	DWORD WantedIdx; // himself
	WANTEDLIST List[MAX_WANTED_NUM];
};

struct SEND_WANTED_NOTCOMPLETE : public MSGBASE
{
	WANTEDTYPE WantedIDX;
	char CompleterName[MAX_NAME_LENGTH+1];
	BYTE type;
};

struct SEND_WANTED_ORDER_SYN : public MSGBASE
{
	WORD page;
	BYTE type;
};

struct WANTNPCLIST
{
	WANTEDTYPE WantedIDX;
	char WantedName[MAX_NAME_LENGTH+1];
	MONEYTYPE Prize;
	BYTE VolunteerNum;
};

struct SEND_WANTNPCLIST : public MSGBASE
{
	WANTNPCLIST WantNpcList[WANTUNIT_PER_PAGE];
	WORD TotalPage;
};

struct SEND_GUILD_CREATE_SYN : public MSGBASE
{
	char GuildName[MAX_GUILD_NAME+1];
	char Intro[MAX_GUILD_INTRO+1];
	int GetMsgLength()
	{
		return sizeof(SEND_GUILD_CREATE_SYN) - (MAX_GUILD_INTRO+1) + strlen(Intro)+1;
	}
};
struct GUILDINFO
{
	DWORD GuildIdx;
	DWORD MasterIdx;
	char MasterName[MAX_NAME_LENGTH+1];
	char GuildName[MAX_GUILD_NAME+1];
	// 06. 03. ���İ��� - �̿���
	char GuildNotice[MAX_GUILD_NOTICE+1];
	DWORD UnionIdx;
	MAPTYPE MapNum;
	BYTE GuildLevel;
	MARKNAMETYPE MarkName;
	//SW070104 �������ϱ�
	BYTE LvUpCounter;
	BYTE bNeedMasterChecking;

};
struct GUILDMEMBERINFO
{
	DWORD MemberIdx;
	char MemberName[MAX_NAME_LENGTH+1];
	LEVELTYPE Memberlvl;
	BYTE Rank;
	BOOL bLogged;
	void InitInfo(DWORD PlayerIDX, char* PlayerName, LEVELTYPE Playerlvl, BYTE PlayerRank = GUILD_MEMBER, BOOL bLogin = FALSE)
	{
		MemberIdx = PlayerIDX;
		strcpy(MemberName, PlayerName);
		Memberlvl = Playerlvl;
		Rank = PlayerRank;
		bLogged = bLogin;
	}
};
struct SEND_GUILD_CREATE_ACK : public MSGBASE
{
	DWORD GuildIdx;
	char GuildName[MAX_GUILD_NAME+1];
};
struct SEND_GUILD_CREATE_NOTIFY : public MSGBASE
{
	GUILDINFO GuildInfo;
	LEVELTYPE MasterLvl;
};
struct SEND_GUILD_MEMBER_INFO : public MSGBASE
{
	DWORD GuildIdx;
	char GuildName[MAX_GUILD_NAME+1];	
	GUILDMEMBERINFO MemberInfo;
};
struct SEND_GUILD_TOTALINFO : public MSGBASE
{
	GUILDINFO GuildInfo;
	//SW060719 ��������Ʈ
	GUILDPOINT_INFO PointInfo;
	int membernum;
	GUILDMEMBERINFO MemberInfo[MAX_GUILDMEMBER_NUM];
	void InitGuildInfo(GUILDINFO* pInfo)
	{
		GuildInfo.GuildIdx = pInfo->GuildIdx;
		GuildInfo.GuildLevel = pInfo->GuildLevel;
		strcpy(GuildInfo.GuildName, pInfo->GuildName);
		GuildInfo.MapNum = pInfo->MapNum;
		GuildInfo.MasterIdx = pInfo->MasterIdx;
		// 06. 03. ���İ��� - �̿���
		strcpy(GuildInfo.GuildNotice, pInfo->GuildNotice);
		GuildInfo.UnionIdx = pInfo->UnionIdx;
		strcpy(GuildInfo.MasterName, pInfo->MasterName);
		GuildInfo.MarkName = pInfo->MarkName;
		//SW070103 ��������Ʈ����
		GuildInfo.LvUpCounter = pInfo->LvUpCounter;
		GuildInfo.bNeedMasterChecking = pInfo->bNeedMasterChecking;
	}
	void InitPointInfo(GUILDPOINT_INFO* pInfo)
	{
		memcpy(&PointInfo, pInfo, sizeof(GUILDPOINT_INFO));
	}
	int GetMsgLength() { return sizeof(SEND_GUILD_TOTALINFO) - (MAX_GUILDMEMBER_NUM-membernum)*sizeof(GUILDMEMBERINFO);	}
};
struct SEND_GUILD_INVITE : public MSGBASE
{
	DWORD MasterIdx;
	char MasterName[MAX_NAME_LENGTH+1];
	char GuildName[MAX_GUILD_NAME+1];
};

struct SEND_GUILD_NICKNAME : public MSGBASE
{
	DWORD TargetIdx;
	char NickName[MAX_GUILD_NICKNAME+1];
};

struct SEND_GUILD_NAME : public MSGBASE
{
	char GuildName[MAX_GUILD_NAME+1];
};

struct SEND_GUILDMEMBER_LOGININFO : public MSGBASE
{
	DWORD MemberIdx;
	BOOL bLogIn;
};

struct SEND_GUILD_HUNTEDMOB_INFO : public MSG_DWORD2
{};

#define GUILDMARKBPP	2
#define GUILDMARKWIDTH	16
#define GUILDMARKHEIGHT	12
#define GUILDMARK_BUFSIZE	(GUILDMARKBPP*GUILDMARKWIDTH*GUILDMARKHEIGHT)
#define GUILDUNIONMARKHEIGHT	15
#define GUILDUNIONMARK_BUFSIZE	(GUILDMARKBPP*GUILDMARKWIDTH*GUILDUNIONMARKHEIGHT)

struct MSG_GUILDMARK_IMG : public MSGBASE
{
	DWORD GuildIdx;
	MARKNAMETYPE MarkName;
	char imgData[GUILDMARK_BUFSIZE];
};

struct MSG_GUILDUNIONMARK_IMG : public MSGBASE
{
	DWORD	dwMarkIdx;
	DWORD	dwGuildUnionIdx;
	char	imgData[GUILDUNIONMARK_BUFSIZE];
};
//----------------------------------


// GuildFieldWar ----------------------------------------------------------------
struct MSG_GUILDFIELDWAR_INFO : public MSGBASE
{
	DWORD		dwEnemyGuildNum;
	GUILDINFO	EnemyGuildInfo[100];

	MSG_GUILDFIELDWAR_INFO()
	{
		dwEnemyGuildNum = 0;
	}
	void	AddEnemyGuildInfo( GUILDINFO* pInfo, DWORD dwMoney = 0 )
	{
		if( dwEnemyGuildNum < 100 )
		{
			EnemyGuildInfo[dwEnemyGuildNum] = *pInfo;
			EnemyGuildInfo[dwEnemyGuildNum].MarkName = dwMoney;
			++dwEnemyGuildNum;
		}
	}
	int		GetMsgLength()	{ return sizeof(MSG_GUILDFIELDWAR_INFO) - (100-dwEnemyGuildNum)*sizeof(GUILDINFO); }
};
//---------------------------------------------------------------------------------------------


// RaMa ( 04.07.12 )
#pragma pack(1)

typedef DWORD	QSTATETYPE;
#define MAX_BIT ((sizeof(QSTATETYPE))*(8))

#define YEARTOSECOND	31536000
#define MONTHTOSECOND	2592000
#define DAYTOSECOND		86400
#define HOURTOSECOND	3600
#define MINUTETOSECOND	60

#define SECTOMILLISEC	1000

extern DWORD DayOfMonth[];
extern DWORD DayOfMonth_Yundal[];

//������������������������������������������������������������������
//��stTIME														  ��
//������������������������������������������������������������������
struct stTIME{
	QSTATETYPE		value;

	stTIME():value(0) {}

	void SetTime(DWORD time)	{	value = time;	}
	void SetTime(DWORD year, DWORD month, DWORD day, DWORD hour, DWORD minute, DWORD second)
	{
		value=0;
		QSTATETYPE ch=0;

		ch = year;
		value = (value | (ch<<28));
		ch = month;
		value = (value | (ch<<24));
		ch = day;
		value = (value | (ch<<18));
		ch = hour;
		value = (value | (ch<<12));
		ch = minute;
		value = (value | (ch<<6));
		ch = second;
		value = (value | ch);
	}

	//
	DWORD GetYear()		{	return value>>28;	}
	DWORD GetMonth()	{	DWORD msk = value<<4;		return msk>>28;		}
	DWORD GetDay()		{	DWORD msk = value<<8;		return msk>>26;		}
	DWORD GetHour()		{	DWORD msk = value<<14;		return msk>>26;		}
	DWORD GetMinute()	{	DWORD msk = value<<20;		return msk>>26;		}
	DWORD GetSecond()	{	DWORD msk = value<<26;		return msk>>26;		}
	inline BOOL	operator >(const stTIME& time);
	inline void operator +=(const stTIME& time);
	inline void operator -=(const stTIME& time);
	inline void operator =(const stTIME& time);

	//SW080104 �����۸� �ð��� ������ �߰� ����
	enum stTIEM_KIND{ST_SEC,ST_MIN,ST_HOUR,ST_DAY};
	inline void AddTimeByValue(DWORD tVal, int flg_valueKind = ST_MIN);
};

inline BOOL	 stTIME::operator >(const stTIME& time)
{
	BOOL bResult = FALSE;
	stTIME ctime = time;

	if( this->GetYear() == ctime.GetYear() )
	{
		if( this->GetMonth() > ctime.GetMonth() )
			bResult = TRUE;
		else if( this->GetMonth() == ctime.GetMonth() )
		{
			if( this->GetDay() > ctime.GetDay() )
				bResult = TRUE;
			else if( this->GetDay() == ctime.GetDay() )
			{
				if( this->GetHour() > ctime.GetHour() )
					bResult = TRUE;
				else if( this->GetHour() == ctime.GetHour() )
				{
					if( this->GetMinute() > ctime.GetMinute() )
						bResult = TRUE;
					else if( this->GetMinute() == ctime.GetMinute() )
						if( this->GetSecond() > ctime.GetSecond() )
							bResult = TRUE;
				}						
			}
		}
	}
	else if( this->GetYear() > ctime.GetYear() )
		bResult = TRUE;

	return bResult;
}
inline void	 stTIME::operator +=(const stTIME& time)
{
	stTIME atime = time;
	int year, month, day, hour, minute, second, calcmonth;
	SYSTEMTIME systime;
	GetLocalTime(&systime);

	year = this->GetYear() + atime.GetYear();
	month = this->GetMonth() + atime.GetMonth();
	day = this->GetDay() + atime.GetDay();
	hour = this->GetHour() + atime.GetHour();
	minute = this->GetMinute() + atime.GetMinute();
	second = this->GetSecond() + atime.GetSecond();

	if( this->GetMonth() <= 0 )			return;
	else if( this->GetMonth() == 1 )	calcmonth = 11;
	else								calcmonth = this->GetMonth()-1;

	if(second >= 60)
	{
		++minute;
		second -= 60;
	}
	if(minute >= 60)
	{
		++hour;
		minute -= 60;
	}
	if(hour >= 24)
	{
		++day;
		hour -= 24;
	}
	if( (systime.wYear%4) == 0 )
	{
		if(day > (int)(DayOfMonth_Yundal[calcmonth]))
		{
			++month;
			day -= DayOfMonth_Yundal[calcmonth];
		}
	}
	else
	{
		if(day > (int)(DayOfMonth[calcmonth]))
		{
			++month;
			day -= DayOfMonth[calcmonth];
		}
	}
	if(month > 12)
	{
		++year;
		month -= 12;
	}
	
	this->SetTime(year, month, day, hour, minute, second);
}
inline void	 stTIME::operator -=(const stTIME& time)
{
	stTIME atime = time;
	int year, month, day, hour, minute, second, calcmonth;
	SYSTEMTIME systime;
	GetLocalTime(&systime);

	year = this->GetYear() - atime.GetYear();
	month = this->GetMonth() - atime.GetMonth();
	day = this->GetDay() - atime.GetDay();
	hour = this->GetHour() - atime.GetHour();
	minute = this->GetMinute() - atime.GetMinute();
	second = this->GetSecond() - atime.GetSecond();

	if( this->GetMonth() <= 0 )			return;
	else if( this->GetMonth() == 1 )	calcmonth = 11;
	else								calcmonth = this->GetMonth()-2;

	if(second < 0)
	{
		--minute;
		second += 60;
	}
	if(minute < 0)
	{
		--hour;
		minute += 60;
	}
	if(hour < 0)
	{
		--day;
		hour += 24;
	}
	if( (systime.wYear%4) == 0 )
	{
		if(day < 0)
		{
			--month;
			day += DayOfMonth_Yundal[calcmonth]; 
		}		
	}
	else
	{
		if(day < 0)
		{
			--month;
			day += DayOfMonth[calcmonth]; 
		}
	}
	if(month <= 0 && 0 < year)
	{
		--year;
		month += 12;
	}
	
	this->SetTime(year, month, day, hour, minute, second);
}
inline void	 stTIME::operator =(const stTIME& time)
{
	stTIME atime = time;
	this->SetTime(atime.GetYear(), atime.GetMonth(), atime.GetDay(), atime.GetHour(), atime.GetMinute(), atime.GetSecond());
}
//

inline void stTIME::AddTimeByValue(DWORD tVal, int flg_valueKind/* = ST_MIN*/)
{
	DWORD day = 0;
	switch(flg_valueKind)
	{
	case ST_SEC:
		day = tVal / 86400; //(60*60*24);
		break;
	case ST_MIN:
		day = tVal / 1440;	//(60*24);
		break;
	case ST_HOUR:
		day = tVal / 24;
		break;
	case ST_DAY:
		day = tVal;
		break;
	default:
		//__asm int 3;
		break;
	}

	DWORD Curyear = this->GetYear();
	DWORD CurMonth = this->GetMonth();
	day += this->GetDay();
	DWORD hour = this->GetHour() + ( tVal %(24*60) )/60;
	DWORD minute = this->GetMinute() + ( tVal %(24*60) )%60;

	DWORD* pDayOfMonth = NULL;
	if(Curyear%4 == 0)
	{
		pDayOfMonth = DayOfMonth_Yundal;
	}
	else
	{
		pDayOfMonth = DayOfMonth;
	}

	while( day > pDayOfMonth[CurMonth -1] )
	{
		day -= pDayOfMonth[CurMonth - 1];
		CurMonth++;

		if( CurMonth > 12 )
		{
			++Curyear;
			CurMonth = 1;

			if(Curyear%4 == 0)
			{
				pDayOfMonth = DayOfMonth_Yundal;
			}
			else
			{
				pDayOfMonth = DayOfMonth;
			}
		}
	}

	this->SetTime(Curyear, CurMonth, day, hour, minute, 0);

}

struct stPlayTime {
	DWORD value;

	stPlayTime::stPlayTime()
	{
		value = 0;
	}
	void GetTime(int& Year, int& Day, int& Hour, int& Minute, int& Second)
	{
		int mv = 0;

		Year = value/YEARTOSECOND;
		mv = value%YEARTOSECOND;

		Day = mv/DAYTOSECOND;
		mv = mv%DAYTOSECOND;

		Hour = mv/HOURTOSECOND;
		mv = mv%HOURTOSECOND;

		Minute = mv/MINUTETOSECOND;
		Second = mv%MINUTETOSECOND;		
	}
};


// LBS 04.01.06----------------------------------
typedef struct tagField{
	DWORD b0:1; 
	DWORD b1:1;
	DWORD b2:1;
	DWORD b3:1;
	DWORD b4:1;
	DWORD b5:1;
	DWORD b6:1;
	DWORD b7:1;

	DWORD b8:1;
	DWORD b9:1;
	DWORD b10:1;
	DWORD b11:1;
	DWORD b12:1;
	DWORD b13:1;
	DWORD b14:1;
	DWORD b15:1;

	DWORD b16:1;
	DWORD b17:1;
	DWORD b18:1;
	DWORD b19:1;
	DWORD b20:1;
	DWORD b21:1;
	DWORD b22:1;
	DWORD b23:1;

	DWORD b24:1;
	DWORD b25:1;
	DWORD b26:1;
	DWORD b27:1;
	DWORD b28:1;
	DWORD b29:1;
	DWORD b30:1;
	DWORD b31:1;

} stFIELD, *pFIELD;

typedef union tagQuestFlag{
	QSTATETYPE	value;
	stFIELD		flag;

	tagQuestFlag():value(0) {}

	void SetField(BYTE bit, BOOL bSetZero=FALSE) // nBit�� ��ȿ ���ڴ� DWORD�ΰ��1 ~ 32 �̴�
	{
		if( bit  < 1 || bit > MAX_BIT ) return;

		QSTATETYPE ch;
		bSetZero ? ch=1 : ch=0;
		value = (value | (ch<<(MAX_BIT- bit)));
	}

	BOOL IsSet(BYTE bit)
	{
		// ������ �������� ��� ���� ����Ǿ��ٰ� �˸���.
		if( bit  < 1 || bit > MAX_BIT ) return TRUE;

		QSTATETYPE ch = 1;
		return (value & (ch<<(MAX_BIT-bit)))? TRUE:FALSE;
	}

} QFLAG, *pQFLAG;


struct QBASE
{
	WORD		QuestIdx;
	QSTATETYPE	state;
};

struct QMBASE
{
	WORD		QuestIdx;	
	QFLAG		state;
	DWORD		EndParam;
	DWORD		Time;
	BYTE		CheckType;
	DWORD		CheckTime;
};

struct QSUBASE
{
	WORD		QuestIdx;
	WORD		SubQuestIdx;
	QSTATETYPE	state;
	QSTATETYPE	time;
};

struct QITEMBASE
{
	DWORD		ItemIdx;
	WORD		Count;
	WORD		QuestIdx;
};

struct SEND_QUEST_IDX_SIMPLE : public MSGBASE
{
	DWORD MainQuestIdx;
};

struct SEND_QUEST_IDX : public MSGBASE
{
	WORD		MainQuestIdx;
	WORD		SubQuestIdx;
	DWORD		dwFlag;
	DWORD		dwData;
	DWORD		dwTime;
};

struct SEND_QUESTITEM_IDX : public MSGBASE
{
	DWORD	dwItemIdx;
	DWORD	dwItemNum;
};

struct SEND_QUEST_TOTALINFO : public MSGBASE
{
	int GetSize() {
		return (sizeof(SEND_QUEST_TOTALINFO) - sizeof(QBASE)*(50 - wCount));
	}

	WORD	wCount;
	QBASE	QuestList[50];
};

struct SEND_MAINQUEST_DATA : public MSGBASE
{
	int GetSize() {
		return (sizeof(SEND_MAINQUEST_DATA) - sizeof(QMBASE)*(100 - wCount));
	}

	WORD	wCount;
	QMBASE	QuestList[100];
};

struct SEND_SUBQUEST_DATA : public MSGBASE
{
	int GetSize() {
		return (sizeof(SEND_SUBQUEST_DATA) -sizeof(QSUBASE)*(100-wCount));
	}

	WORD	wCount;
	QSUBASE	QuestList[100];
};

struct SEND_SUBQUEST_UPDATE : public MSGBASE
{
	WORD	wQuestIdx;
	WORD	wSubQuestIdx;
	DWORD	dwMaxCount;
	DWORD	dwData;
	DWORD	dwTime;
};

struct SEND_QUESTITEM : public MSGBASE
{
	int GetSize() {
		return (sizeof(SEND_QUESTITEM)-sizeof(QITEMBASE)*(100-wCount));
	}

	WORD		wCount;
	QITEMBASE	ItemList[100];
};

struct QUESTINFO {
	DWORD	m_dwIdx;		// Quest idx
	QFLAG	m_flag;			// Quest State Value
};

struct JOURNALINFO
{
	DWORD Index;
	BYTE Kind;
	DWORD Param;
	DWORD Param_2;			// SubQuestIndex
	DWORD Param_3;			// Quest �������� �Ϸ�����
	BYTE bSaved;
	char ParamName[MAX_NAME_LENGTH+1];
	char RegDate[11];
};

struct SEND_JOURNALLIST : public MSGBASE
{
	WORD	wCount;
	JOURNALINFO m_Info[MAX_JOURNAL_ITEM];
	int GetSize() {
	return (sizeof(SEND_JOURNALLIST)-sizeof(JOURNALINFO)*(MAX_JOURNAL_ITEM-wCount));
	}

};

struct SEND_JOURNALINFO : public MSGBASE
{
	JOURNALINFO		m_Info;
};

struct SEND_JOURNAL_DWORD : public MSGBASE
{
	DWORD Param;
};




// PartyIcon
struct MoveCoord
{
	WORD posX;
	WORD posZ;

	MoveCoord::MoveCoord()
	{
		posX = posZ = 0;
	}
};

struct PARTYICON_MOVEINFO
{
	DWORD	dwMoverID;
	BYTE	tgCount;
	WORD	KyungGongIdx;
	BYTE	MoveState;
	MoveCoord	cPos;
	MoveCoord	tgPos[MAX_CHARTARGETPOSBUF_SIZE];

	PARTYICON_MOVEINFO::PARTYICON_MOVEINFO()
	{
		dwMoverID	= 0;
		tgCount		= 0;
		KyungGongIdx = 0;
		MoveState	= 0;
		memset(&cPos, 0, sizeof(MoveCoord));
		memset(tgPos, 0, sizeof(MoveCoord)*MAX_CHARTARGETPOSBUF_SIZE);
	}

	int GetSize()
	{
		return sizeof(PARTYICON_MOVEINFO)-sizeof(MoveCoord)*(MAX_CHARTARGETPOSBUF_SIZE-tgCount);
	}
};

struct SEND_PARTYICON_MOVEINFO : public MSGBASE
{
	PARTYICON_MOVEINFO MoveInfo;

	int GetSize()
	{
		return sizeof(MSGBASE)+MoveInfo.GetSize();
	}
};

struct SEND_PARTYICON_REVIVE : public MSGBASE
{
	DWORD		dwMoverID;
	MoveCoord	Pos;
};


// ��������������������������������������������������������������
// �� RaMa - ItemMall Item                                     ��
// ��������������������������������������������������������������
struct SHOP_ITEM
{
	ITEMBASE	Item[SLOT_SHOPITEM_NUM];
};

struct SHOPITEMBASE
{
	ITEMBASE	ItemBase;	
	DWORD		Param;				// ����� �ð��� �����θ� ���δ�. 1-����ð�, 2-�÷��̽ð�
	stTIME		BeginTime;
	DWORD		Remaintime;

	SHOPITEMBASE::SHOPITEMBASE()
	{
		memset(&ItemBase, 0, sizeof(ITEMBASE));
		Param = 0;
		BeginTime.value = 0;
		Remaintime=0;
	}
};

struct SHOPITEMWITHTIME
{
	SHOPITEMBASE	ShopItem;
	DWORD			LastCheckTime;

	SHOPITEMWITHTIME::SHOPITEMWITHTIME()
	{
		LastCheckTime = 0;
	}
};

struct SHOPITEMUSEBASE
{
	POSTYPE ShopItemPos;
	WORD	ShopItemIdx;
	POSTYPE TargetItemPos;
	WORD	TargetItemIdx;
};

struct SHOPITEM_SIMPLE
{
	ITEMBASE	Item;
	stTIME		EndTime;

	SHOPITEM_SIMPLE::SHOPITEM_SIMPLE()
	{
		memset(&Item, 0, sizeof(ITEMBASE));
		EndTime.value = 0;
	}
};

struct MSG_SHOPITEM_USE_SYN : public MSGBASE
{
	SHOPITEMUSEBASE UseBaseInfo;
};

struct SEND_SHOPITEM_BASEINFO : public MSGBASE
{
	SHOPITEMBASE	ShopItemBase;
	POSTYPE			ShopItemPos;
	WORD			ShopItemIdx;
};


struct SEND_SHOPITEM_USEONE : public MSGBASE
{
	SHOPITEMBASE	ShopItemBase;
};

struct SEND_SHOPITEM_USEDINFO : public MSGBASE
{
	WORD ItemCount;
	SHOPITEMBASE Item[100];


	SEND_SHOPITEM_USEDINFO::SEND_SHOPITEM_USEDINFO()
	{
		ItemCount = 0;
		memset(Item, 0, sizeof(SHOPITEMBASE)*100);
	}

	int GetSize()	{
		return (sizeof(SEND_SHOPITEM_USEDINFO) - sizeof(SHOPITEMBASE)*(100-ItemCount));
	}
};

struct SEND_SHOPITEM_INFO : public MSGBASE
{
	WORD		ItemCount;
	ITEMBASE	Item[SLOT_SHOPITEM_NUM];

	SEND_SHOPITEM_INFO::SEND_SHOPITEM_INFO() {
		ItemCount = 0;
		memset(Item, 0, sizeof(ITEMBASE)*SLOT_SHOPITEM_NUM);
	}
	int GetSize() {
		return (sizeof(SEND_SHOPITEM_INFO) - sizeof(ITEMBASE)*(SLOT_SHOPITEM_NUM-ItemCount));
	}
};


struct SEND_SHOPITEMOPTION : public MSGBASE
{
	SHOPITEMOPTION	Info;

	SEND_SHOPITEMOPTION::SEND_SHOPITEMOPTION() {
		memset(&Info, 0, sizeof(Info));
	}
};

//


// ��������������������������������������������������������������
// �� RaMa - MoveDialog		                                   ��
// ��������������������������������������������������������������
struct stMOVEPOINT
{
	DWORD value;

	stMOVEPOINT::stMOVEPOINT()
	{
		value = 0;
	}
	void SetMovePoint(WORD wx, WORD wz)
	{
		value = 0;
		DWORD msk = 0;
		
		msk = wx;
		value = (value | (msk<<16));
		msk = wz;
		value = (value | msk);
	}
	void GetMovePoint(WORD& wx, WORD& wz)
	{
		DWORD msk = 0;

		wx = (WORD)(value>>16);
		msk = value<<16;
		wz = (WORD)(msk>>16);
	}	
};


struct MOVEDATA
{
	DWORD		DBIdx;
	char		Name[MAX_SAVEDMOVE_NAME];
	WORD		MapNum;
	stMOVEPOINT Point;
	
	MOVEDATA::MOVEDATA()
	{
		DBIdx = 0;
		memset(Name, 0, MAX_SAVEDMOVE_NAME);
		MapNum = 0;
		Point.value = 0;
	}
};


struct SEND_MOVEDATA_INFO : public MSGBASE
{
	BYTE		bInited;
	WORD		Count;
	MOVEDATA	Data[MAX_MOVEDATA_PERPAGE*MAX_MOVEPOINT_PAGE];

	SEND_MOVEDATA_INFO::SEND_MOVEDATA_INFO()
	{
		bInited = 0;
		Count = 0;
		memset(Data, 0, sizeof(MOVEDATA)*MAX_MOVEDATA_PERPAGE*MAX_MOVEPOINT_PAGE);
	}

	int GetSize()
	{
		return (sizeof(SEND_MOVEDATA_INFO) - sizeof(MOVEDATA)*(MAX_MOVEDATA_PERPAGE*MAX_MOVEPOINT_PAGE-Count));
	}
};


struct SEND_MOVEDATA_SIMPLE : public MSGBASE
{
	MOVEDATA Data;
};


struct SEND_MOVEDATA_WITHITEM : public MSGBASE
{
	MOVEDATA			Data;
	SHOPITEMUSEBASE		Item;
};

struct SEND_GAMEIN_USEITEM : public MSGBASE
{
	DWORD dwData1;
	DWORD dwData2;
	DWORD dwData3;

	DWORD dwPosition;
	DWORD dwItemIdx;
	DWORD dwItemPos;
};



//

struct REVIVEDATA
{
	DWORD TargetID;
	WORD ItemIdx;
	POSTYPE ItemPos;

	REVIVEDATA::REVIVEDATA()
	{
		TargetID = 0;
		ItemIdx = 0;
		ItemPos = 0;
	}
};


struct SEND_REVIVEOTHER : public MSGBASE
{
	DWORD	TargetID;
	char	TargetName[MAX_NAME_LENGTH+1];

	SEND_REVIVEOTHER::SEND_REVIVEOTHER()
	{
		TargetID = 0;
		memset( TargetName, 0, sizeof(char)*(MAX_NAME_LENGTH+1) );
	}
};



struct AVATARITEM
{
	BYTE	Gender;				// 0-����, 1-����, 2-�Ѵ�
	BYTE	Position;
	WORD	Item[eAvatar_Max];

	AVATARITEM::AVATARITEM()
	{
		Gender = 2;
		Position = 0;
		memset( Item, 0, sizeof(WORD)*eAvatar_Max );
		for(int i=eAvatar_Weared_Hair; i<eAvatar_Max; i++)
			Item[i]=1;
	}
};


struct SEND_AVATARITEM_INFO : public MSGBASE
{
	DWORD	PlayerId;
	WORD	ItemIdx;
	WORD	ItemPos;
	WORD	Avatar[eAvatar_Max];

	SEND_AVATARITEM_INFO::SEND_AVATARITEM_INFO()
	{
		PlayerId=0;
		ItemIdx=0;
		ItemPos=0;
		memset( Avatar, 0, sizeof(WORD)*eAvatar_Max );
		for(int i=eAvatar_Weared_Hair; i<eAvatar_Max; i++)
			Avatar[i]=1;
	}
};


struct CHARACTERCHANGE_INFO
{
	BYTE	Gender;
	BYTE	FaceType;
	BYTE	HairType;
	float	Height;
	float	Width;

	CHARACTERCHANGE_INFO::CHARACTERCHANGE_INFO()
	{
		Gender = 0;
		FaceType = 0;
		HairType = 0;
		Height = 0;
		Width = 0;
	}
};

struct SEND_CHARACTERCHANGE_INFO : public MSGBASE
{
	DWORD		Param;
	CHARACTERCHANGE_INFO		Info;

	SEND_CHARACTERCHANGE_INFO::SEND_CHARACTERCHANGE_INFO()
	{
		Param = 0;
	}
};


struct SHOPITEMDUP
{
	DWORD Index;
	DWORD DupType;
	DWORD Param;

	SHOPITEMDUP::SHOPITEMDUP()
	{
		Index = 0;
		DupType = 0;
		Param = 0;
	}
};




// partywar
struct MSG_PW_CREATE : public MSGBASE
{
	DWORD	dwTeam;
	VECTOR3	vPosition;

};




// ��������������������������������������������������������������
// ��Memory Checker                                            ��
// ��������������������������������������������������������������
struct stMemoryInfo
{
	DWORD type;
	DWORD Created;
	DWORD Released;

	int MemorySize;

	stMemoryInfo::stMemoryInfo()
	{
		type=0;
		Created=0;
		Released=0;
		MemorySize=0;
	}
	void stMemoryInfo::Increase( int size )
	{
		++Created;
		MemorySize += size;
	}
	void stMemoryInfo::Decrease( int size )
	{
		++Released;
		MemorySize -= size;
	}
};

enum
{
	eSkillFactory_MakeSkillObject=0,
	eObjectFactory_Player,
	eObjectFactory_Monster,
	eObjectFactory_Npc,
	eObjectFactory_Tactic,
	eObjectFactory_BossMonster,
	ePackedData_Init,

	eMemoryChecker_Max,
};

// ��������������������������������������������������������������
// �� RaMa												       ��
// ��������������������������������������������������������������
struct SHOUTBASE
{
	DWORD CharacterIdx;
	char Msg[MAX_SHOUT_LENGTH+1];
};

struct SHOUTRECEIVE
{
	BYTE Count;
	WORD Time;
	DWORD CharacterIdx;
};

struct SEND_SHOUTBASE : public MSGBASE
{
	BYTE		Count;
	SHOUTBASE	ShoutMsg[3];

	SEND_SHOUTBASE::SEND_SHOUTBASE()
	{
		Count = 0;
		memset( ShoutMsg, 0, sizeof(SHOUTBASE)*3 );
	}
	DWORD SEND_SHOUTBASE::GetSize()
	{
		return (sizeof(SEND_SHOUTBASE) - sizeof(SHOUTBASE)*(3-Count));
	}
};

struct SEND_SHOUTRECEIVE : public MSGBASE
{
	SHOUTRECEIVE	Receive;
};

struct SEND_SHOUTBASE_ITEMINFO : public MSGBASE
{	
	WORD ItemIdx;
	WORD ItemPos;
	char Msg[MAX_SHOUT_LENGTH+1];

	SEND_SHOUTBASE_ITEMINFO::SEND_SHOUTBASE_ITEMINFO()
	{
		ItemIdx = 0;
		ItemPos = 0;
		memset( Msg, 0, sizeof(char)*MAX_SHOUT_LENGTH+1 );
	}
};



struct SEND_CHASEBASE : public MSGBASE
{	
	char	WantedName[MAX_NAME_LENGTH+1];
	DWORD	dwItemIdx;
};

struct SEND_CHASEINFO : public MSGBASE
{
	DWORD CharacterIdx;
	char WandtedName[MAX_NAME_LENGTH+1];
	COMPRESSEDPOS Pos;
	MAPTYPE	MapNum;
	MAPTYPE	EventMapNum;
};

struct SEND_CHANGENAMEBASE : public MSGBASE
{
	char	Name[MAX_NAME_LENGTH+1];
	DWORD	DBIdx;
};



///////////////////////////////////
enum eCheatEvent
{
	eEvent_None=0,
	eEvent_ExpRate,
	eEvent_ItemRate,
	eEvent_MoneyRate,
	eEvent_DamageReciveRate,
	eEvent_DamageRate,
	eEvent_NaeRuykRate,
	eEvent_UngiSpeed,
	eEvent_PartyExpRate,
	eEvent_AbilRate,
	eEvent_GetMoney,
	eEvent_MugongExp,

	eEvent_Max,
};


enum ePartyEvent
{
	ePartyEvent_None=0,
	ePartyEvent_ItemRate,
	ePartyEvent_NaeRyukRate,
	ePartyEvent_MussangCharge,
	ePartyEvent_DefenceRate,
	ePartyEvent_CriticalRate,

	ePartyEvent_Max,
};


struct PARTYEVENT
{
	float	Rate[MAX_PARTY_LISTNUM];
};

struct MSG_GM_MOVE : public MSGBASE
{
	char Name[MAX_NAME_LENGTH+1];
	DWORD dwData1;
	DWORD dwData2;
	DWORD dwGMIndex;
	char strGMID[MAX_NAME_LENGTH+1];
};

struct MSG_EVENTNOTIFY_ON : public MSGBASE
{
	char strTitle[32];
	char strContext[128];
	BYTE EventList[eEvent_Max];
};

struct GAMEEVENT 
{
	void Init(BYTE kind, DWORD data)
	{
		bEventKind = kind;
		dwData = data;
	}

	BYTE bEventKind;			// �̺�Ʈ�� ����
	DWORD dwData;				// ������1
};

enum  eGameEvent1 {
	eGameEvent1_LevelUp,
	eGameEvent1_HeroDie,
	eGameEvent1_SavePoint,
	eGameEvent1_Ungijosik,
	eGameEvent1_GameIn,
	eGameEvent1_MonsterKill,
	eGameEvent1_ObtainItem,
	eGameEvent1_Max,
};

enum  eGameEvent
{
	eGameEvent_CharInterface,
	eGameEvent_InvenInterface,
	eGameEvent_MugongInterface,
	eGameEvent_QuestInterface,
	eGameEvent_MapInterface,
	eGameEvent_MunpaInterface,
	eGameEvent_FriendInterface,
	eGameEvent_NoteInterface,
	eGameEvent_OptionInterface,
	eGameEvent_ExitInterface,
	eGameEvent_PKInterface,
	
	eGameEvent_Hunt,
	eGameEvent_Die,
	eGameEvent_DieAfter5,
	eGameEvent_NpcClick,
	eGameEvent_DieFromPK,
	eGameEvent_KillPK,
	eGameEvent_ObtainItem,
	eGameEvent_ItemSetting,
	eGameEvent_LoginPoint,
	eGameEvent_Mugong0,
	eGameEvent_Mugong1,
	eGameEvent_Munpaunder,//magi82 - ���ϻ� ���������ý���
	
	eGameEvent_Ability,
	eGameEvent_LevelUp,
	eGameEvent_MapChange,

	eGameEvent_Max,
};


// ��������������������������������������������������������������
// �� Guild Tournament									       ��
// ��������������������������������������������������������������

struct stTournamentTime
{
	WORD	Day;
	WORD	Hour;
	WORD	Minute;
};

// from DB
struct GTDBLoadInfo
{
	DWORD	GuildIdx;
	WORD	Ranking;
	WORD	Position;
	WORD	ProcessTournament;
};

struct REGISTEDGUILDINFO
{
	DWORD			GuildIdx;
	char			ImageData[GUILDMARK_BUFSIZE];
	char			GuildName[MAX_GUILD_NAME+1];
	DWORD			MarkExist;
	BYTE			Position;
	BYTE			Ranking;
	BYTE			ProcessTournament;
};

struct GUILDRANKING
{
	DWORD			GuildIdx;
	BYTE			Ranking;
};


struct GTTEAM_MEMBERINFO
{
	DWORD		TeamGuildIdx[2];
	WORD		TeamMember[2];
	WORD		TeamDieMember[2];
};

struct SEND_GTWINLOSE : public MSGBASE
{
	WORD		TournamentCount;
	DWORD		GuildIdx;
	char		GuildName[MAX_GUILD_NAME+1];
	BYTE		bWin;
	DWORD		Param;			// ��� �̱�� ���°�, client-��� ����ΰ�
	BYTE		bUnearnedWin;	// �������ΰ�
};

struct SEND_BATTLEJOIN_INFO : public MSGBASE
{
	DWORD		AgentIdx;
	DWORD		UserLevel;
	DWORD		GuildIdx;
	DWORD		BattleIdx;
	DWORD		ReturnMapNum;
};


struct SEND_BATTLESCORE_INFO : public MSGBASE
{
	char		GuildName[2][MAX_GUILD_NAME+1];
	DWORD		Score[2];
	DWORD		FightTime;
	DWORD		EntranceTime;
	DWORD		State;
	BYTE		Team;

	SEND_BATTLESCORE_INFO::SEND_BATTLESCORE_INFO()
	{
		memset( GuildName[0], 0, sizeof(char)*(MAX_GUILD_NAME+1) );
		memset( GuildName[1], 0, sizeof(char)*(MAX_GUILD_NAME+1) );
		Score[0] = 0;
		Score[1] = 0;
		FightTime = 0;
		EntranceTime = 0;
		State = 0;
		Team = 0;
	}
};


struct SEND_REGISTEDGUILDINFO : public MSGBASE
{
	BYTE				Count;
	BYTE				CurTournament;
	DWORD				PlayerID;
	REGISTEDGUILDINFO	GInfo[MAXGUILD_INTOURNAMENT];

	SEND_REGISTEDGUILDINFO::SEND_REGISTEDGUILDINFO()
	{
		Count = 0;
		CurTournament = 0;
		PlayerID = 0;
		memset( GInfo, 0, sizeof(REGISTEDGUILDINFO)*MAXGUILD_INTOURNAMENT );
	}
	DWORD SEND_REGISTEDGUILDINFO::GetSize()
	{
		return (sizeof(SEND_REGISTEDGUILDINFO) - sizeof(REGISTEDGUILDINFO)*(MAXGUILD_INTOURNAMENT-Count));
	}
};

struct SEND_GUILDRANKING : public MSGBASE
{
	BYTE			Count;
	GUILDRANKING	Ranking[MAXGUILD_INTOURNAMENT];

	SEND_GUILDRANKING::SEND_GUILDRANKING()
	{
		Count = 0;
		memset( Ranking, 0, sizeof(GUILDRANKING)*MAXGUILD_INTOURNAMENT );
	}
	DWORD SEND_GUILDRANKING::GetSize()
	{
		return (sizeof(SEND_GUILDRANKING) - sizeof(GUILDRANKING)*(MAXGUILD_INTOURNAMENT-Count));
	}
};


struct GTBATTLE_INFO
{
	BYTE	Group;		// A, B, C, D
	DWORD	BattleID;
	char	GuildName1[MAX_GUILD_NAME+1];
	char	GuildName2[MAX_GUILD_NAME+1];
};


struct SEND_GTBATTLE_INFO : public MSGBASE
{
	BYTE			Count;
	BYTE			PlayOff;		// ����-1, ����-2
	GTBATTLE_INFO	BattleInfo[MAXGUILD_INTOURNAMENT/2];
	
	SEND_GTBATTLE_INFO::SEND_GTBATTLE_INFO()
	{
		Count = 0;
		memset( BattleInfo, 0, sizeof(GTBATTLE_INFO)*(MAXGUILD_INTOURNAMENT/2) );
	}
	DWORD SEND_GTBATTLE_INFO::GetSize()
	{
		return (sizeof(SEND_GTBATTLE_INFO) - sizeof(GTBATTLE_INFO)*(MAXGUILD_INTOURNAMENT/2-Count));
	}
};


struct GTBATTLE_INFOBROAD
{
	DWORD	BattleID;
	DWORD	Group;			// �����
	DWORD	GuildIdx[2];
};


struct SEND_GTBATTLE_INFOBROAD : public MSGBASE
{
	BYTE					Count;
	DWORD					CurTournament;
	GTBATTLE_INFOBROAD		BattleInfo[MAXGUILD_INTOURNAMENT/2];
	
	SEND_GTBATTLE_INFOBROAD::SEND_GTBATTLE_INFOBROAD()
	{
		Count = 0;
		memset( BattleInfo, 0, sizeof(GTBATTLE_INFOBROAD)*MAXGUILD_INTOURNAMENT/2 );
	}
	DWORD SEND_GTBATTLE_INFOBROAD::GetSize()
	{
		return (sizeof(SEND_GTBATTLE_INFOBROAD) - sizeof(GTBATTLE_INFOBROAD)*(MAXGUILD_INTOURNAMENT/2-Count));
	}
};

// guildunion

#ifdef _CHINA_LOCAL_
#define		MAX_GUILD_UNION_NUM		4
#else
#define		MAX_GUILD_UNION_NUM		7
#endif

struct sGUILDIDXNAME
{
	DWORD	dwGuildIdx;
	char	sGuildName[MAX_GUILD_NAME+1];
};

struct SEND_GUILD_UNION_INFO : public MSGBASE
{
	DWORD			dwGuildUnionIdx;
	char			sGuildUnionName[MAX_GUILD_NAME+1];
	DWORD			dwGuildUnionMarkIdx;
	int				nMaxGuildNum;
	sGUILDIDXNAME	GuildInfo[MAX_GUILD_UNION_NUM];
	
	SEND_GUILD_UNION_INFO()
	{
		dwGuildUnionIdx = 0;
		memset( sGuildUnionName, 0, MAX_GUILD_NAME+1 );
		dwGuildUnionMarkIdx = 0;
		memset( GuildInfo, 0, sizeof(sGUILDIDXNAME)*MAX_GUILD_UNION_NUM );
		nMaxGuildNum = 0;
	}
	void AddGuildInfo( DWORD dwGuildIdx, char* pGuildName )
	{
		GuildInfo[nMaxGuildNum].dwGuildIdx = dwGuildIdx;
		strncpy( GuildInfo[nMaxGuildNum].sGuildName, pGuildName, MAX_GUILD_NAME+1 );
		++nMaxGuildNum;
	}
	int GetSize()
	{
		return ( sizeof(SEND_GUILD_UNION_INFO) - sizeof(sGUILDIDXNAME)*(MAX_GUILD_UNION_NUM-nMaxGuildNum) );
	}
};

struct SEND_CHEAT_ITEM_OPTION : public MSGBASE
{
	SEND_CHEAT_ITEM_OPTION(){memset(this, 0, sizeof(SEND_CHEAT_ITEM_OPTION));}
	POSTYPE bPosition;
	WORD wItemIdx;
	WORD wOptionKind;

	ITEM_OPTION_INFO		OptionInfo;
};


// ��������������������������������������������������������������
// �� SiegeWar											       ��
// ��������������������������������������������������������������

#define SIEGEWAR_MAXGUILDCOUNT_PERTEAM		128
#define SIEGEWAR_MAX_SIEGEMAP				20
#define SIEGEWAR_MAX_AFFECTED_MAP			10
#define CASTLEGATE_NAME_LENGTH				16
enum
{
	eCastleGateLevel_Level0 = 0,
	eCastleGateLevel_Level1,
	eCastleGateLevel_Level2,
	eCastleGateLevel_Level3,
	eCastleGateLevel_Max,
};



struct MAPOBJECT_INFO
{
	DWORD	Life;
	DWORD	MaxLife;
	DWORD	Shield;
	DWORD	MaxShield;
	DWORD	PhyDefence;
	float	Radius;


	ATTRIBUTEREGIST		AttrRegist;
};



struct CASTLEGATE_BASEINFO
{
	DWORD		Index;
	DWORD		MapNum;
	char		Name[CASTLEGATE_NAME_LENGTH+1];
	DWORD		Kind;
	char		DataName[33];
	DWORD		Life[eCastleGateLevel_Max];
	DWORD		Shield[eCastleGateLevel_Max];
	DWORD		Defence;
	ATTRIBUTEREGIST	Regist;
	float		Radius;		
	VECTOR3		Position;
	DWORD		Width;
	DWORD		Wide;
	float		Angle;
	float		Scale;
};



struct stCASTLEGATELEVEL{
	/// 4bit : ����, 5bit : �ε���, 2bit : ����
	DWORD		value;

	stCASTLEGATELEVEL():value(0) {}

	void stCASTLEGATELEVEL::AddGate( DWORD Index, DWORD Level )
	{
		DWORD tmp = value;
		if( Index > 1000 )			Index -= 1000;

		DWORD Count = tmp >> 28;
		DWORD msk = Index;

		// �ε����� ������ �����Ų msk�� �����
		msk = msk << 2;
		msk |= Level;
		msk = msk << (7*(4-(Count+1)));

		// ������ ����Ÿ�� �����Ų��.
		tmp = tmp << 4;
		tmp = tmp >> 4;
		tmp |= msk;

		// count�ϳ� �ø���
		++Count;
		msk = Count;
		msk = msk << 28;		
		tmp |= msk;

		// �ٽ� ����
		value = tmp;
	}
	BOOL stCASTLEGATELEVEL::SetLevel( DWORD Index, DWORD Level )
	{
		DWORD tmp = value;
		if( Index > 1000 )			Index -= 1000;

		DWORD Count = tmp >> 28;
		for(DWORD i=0; i<Count; ++i)
		{
			DWORD t = tmp << (4+(7*i));
			t = t >> 25;
			DWORD data = t;
			
			DWORD idx = data >> 2;			
			if( idx == Index )
			{
				DWORD msk = Level;
				idx = idx << 2;
				msk |= idx;
				msk = msk << (7*((4-i)-1));

				DWORD msk1 = tmp >> 7*(4-i);
				msk1 = msk1 << 7*(4-i);
				DWORD msk2 = tmp << (4+(7*(i+1)));
				msk2 = msk2 >> (4+(7*(i+1)));

				msk1 |= msk2;
				msk |= msk1;

				value = msk;
				return TRUE;
			}
		}
		return FALSE;
	}
	DWORD stCASTLEGATELEVEL::GetLevel( DWORD Index )
	{
		DWORD tmp = value;
		if( Index > 1000 )			Index -= 1000;

		DWORD Count = tmp >> 28;
		for(DWORD i=0; i<Count; ++i)
		{
			DWORD t = tmp << (4+(7*i));
			t = t >> 25;
			DWORD data = t;
			
			DWORD idx = data >> 2;			
			if( idx == Index )
			{
				DWORD level = t << 30;
				level = level >> 30;
				return level;
			}
		}

		return 0;
	}
	void stCASTLEGATELEVEL::SetZeroAllLevel()
	{
		DWORD tmp = value;
		DWORD Count = tmp >> 28;

		for(DWORD i=0; i<Count; ++i)
		{
			DWORD t = tmp << (4+(7*i));
			t = t >> 25;
			DWORD data = t;
			
			DWORD idx = data >> 2;

			DWORD msk = 0;
			idx = idx << 2;
			msk |= idx;
			msk = msk << 7*((4-i)-1);

			DWORD msk1 = tmp >> 7*(4-i);
			msk1 = msk1 << 7*(4-i);
			DWORD msk2 = tmp << (4+(7*(i+1)));
			msk2 = msk2 >> (4+(7*(i+1)));

			msk1 |= msk2;
			msk |= msk1;

			value = msk;		
		}
	}
};


struct CASTLEGATE_SENDINFO
{
	DWORD		Index;
	DWORD		Life;
	DWORD		Shield;
	BYTE		Level;
};

struct SEND_SIEGEWAR_JOININFO : public MSGBASE
{
	DWORD		AgentIdx;
	DWORD		UserLevel;
	DWORD		GuildIdx;
	BYTE		bObserver;
	DWORD		ReturnMapNum;
};

enum SIEGEGUILDTYPE
{
	eSGT_DEFENCEPROPOSALGUILD = 1,
	eSGT_DEFENCEGUILD,
	eSGT_ATTACKGUILD,
};

struct SIEGEWAR_GUILDDBINFO
{
	DWORD	GuildIdx;
	BYTE	Type;				// 1 - CastleGuild, 2 - DefenceGuild, 3 - DefenceProposal, 4 - AttackGuild

	SIEGEWAR_GUILDDBINFO()
	{
		GuildIdx = 0;
		Type = 0;
	}
};

struct SEIGEWAR_CHARADDINFO
{
	BYTE Team;
	BYTE bEngrave;
};

struct SIEGEWAR_GUILDSENDINFO
{
	SIEGEWAR_GUILDDBINFO	Info;
	char					GuildName[MAX_GUILD_NAME+1];

	SIEGEWAR_GUILDSENDINFO()
	{
		memset( GuildName, 0, sizeof(char)*(MAX_GUILD_NAME+1) );
	}
};

struct SEND_SW_PROPOSALGUILDLIST : public MSGBASE
{
	WORD						Count;
	SIEGEWAR_GUILDSENDINFO		GuildList[SIEGEWAR_MAXGUILDCOUNT_PERTEAM];

	SEND_SW_PROPOSALGUILDLIST()
	{
		Count = 0;
		memset( GuildList, 0, sizeof(SIEGEWAR_GUILDSENDINFO)*SIEGEWAR_MAXGUILDCOUNT_PERTEAM );
	}
	int GetSize()
	{
		return ( sizeof(SEND_SW_PROPOSALGUILDLIST) - 
			sizeof(SIEGEWAR_GUILDSENDINFO)*(SIEGEWAR_MAXGUILDCOUNT_PERTEAM-Count) );
	}
};

struct SEND_SW_GUILDLIST : public MSGBASE
{
	WORD						DefenceCount;
	WORD						AttackCount;
	SIEGEWAR_GUILDSENDINFO		GuildList[SIEGEWAR_MAXGUILDCOUNT_PERTEAM*2];

	SEND_SW_GUILDLIST()
	{
		DefenceCount = 0;
		AttackCount = 0;
		memset( GuildList, 0, sizeof(SIEGEWAR_GUILDSENDINFO)*SIEGEWAR_MAXGUILDCOUNT_PERTEAM*2 );
	}
	int GetSize()
	{
		return ( sizeof(SEND_SW_GUILDLIST) - 
			sizeof(SIEGEWAR_GUILDSENDINFO)*(SIEGEWAR_MAXGUILDCOUNT_PERTEAM*2-(DefenceCount+AttackCount)) );
	}
};

struct SEND_AFFECTED_MAPLIST : public MSGBASE
{
	DWORD		Param;
	WORD		Count;
	WORD		MapList[SIEGEWAR_MAX_AFFECTED_MAP];

	SEND_AFFECTED_MAPLIST()
	{
		Param = 0;
		Count = 0;
		memset( MapList, 0, sizeof(WORD)*10 );
	}
	int GetSize()
	{
		return ( sizeof(SEND_AFFECTED_MAPLIST) - sizeof(WORD)*(SIEGEWAR_MAX_AFFECTED_MAP-Count) );
	}
};

struct SEND_SW_BTGUILDLIST : public MSGBASE
{
	WORD		Team;
	WORD		DefenceCount;
	WORD		AttackCount;
	DWORD		GuildList[SIEGEWAR_MAXGUILDCOUNT_PERTEAM*2];

	SEND_SW_BTGUILDLIST()
	{
		Team = 0;
		DefenceCount = 0;
		AttackCount = 0;
		memset( GuildList, 0, sizeof(DWORD)*SIEGEWAR_MAXGUILDCOUNT_PERTEAM*2 );
	}
	int GetSize()
	{
		return ( sizeof(SEND_SW_BTGUILDLIST) - 
			sizeof(DWORD)*(SIEGEWAR_MAXGUILDCOUNT_PERTEAM*2-(DefenceCount+AttackCount)) );
	}
};

struct SEND_SW_SUCCESSINFO : public MSGBASE
{
	char GuildName[MAX_GUILD_NAME+1];
};

struct SEND_SW_ENGRAVE : public MSGBASE
{
	char CharacterName[MAX_NAME_LENGTH+1];
	char GuildName[MAX_GUILD_NAME+1];
};

struct SEND_SW_INITINFO : public MSGBASE
{
	DWORD	Time;
	char	GuildName[MAX_GUILD_NAME+1];
	BYTE	GateCount;
	CASTLEGATE_SENDINFO	GateInfo[5];
	
	SEND_SW_INITINFO()
	{
		GateCount = 0;
		memset( GateInfo, 0, sizeof(CASTLEGATE_SENDINFO)*5 );
	}
	int GetSize()
	{
		return ( sizeof(SEND_SW_INITINFO) - sizeof(CASTLEGATE_SENDINFO)*(5-GateCount) );
	}
};



//----------------------------------
struct MSG_ASSERT : public MSGBASE
{
	DWORD UserID;
	DWORD CharacterID;
	char strAssertMsg[256];
	void SetMsg(DWORD UserID,DWORD CharacterID,char* Msg)
	{
		Category = MP_DEBUG;
		Protocol = MP_DEBUG_CLIENTASSERT;
		this->UserID = UserID;
		this->CharacterID = CharacterID;
		strncpy(strAssertMsg,Msg,255);
		strAssertMsg[255] = 0;
	}
};

struct MSG_JACKPOT_PRIZE_NOTIFY : public MSGBASE
{
	MSG_JACKPOT_PRIZE_NOTIFY():nPrizeKind(-1),dwPrizeUserID(0),dwPrizeMoney(0),dwRestTotalMoney(0){};
	int nPrizeKind;				// prize kind
	DWORD dwPrizeUserID;
	MONEYTYPE dwPrizeMoney;
	MONEYTYPE dwRestTotalMoney;		// DB TotalMoney
	char Name[MAX_NAME_LENGTH+1];	// prize character name
};

//struct MSG_JACKPOT_TOTALMONEY : public MSGBASE
//{
//	MSG_JACKPOT_TOTALMONEY():dwServerTotalMoney(0){};
//	DWORD dwServerTotalMoney;
//};

struct MSG_SW_PROFIT_INFO : public MSGBASE
{
	DWORD	dwGuildIdx;
	char	sGuildName[MAX_GUILD_NAME+1];
	int		nTaxRateForStore;				// 100����
	DWORD	dwTotalMoney;	

	MSG_SW_PROFIT_INFO()
	{
		dwGuildIdx = 0;
		memset( sGuildName, 0, MAX_GUILD_NAME+1 );
		nTaxRateForStore = 0;
		dwTotalMoney = 0;		
	}
};



// RareItem
enum { eRI_LIFE, eRI_NAERYUK, eRI_SHIELD, ValueKindMAX = 3 };
enum { eRI_GENGOL, eRI_MINCHUB, eRI_CHERYUK, eRI_SIMMEK, StatesKindMAX = 4 };
enum eRareItemKind{eRI_GUM,eRI_GWUN,eRI_DOW,eRI_CHANG,eRI_GOONG,eRI_AMGI,eRI_DRESS,eRI_HAT,eRI_SHOES,eRI_RING,eRI_CAPE,eRI_NECKLACE,eRI_ARMLET,eRI_BELT,RareItemKindMAX};

struct sRareOptionBase
{
	sRareOptionBase():BaseMin(0),BaseMax(0){}
	DWORD BaseMin;	//(����)�⺻ �ɼ� �ּҰ� (����:+���������� ��:+������� �Ǽ�����:+����)
	DWORD BaseMax;
};

struct sRareOptionWeaponInfo : public sRareOptionBase
{
	sRareOptionWeaponInfo():AddMin(0),AddMax(0){}
	DWORD AddMin;	//(����)�ΰ� �ɼ� �ּҰ� (����:+������,etc. ��:+�Ӽ����׷� �Ǽ�����:����)
	DWORD AddMax;
};

struct sRareOptionProtectorInfo : public sRareOptionBase
{
	sRareOptionProtectorInfo():AttribMin(0),AttribMax(0){}
	DWORD AttribMin;
	DWORD AttribMax;
	DWORD PlusValue[ValueKindMAX];
};

struct sRareItemInfo
{
	sRareItemInfo() : ItemIdx(0),RareProb(0){}
	DWORD ItemIdx;
	DWORD RareProb;
};

struct sRareOptionInfo : public ITEM_RARE_OPTION_INFO
{
	sRareOptionInfo() { memset(this, 0, sizeof(sRareOptionInfo)); }

	WORD	GenGol_Min;
	WORD	MinChub_Min;
	WORD	CheRyuk_Min;
	WORD	SimMek_Min;
	DWORD	Life_Min;
	WORD	NaeRyuk_Min;
	DWORD	Shield_Min;
	WORD	PhyAttack_Min;
	//	WORD	CriticalPercent_Min;
	WORD	PhyDefense_Min;
	ATTRIBUTEREGIST AttrRegist_Min;
	ATTRIBUTEATTACK AttrAttack_Min;
};

struct MSG_SVVMODE_ENDINFO : public MSGBASE
{
	MSG_SVVMODE_ENDINFO() { memset(this, 0, sizeof(MSG_SVVMODE_ENDINFO)); }
	DWORD	WinnerID;
	char WinnerName[MAX_NAME_LENGTH+1];
};

struct MSG_SINGLE_SPECIAL_STATE_NACK : public MSGBASE
{
	WORD	State;
	WORD	ErrCode;
};
struct MSG_SINGLE_SPECIAL_STATE : public MSGBASE
{
	DWORD	Operator;
	WORD	State;
	bool	bVal;
	bool	Forced;

	MSG_SINGLE_SPECIAL_STATE()
	{
		Operator = 0;
		State = 0;
		bVal = 0;
		Forced = 0;
	}
};

// magi82 //////////////////////////////////////////////////////////////////////////
// ������ ���� ����ġ �г�Ƽ�� �ٸ� ����ü
struct LEV_PENALTY
{
	float	fNow;
	float	fSave;
};
/////////////////////////////////////////////////////////////////////////////////////

//magi82 - ���ϻ� ���������ý��� ////////////////////////////////////////////////////

// DB�������� �޾Ƽ� ������ �ִ� ���ļҰ���� ����ü
struct GUILD_TRAINEE_GUILDLIST
{
	DWORD dwGuildIdx;
	DWORD dwRank;
};

struct MSG_GUILD_TRAINEE_GUILDLIST : public MSGBASE
{
	GUILD_TRAINEE_GUILDLIST sTraineeGuildList;
};

// DB�������� �޾Ƽ� ���ϻ� �����ڿ� ���� ����ü
struct GUILD_TRAINEE_LIST
{
	DWORD dwGuildIdx;
	DWORD dwUserIdx;
	char strName[MAX_NAME_LENGTH+1];
	DWORD dwTime;
};

struct MSG_GUILD_TRAINEE_LIST : public MSGBASE
{
	GUILD_TRAINEE_LIST sTraineeList;
};

// Ŭ���̾�Ʈ���� �����ֱ� ���� ���ļҰ�â�� ���� ����ü
struct MUNHA_INFO
{
	int nMunpaIdx;
//	int nMunpaRank;
	int nMunhaCount;
	int nMunhaGuildMark;
	char strName[MAX_GUILD_NAME+1];
	char strIntro[MAX_GUILD_INTRO+1];
};

struct MSG_MUNHAINFO : public MSGBASE
{
	WORD Index;
	WORD MaxCount;
	MUNHA_INFO sMunHa_Info[4];
};

// ���� �Ұ��� DB���� �޾Ƽ� Map ������ �����ϴ� ����ü
struct GUILD_INTRO_INFO
{
	DWORD dwGuildIdx;
	char strIntro[MAX_GUILD_INTRO+1];
};

// ���� �Ұ� ������Ʈ�� ���� ����ü
struct MSG_MUNHAINFOUPDATE : public MSGBASE
{
	DWORD Index;
	char Intro[MAX_GUILD_INTRO+1];
};

// ���ϻ������ڸ� Ŭ���̾�Ʈ���� �����ֱ� ���� ����ü
struct MUNHACATALOG
{
    DWORD dwMunha_Index;
	char strName[MAX_NAME_LENGTH+1];
};

struct MSG_MUNHACATALOG : public MSGBASE
{
	DWORD dwCnt;
	MUNHACATALOG Value[50];


	MSG_MUNHACATALOG::MSG_MUNHACATALOG()
	{
		dwCnt = 0;
		memset( Value, 0, sizeof(MUNHACATALOG)*50 );
	}
	int MSG_MUNHACATALOG::GetSize()
	{
        return (sizeof(MSG_MUNHACATALOG) - (sizeof(MUNHACATALOG)*(50-dwCnt)));
	}
};

// 24�ð����� ���û������ ���� �޼��� ����
struct MSG_MUNPAJOINERROR : public MSGBASE
{
	DWORD errorCode;
	DWORD errorTime;
	char strName[MAX_GUILD_NAME+1];
};

// magi82 - ���ϻ�����(070125)
// ���ϻ������ؼ� ���������� ������Ʈ�� ������ �޼���
struct MSG_MUNHANOTEINFO : public MSGBASE
{
	DWORD dwData;
	char Intro[MAX_CHAT_LENGTH+1];
};
struct MSG_MUNHA_NAME2_DWORD_NOTE : public MSGBASE
{
	char	Name1[MAX_NAME_LENGTH+1];
	char	Name2[MAX_NAME_LENGTH+1];
	DWORD	dwData;
	char Intro[MAX_CHAT_LENGTH+1];
};
////////////////////////////////////////////////////////////////////////////////////

// magi82 - Titan(070130) //////////////////////////////////////////////////////////////////
struct TITANMIX_ICONINFO
{
	DWORD dwItemIdx;
	POSTYPE itempos;
};

struct MSG_TITANMIX : public MSGBASE
{
	WORD wTitanScale;
	TITANMIX_ICONINFO iconInfo[4];
};
////////////////////////////////////////////////////////////////////////////////////////////

// magi82 - Titan(070201) //////////////////////////////////////////////////////////////////
struct MSG_TITAN_UPGRADE_SYN : public MSGBASE
{
	WORD	wBasicItemIdx;
	WORD	wBasicItemPos;
	WORD	wMaterialNum;
	DWORD	dwMoney;
	MATERIAL_ARRAY Material[MAX_MIX_MATERIAL];
	void Init()
	{
		wBasicItemIdx	= 0;
		wBasicItemPos	= 0;
		wMaterialNum	= 0;
		dwMoney			= 0;
	}
	void AddMaterial(WORD wItemIdx, POSTYPE ItemPos, DURTYPE Dur)
	{
		Material[wMaterialNum].wItemIdx = wItemIdx;
		Material[wMaterialNum].ItemPos = ItemPos;
		Material[wMaterialNum++].Dur = Dur;
	}
	int GetSize()
	{
		return sizeof(MSG_TITAN_UPGRADE_SYN) - (MAX_MIX_MATERIAL - wMaterialNum)*sizeof(MATERIAL_ARRAY);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////

// magi82 - Titan(070206)
struct TITAN_BREAK_MATERIAL_ARRAY
{
	DWORD	dwItemIdx;
	DURTYPE	wMaterialDur;
};

struct TITAN_BREAK_ACK
{
	WORD	wMaterialNum;
	TITAN_BREAK_MATERIAL_ARRAY sMaterial[MAX_TITANBREAK_MATERIAL];

	TITAN_BREAK_ACK::TITAN_BREAK_ACK()
	{
		ZeroMemory( sMaterial, sizeof(TITAN_BREAK_MATERIAL_ARRAY) );
		wMaterialNum	= 0;
	}
	void AddMaterial(WORD wItemIdx, POSTYPE ItemPos, DURTYPE Dur)
	{
        sMaterial[wMaterialNum].dwItemIdx = wItemIdx;
		sMaterial[wMaterialNum++].wMaterialDur = Dur;
	}
	int GetSize()
	{
		return sizeof(TITAN_BREAK_ACK) - (MAX_TITANBREAK_MATERIAL - wMaterialNum)*sizeof(TITAN_BREAK_MATERIAL_ARRAY);
	}
};
//////////////////////////////////////////////////////////////////////////

// magi82 - Titan(070515) //////////////////////////////////////////////////////////////////
struct MSG_TITAN_REPAIR_TOTAL_EQUIPITEM_ACK : public MSGBASE
{
	WORD	wRepairNum;
	TITAN_ENDURANCE_ITEMINFO RepairInfo[MAX_TITAN_EQUIPITEM_NUM];
	void Init()
	{
		wRepairNum	= 0;
		ZeroMemory(RepairInfo, sizeof(RepairInfo));
	}
	void AddRepairInfo(TITAN_ENDURANCE_ITEMINFO* info)
	{
		CopyMemory(&RepairInfo[wRepairNum], info, sizeof(TITAN_ENDURANCE_ITEMINFO));
		wRepairNum++;
	}
	int GetSize()
	{
		return sizeof(MSG_TITAN_REPAIR_TOTAL_EQUIPITEM_ACK) - (MAX_TITAN_EQUIPITEM_NUM - wRepairNum)*sizeof(TITAN_ENDURANCE_ITEMINFO);
	}
};
////////////////////////////////////////////////////////////////////////////////////////////

// magi82 - Titan(070518) ////////////////////////////////////////////////
struct MSG_TITAN_REPAIR_EQUIPITEM_SYN : public MSGBASE
{
	TITAN_ENDURANCE_ITEMINFO RepairInfo;
};
//////////////////////////////////////////////////////////////////////////

// magi82 - Titan(070619) - ���� ����
struct TITAN_REPAIR_INFO_ARRAY
{
	DWORD	dwItemIdx;
	WORD	dwItemPos;
};

struct MSG_TITAN_REPAIR_TOTAL_EQUIPITEM_SYN : public MSGBASE
{
	WORD	wRepairNum;
	TITAN_REPAIR_INFO_ARRAY	sRepairInfo[MAX_TITAN_EQUIPITEM_NUM];
	
	void Init()
	{
		wRepairNum = 0;
		ZeroMemory(sRepairInfo, sizeof(sRepairInfo));
	}

	void AddRepairInfo( DWORD idx, WORD pos )
	{
		sRepairInfo[wRepairNum].dwItemIdx = idx;
		sRepairInfo[wRepairNum].dwItemPos = pos;

		wRepairNum++;
	}

    DWORD GetSize()
	{
		return sizeof(MSG_TITAN_REPAIR_TOTAL_EQUIPITEM_SYN) - (MAX_TITAN_EQUIPITEM_NUM - wRepairNum) * sizeof(TITAN_REPAIR_INFO_ARRAY);
	}
};
//////////////////////////////////////////////////////////////////////////
//


// RaMa - �������ľ��� ����
struct PROCESSINFO
{
	DWORD	ServerProcesstime[5];
	WORD	DBQueryCount;
    DWORD	GameQueryReturntime;
	DWORD	LogQueryReturntime;
};

struct MSG_PROSESSINFO : public MSGBASE
{
	BYTE			Count;
	DWORD			StarTime;
	PROCESSINFO		ProcessInfo;	
};

// magi82 - UniqueItem(070710)
struct MSG_UNIQUEITEM_MIX_SYN : public MSGBASE
{
	WORD	wBasicItemIdx;
	POSTYPE wBasicItemPos;
	WORD	wMaterialNum;
	MATERIAL_ARRAY Material[MAX_MIX_MATERIAL];
	void Init()
	{
		wBasicItemIdx = 0;
		wBasicItemPos = 0;
		wMaterialNum = 0;
	}
	void AddMaterial(WORD wItemIdx, POSTYPE ItemPos, DURTYPE Dur)
	{
		Material[wMaterialNum].wItemIdx = wItemIdx;
		Material[wMaterialNum].ItemPos = ItemPos;
		Material[wMaterialNum++].Dur = Dur;
	}
	int GetSize()
	{
		return sizeof(MSG_UNIQUEITEM_MIX_SYN) - (MAX_MIX_MATERIAL - wMaterialNum)*sizeof(MATERIAL_ARRAY);
	}
};

//SW070626 ���λ�NPC
struct BOBUSANGINFO
{
	BOBUSANGINFO() : AppearanceMapNum(0), AppearanceChannel(0), AppearanceTime(0), DisappearanceTime(0), AppearancePosIdx(0), SellingListIndex(0) {};
	DWORD	AppearanceMapNum;
	DWORD	AppearanceChannel;
	DWORD	AppearanceTime;
	DWORD	DisappearanceTime;
	DWORD	AppearancePosIdx;
	DWORD	SellingListIndex;
};

struct MSG_BOBUSANG_INFO : public MSGBASE
{
	BOBUSANGINFO bobusangInfo;	
};

struct MSG_BOBUSANG_INFO_REQUEST : public MSG_BOBUSANG_INFO
{
	int appearanceState;
	BOBUSANGINFO bobusangInfo[2];
};

struct BOBUSANG_ITEM
{
	DWORD tabNum;
	DWORD itemIdx;
	DWORD restNum;
};

struct MSG_ADDABLE_ONEKIND : public MSGBASE
{
	MSG_ADDABLE_ONEKIND() : DataNum(0) {};
	DWORD DataNum;
	CAddableInfoList AddableInfo;

	WORD GetSize()
	{
		return sizeof( MSG_ADDABLE_ONEKIND ) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();
	}
};

// magi82(26)
struct TITAN_SHOPITEM_OPTION
{
	TITAN_SHOPITEM_OPTION::TITAN_SHOPITEM_OPTION()
	{
		dwSummonReduceTime	= 0;
		dwRecallReduceRate	= 0;
		dwEPReduceRate		= 0;
	}

	DWORD	dwSummonReduceTime;
	float	dwRecallReduceRate;
	float	dwEPReduceRate;
};

struct stMAPKINDINFO 
{
	DWORD	dwMapNum;
	char	strMapName[128];
	DWORD	dwMapStateBItFlag;
};

//2008. 1. 22. CBH - ��Ų ���� �޼���
struct SEND_SKIN_INFO : public MSGBASE
{	
	WORD	wSkinItem[eSkinItem_Max];

	SEND_SKIN_INFO::SEND_SKIN_INFO()
	{		
		memset( wSkinItem, 0, sizeof(WORD)*eSkinItem_Max );		
	}
};

// magi82 ������ �˻� -> �ش� �������� �Ĵ� �������� ���� ����ü
struct STREETSTALL_PRICE_INFO
{
	DWORD	dwOwnerIdx;					// ������ �ε���
	char	strName[MAX_NAME_LENGTH];	// ������ �̸�
	DWORD	dwPrice;					// �ش� ������ ����
};

// magi82 ������ �˻� -> ������ �˻� ���� �޼��� ����ü
#define MAX_STALLITEM_NUM	500			// ������ �˻� �ִ� ����
struct SEND_STREETSTALL_INFO : public MSGBASE
{
    WORD					wCount;						// �˻��� ������ ��
	STREETSTALL_PRICE_INFO	sInfo[MAX_STALLITEM_NUM];	// ������ ���� �迭

	int GetSize()
	{
		return sizeof(SEND_STREETSTALL_INFO) - ( ( MAX_STALLITEM_NUM - wCount ) * sizeof( STREETSTALL_PRICE_INFO ) );
	}
};

// magi82 ������ �˻� -> �ش� �������� �Ĵ� �������� ���� ����ü
struct STREETSTALL_ITEM_INFO
{
	DWORD	dwItemIdx;
	DWORD	dwPrice;
	DWORD	dwDur;		// magi82 ������ �˻� -> �ɼǾ������� �ε����� �̹� WORD���� �Ѿ������ DWORD������ ����
	DWORD	dwDBIdx;
	DWORD	dwRareIdx;
};

// magi82 ������ �˻� -> �ش� �������� �Ĵ� �������� ���� ���̴� �޼��� ����ü
struct MSG_STREETSTALL_ITEMVIEW : public MSGBASE
{
	WORD					wCount;
	WORD					wOptioncount;
	WORD					wRareCount;
	WORD					wPetItemCount;
	WORD					wTitanItemCount;
	WORD					wTitanEquipItemCount;
	STREETSTALL_ITEM_INFO	sInfo[SLOT_STREETSTALL_NUM];

	CAddableInfoList		AddableInfo;

	WORD GetMsgSize()
	{
		return sizeof(MSG_STREETSTALL_ITEMVIEW) - sizeof(CAddableInfoList) + AddableInfo.GetInfoLength();
	}
};

// magi82 ������ �˻� -> ������ ��� ����ü
struct STALL_DEALER_INFO
{
	DWORD	dwCharIdx;
	DWORD	dwPrice;
};

// magi82 ������ �˻� -> �������� �Ĵ� �����۵��� ����ü
struct STALL_DEALITEM_INFO
{
	DWORD ItemIdx;
	CYHHashTable<STALL_DEALER_INFO> Info;
};

// autonote
struct MSG_AUTONOTE_IMAGE : public MSGBASE
{
	DWORD	dwReplyTime;		// �亯�ð�(��)
	BYTE	Image[128*32*3];
};

struct MSG_AUTOLIST_ADD : public MSGBASE
{
	char Name[MAX_NAME_LENGTH+1];
	char Date[20];		//2008.01.25 00:00:00  (19����)
};

struct MSG_AUTOLIST_ALL : public MSGBASE
{
	struct AUTOLIST_ROW
	{
		char Name[MAX_NAME_LENGTH+1];
		char Date[20];
	};

	int nCount;
	AUTOLIST_ROW	row[100];

	int	GetMsgLength()
	{
		return sizeof(MSG_AUTOLIST_ALL) - ( sizeof(AUTOLIST_ROW) * ( 100-nCount ) );
	}
};

struct MSG_AUTOUSER_BLOCK : public MSGBASE
{
	DWORD dwAskUserIdx;
	DWORD dwAskCharacterIdx;
	char AskCharacterName[MAX_NAME_LENGTH+1];
	DWORD dwAutoUserIdx;
	DWORD dwAutoCharacterIdx;
	char AutoCharacterName[MAX_NAME_LENGTH+1];
};

//2008. 5. 28. CBH - ��Ƽ ��Ī���� ���� �޼��� ����ü
#define MAX_PARTYMATCHING_INFO_NUM	500			// ��Ƽ���� �ִ� ����
struct MSG_PARTYMATCHING_INFO : public MSGBASE
{
	WORD					wCount;		
	PARTY_MATCHINGLIST_INFO				PartyMatchingInfo[MAX_PARTYMATCHING_INFO_NUM];

	MSG_PARTYMATCHING_INFO()
	{
		wCount = 0;
		memset(PartyMatchingInfo, 0, sizeof(PartyMatchingInfo));
	}

	DWORD GetMsgSize()
	{
		return sizeof(MSG_PARTYMATCHING_INFO) - ( ( MAX_PARTYMATCHING_INFO_NUM - wCount ) * sizeof(PARTY_MATCHINGLIST_INFO) );
	}
};

//2008. 6. 18. CBH - +10 �̻� �������� ����Ʈ ���� �ִ� ����(���� +10 �����Ƹ� ����)
#define MAX_PLUSITEM_EFFECT_NUM	1

//2008. 6. 18. CBH - +10 �̻� ������ ����Ʈ ���� ���� �������� ��� ���
enum PLUSITEM_EFFECT_KIND
{
	ePLUSITEM_EFFECT_10 = 10,	//+10 ������ ����Ʈ
};

//2008. 6. 18. CBH - +10 �̻� ������ ����Ʈ ���� ���� ����ü
struct PLUSITEM_EFFECT_INFO
{	
	WORD wItemType;	//������ Ÿ��	
	char szCHXName[MAX_PLUSITEM_EFFECT_NUM][MAX_CHXNAME_LENGTH+1];	//����Ʈ CHX �̸�

	PLUSITEM_EFFECT_INFO()
	{		
		wItemType = 0;	//������ Ÿ��		
		memset(szCHXName, 0, sizeof(szCHXName));		
	}
};

// �����
struct MSG_FORTWAR_START : public MSGBASE
{
	WORD	wForWarMapNum;
	DWORD	dwChannelNum;
	DWORD	dwFortWarTime;		//��
	char	EngraveSuccessPlayerName[MAX_NAME_LENGTH+1];
	VECTOR2	vNpcPos;
};

struct MSG_FORTWAR_END : public MSGBASE
{
	WORD	wForWarMapNum;
	DWORD	dwChannelNum;
	char	EngraveSuccessPlayerName[MAX_NAME_LENGTH+1];
};

struct MSG_FORTWAR_WAREHOUSE_INFO : public MSGBASE
{
	int			nFortWarIDX;
	MONEYTYPE	dwMoney;
	int			nItemCount;
	ITEMBASE	WarehouseItem[SLOT_SEIGEFORTWARE_NUM];

	MSG_FORTWAR_WAREHOUSE_INFO()
	{
		nFortWarIDX = 0;
		dwMoney = 0;
		nItemCount = 0;
		memset( WarehouseItem, 0, sizeof(ITEMBASE)*SLOT_SEIGEFORTWARE_NUM );
	}
	
	WORD GetSize()
	{
		return sizeof(MSG_FORTWAR_WAREHOUSE_INFO) - (sizeof(ITEMBASE)*(SLOT_SEIGEFORTWARE_NUM-nItemCount));
	}
	void AddItem( ITEMBASE* pInfo )
	{
		WarehouseItem[nItemCount] = *pInfo;
		++nItemCount;
	}
};

struct MSG_FORTWAR_WAREITEM_INFO : public MSGBASE
{
	int			nFortWarIDX;
	ITEMBASE	ItemInfo;
	WORD		wFromPos;
	WORD		wToPos;

	MSG_FORTWAR_WAREITEM_INFO()
	{
		memset( this, 0, sizeof(MSG_FORTWAR_WAREITEM_INFO) );
	}
};


#pragma pack(pop)
#endif //__COMMONSTRUCT_H__

#ifndef __COMMONGAMEDEFINE_H__
#define __COMMONGAMEDEFINE_H__

// Includes standard library headers (for memset, strlen, sprintf, etc.)
#include <cstring>
#include <cstdio>

// Note: BasicTypes.h is already included by CommonDefine.h
// to avoid duplicate inclusion conflicts

enum TACTIC_FAILED_REASON
{
	eTFR_Naeryuk,
	eTFR_MemberDie,
	eTFR_TimeOver,
};

//for GMPower
enum eGM_POWER
{
	eGM_POWER_MASTER = 0,
	eGM_POWER_MONITOR,
	eGM_POWER_PATROLLER,
	eGM_POWER_AUDITOR,
	eGM_POWER_EVENTER,
	eGM_POWER_MAX,
};

//for Network

enum eTRAFFIC_KIND
{
	eTraffic_User_To_Agent			= 1,
	eTraffic_Agent_To_User			= 2,
	
	eTraffic_Agent_To_Map			= 3,
	eTraffic_Map_To_Agent			= 4,
	eTraffic_Agent_to_Agent			= 5,
	eTraffic_Map_to_Map				= 6,
	eTraffic_User_To_Distribute		= 7,
	eTraffic_Distribute_To_User		= 8,
};


//for Exit
#define EXIT_COUNT	5
enum eEXITCODE
{
	eEXITCODE_OK,
	eEXITCODE_PKMODE,
	eEXITCODE_LOOTING,
	eEXITCODE_SPEEDHACK,
	eEXITCODE_DIEONEXIT,
	eEXITCODE_NOT_ALLOW_STATE,
};
//

#define SPEEDHACK_CHECKTIME	60000	//1���ע������ע�E

//for PK
#define PKLOOTING_ITEM_NUM			20
#define PKLOOTING_DLG_DELAY_TIME	2000
#define PKLOOTING_LIMIT_TIME		10000

enum ePKCODE
{
	ePKCODE_OK,
	ePKCODE_ALREADYPKMODEON,
	ePKCODE_VIMUING,
	ePKCODE_STATECONFLICT,
	ePKCODE_NOTALLAW,
};

enum eLOOTINGERROR
{
	eLOOTINGERROR_OK,
	eLOOTINGERROR_NOMORE_CHANCE,
	eLOOTINGERROR_NOMORE_ITEMLOOTCOUNT,
	eLOOTINGERROR_INVALID_POSITION,
	eLOOTINGERROR_ALREADY_SELECTED,
	eLOOTINGERROR_OVER_DISTANCE,
	eLOOTINGERROR_NO_LOOTINGROOM,
};

//

#define NPC_TALKING_DISTANCE		700
#define PK_LOOTING_DISTANCE			1000

enum eDROPITEMKIND
{
	eNOITEM,
	eMONEY,
	eITEM1,
	eITEM2,
	eITEM3,

	eDROPITEMKIND_MAX,
};

#define MAX_DISSOLVEGRID_NUM		20
#define ITEM_DISSOLVE_PRICE_VALUE	200
enum eREINFORCEJEWELPOWER
{
	eRJP_GENGOL=1,		//	�ˢ硧uU�ˢ硧��n+
	eRJP_MINCHUB,		//	��IoIA��E����+
	eRJP_CHERYUK,		//	A����u�ˢ�ˢ�A+
	eRJP_SIMMEK,		//	����oE��E�������סˢ�+
	eRJP_LIFE,			//	AO��E��Ie�ˢ�iy��E����i�ˢ�ˢ�A+
	eRJP_NAERYUK,		//	AO��E��Ie��I�Ϩ��ˢ�i�ˢ�ˢ�A+
	eRJP_SHIELD,		//	E�ˢ�I����oA�ˢ硧���ˢ硧I�ˢ硧ua+

#ifdef _JAPAN_LOCAL_
	eRJP_FIREREGIST,	//	E�ˢ硧I ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
	eRJP_EARTHREGIST,	//	Aa ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
	eRJP_GOLDREGIST,	//	�ˢ硧uY ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
	eRJP_WATERREGIST,	//	����uo ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
	eRJP_TREEREGIST,	//	��E����n ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
#else
	eRJP_FIREREGIST,	//	E�ˢ硧I ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
	eRJP_WATERREGIST,	//	����uo ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
	eRJP_TREEREGIST,	//	��E����n ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
	eRJP_GOLDREGIST,	//	�ˢ硧uY ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
	eRJP_EARTHREGIST,	//	Aa ����uO����u���ע��� AuC�ˢ��?�ˢ�ˢ�A
#endif

	eRJP_PHYATTACK,		//	��Io�ˢ硧����E������Ec�ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+
	eRJP_CRITICAL,		//	A����I��E������Ec���סˢ碮��uAA+

#ifdef _JAPAN_LOCAL_
	eRJP_FIREATTACK,	//	E�ˢ硧I����uO����u���ע����ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
	eRJP_EARTHATTACK,	//	Aa�ˢ硧����I���̡ˢ硧��Y�ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
	eRJP_GOLDATTACK,	//	�ˢ硧uY����uO����u���ע����ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
	eRJP_WATERATTACK,	//	����uo����uO����u���ע����ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
	eRJP_TREEATTACK,	//	��E����n����uO����u���ע����ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
#else
	eRJP_FIREATTACK,	//	E�ˢ硧I����uO����u���ע����ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
	eRJP_WATERATTACK,	//	����uo����uO����u���ע����ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
	eRJP_TREEATTACK,	//	��E����n����uO����u���ע����ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
	eRJP_GOLDATTACK,	//	�ˢ硧uY����uO����u���ע����ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
	eRJP_EARTHATTACK,	//	Aa�ˢ硧����I���̡ˢ硧��Y�ˢ硧����I���̡ˢ硧��Y�ˢ�ˢ�A+%
#endif

	eRJP_PHYDEFENSE,	//	��Io�ˢ硧����E������Ec ��Ii��o|i��������ui�ˢ�ˢ�A+ 
	eRJP_MAX = eRJP_PHYDEFENSE,
};

enum
{
	eUSERLEVEL_GOD = 1,
	eUSERLEVEL_PROGRAMMER,
	eUSERLEVEL_DEVELOPER,
	eUSERLEVEL_GM,
	eUSERLEVEL_SUPERUSER,
	eUSERLEVEL_USER,
	eUSERLEVEL_SUBUSER,	
};

#define MAX_ITEMBUY_NUM		50	//CN��Ii��o|i������E?��E��c �ˢ�ii����uo AO��E��IA AO��E��Ie ����u���סˢ�AIAU �ˢ硧����I�Ϩ�����uo
#define MAX_INVENTORY_MONEY	4000000000
#define MAX_MIX_MATERIAL	128
#define MAX_TITANBREAK_MATERIAL	20
#define MAX_TITAN_EQUIPITEM_NUM	40	// magi82 - Titan(070515) - �κ��丮 + Ÿ��ź�����κ�

#define PKMODETIME	60000			//1���ע������ע�E : AO����oA 10AE


enum eSTATUSICON
{
	eStatusIcon_FireContinueDamage = 1,
	eStatusIcon_WaterContinueDamage,

	eStatusIcon_SpeedDown,
	
	eStatusIcon_TieUpCanMove,
	eStatusIcon_TieUpCanAttack,
	eStatusIcon_TieUpAll,

	eStatusIcon_HealContinue,

	eStatusIcon_PhyAttackUp,
	eStatusIcon_PhyDefenceUp,

	eStatusIcon_MaxLifeUp,	//10
	eStatusIcon_MaxNaeryukUp,

	eStatusIcon_PhyCounter,

	eStatusIcon_AttrDefenceFire,
	eStatusIcon_AttrDefenceWater,
	eStatusIcon_AttrDefenceTree,
	eStatusIcon_AttrDefenceIron,
	eStatusIcon_AttrDefenceEarth,
				
	eStatusIcon_MaxShieldUp,			//eStatusIcon_AttrDefenceAll,	// ����u���סˢ�AIAU ����u��I���̢���ui����u�ˢ硧I ��E��IeA����u
#ifdef _JAPAN_LOCAL_
	eStatusIcon_AttrCounterFire,
	eStatusIcon_AttrCounterEarth,
	eStatusIcon_AttrCounterIron,
	eStatusIcon_AttrCounterWater,
	eStatusIcon_AttrCounterTree,
	eStatusIcon_AttrCounterAll,
#else
	eStatusIcon_AttrCounterFire,
	eStatusIcon_AttrCounterWater,	//20			
	eStatusIcon_AttrCounterTree,
	eStatusIcon_AttrCounterIron,
	eStatusIcon_AttrCounterEarth,
	eStatusIcon_AttrCounterAll,
#endif
	eStatusIcon_Immune,

	eStatusIcon_AmpliedPower,
	eStatusIcon_PoisonContinueDamage,
	eStatusIcon_IronContinueDamage,

	eStatusIcon_PhyAttackDown,
	eStatusIcon_PhyDefenseDown,	//30
	eStatusIcon_MaxLifeDown,
	eStatusIcon_MaxNaeryukDown,
	eStatusIcon_MaxShieldDown,
	
	eStatusIcon_DamageDown	= 34,
	eStatusIcon_AttackUp	= 35,
	eStatusIcon_ContinueRecover	= 36,
	eStatusIcon_ReverseVampiricNaeryuk	= 37,
	eStatusIcon_AttackPhyLastUp			= 39,
	eStatusIcon_AttackAttLastUp			= 40,
	eStatusIcon_ReverseVampiricLife		= 45,

	eStatusIcon_MussangMode				= 48,
	eStatusIcon_PartyHelp				= 49,

	// ShopItem�� Icon�߰�
	eStatusIcon_ShopItem_55101 = 50,
	eStatusIcon_ShopItem_55102 = 51,
	eStatusIcon_ShopItem_55103 = 52,
	eStatusIcon_ShopItem_55104 = 53,
	eStatusIcon_ShopItem_55105 = 54,
	eStatusIcon_ShopItem_55106 = 55,
	eStatusIcon_ShopItem_55107 = 56,
	eStatusIcon_ShopItem_55108 = 57,

	eStatusIcon_ShopItem_55311 = 58,
	eStatusIcon_ShopItem_55312 = 59,
	
	eStatusIcon_ShopItem_55322 = 60,	

	////////////////////////////////////////////////////////
	//06. 06 2�� ���� - �̿���
	//����Ʈ ����(����)
	eStatusIcon_SkipEffect	= 133,
	////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////
	//06. 06 2�� ���� - �̿���
	//����/����
	eStatusIcon_Hide = 134,
	eStatusIcon_Detect = 135,
	////////////////////////////////////////////////////////
	eStatusIcon_QuestTime				= 127,		// quest time

	 
	eStatusIcon_GuildPlustime_SuRyun = 140,
	eStatusIcon_GuildPlustime_MuGong = 141,
	eStatusIcon_GuildPlustime_Exp = 142,
	eStatusIcon_GuildPlustime_DamageUp = 143,

	// magi82(23)
	eStatusIcon_Titan_General = 165,
	eStatusIcon_Titan_Fantasy = 166,

	eStatusIcon_Max = 200,
};


enum eBOSSMONSTERKIND
{
	eBOSSKIND_EMPERIORTOMB = 151,
	eBOSSKIND_EMPERIORTOMB_EVENT = 158,

	eBOSSKIND_MOOUN = 181,
	eBOSSKIND_COWF = 182,
	eBOSSKIND_COWL = 183,
	eBOSSKIND_COWKING = 184,

	eBOSSKIND_GENERAL=1001,
	eBOSSKIND_YUKJI=1002,
	eBOSSKIND_MAX,
};
enum eBOSSMONSTERTYPE
{
	eBOSSTYPE_EMPERIORTOMB,
	eBOSSTYPE_MOOUN,
	eBOSSTYPE_COWF,
	eBOSSTYPE_COWL,
	eBOSSTYPE_COWKING,
	eBOSSTYPE_GENERAL,
	eBOSSTYPE_YUKJI,
	eBOSSTYPE_MAX,
};
#define RECALL_GROUP_ID			-1
#define EVENTRECALL_GROUP_ID	-2
#define QUESTRECALL_GROUP_ID	-3
enum eOPTION	//����u�ˢ硧I��Ioo��E?I ��Ii��E?AICI�ˢ硧��O ��E����AA��I��������oA��E?A.
{
	eOPT_NULL		= 0,
	eOPT_NODEAL		= 1,
	eOPT_NOPARTY	= 2,
	eOPT_NOFRIEND	= 4,
	eOPT_NOVIMU		= 8,
	eOPT_NOWHISPER	= 16,
	eOPT_NOAVATARVIEW = 32,
};



//---murim net
#define MAXPLAYER_IN_MURIMNET	50000
#define MAXPLAYER_IN_FRONT		10000
#define MAXPLAYER_IN_CHANNEL	300
#define MAXPLAYER_IN_PLAYROOM	300
#define MAXPLAYER_PERTEAM		100

#define MAXCHANNEL_IN_MURIMNET	10000
#define MAXPLAYROOM_IN_MURIMNET	10000
#define MAXSERVER_IN_MURIMNET	100

enum eBATTLE_TEAM	//battle team enum �ˢ硧��u AeCOCIAU CONFIRM
{
	eTEAM_LEFT,
	eTEAM_RIGHT,
	eTEAM_OBSERVER,
	eTEAM_MAX,
};

enum eCHANNEL_MODE
{
	eCNL_MODE_ID,
	eCNL_MODE_CHANNEL,
	eCNL_MODE_PLAYROOM,
	eCNL_MODE_MAX,
};

enum ePR_CHATCLASS
{
	PRCTC_WHOLE,
	PRCTC_TEAM,
	PRCTC_WHISPER,
	PRCTC_SYSTEM,
};
//-------------


enum eBATTLE_KIND
{
	eBATTLE_KIND_NONE = 0,		// Player vs Monster ���סˢ�o����uOAC �ˢ�ic��I�Ϩ�ECO��EO�ˢ碮��AC Battle
	eBATTLE_KIND_MURIMFIELD,	// murim �ˢ硧��u munpa��E��IA ��E��IU��E����| �ˢ硧��A��EOo ��E?��Io�ˢ�io
	eBATTLE_KIND_VIMUSTREET,
	eBATTLE_KIND_MUNPAFIELD,
	eBATTLE_KIND_SURYUN,
	eBATTLE_KIND_GTOURNAMENT,
	eBATTLE_KIND_SIEGEWAR,
	
	eBATTLE_KIND_MAX,
};

enum GRID_TYPE
{
	eGridType_General, //eBATTLE_KIND_NONE
	eGridType_Murim, //eBATTLE_KIND_MURIMFIELD
};

enum eMURIMBATTLE_KIND
{
	eMurimBattle_MaxKill = 0,

	eMurimBattle_Max,
};

enum Friend_ERROR_CODE
{
	//add friend
	eFriend_AddSuccess=0,
	eFriend_AddInvalidUser,
	eFriend_AddFullList,
	eFriend_AddDeny,
	eFriend_NotLogin,
	eFriend_AlreadyRegist,
//	eFriend_RegistSelf,
	//delete friend
	eFriend_DelInvaliedUser,
//----
	eFriend_OptionNoFriend,
};

enum {
LOGIN_SUCCESS = 0,
LOGIN_ERROR_INVALID_VERSION,
LOGIN_ERROR_OVERLAPPEDLOGIN,
LOGIN_ERROR_NOAGENTSERVER,
LOGIN_ERROR_NODISTRIBUTESERVER,
LOGIN_ERROR_INVALIDUSERLEVEL,
LOGIN_ERROR_WRONGIDPW,
LOGIN_ERROR_INVALIDDISTAUTHKEY,
LOGIN_ERROR_DISTSERVERISBUSY,
LOGIN_ERROR_AGENTSERVERISBUSY,
LOGIN_ERROR_AGENTMAXCONNECTION,
LOGIN_ERROR_BLOCKUSERLEVEL,
LOGIN_ERROR_INVALID_IP,	//ipcheck
LOGIN_ERROR_DISTCONNET_ERROR,
LOGIN_ERROR_MAXUSER,
LOGIN_ERROR_OVERLAPPEDLOGININOTHERSERVER,
LOGIN_ERROR_MINOR_INADULTSERVER,
LOGIN_ERROR_NOT_CLOSEBETAWINNER,
LOGIN_ERROR_SECEDEDACCOUNT,
LOGIN_ERROR_NOREMAINTIME,
LOGIN_ERROR_NOIPREGEN,
LOGIN_ERROR_NONACCADMINIP,
LOGIN_ERROR_BUDDY_WEBFAILED,
LOGIN_ERROR_BUDDY_IDAUTHFAILED,
LOGIN_ERROR_BUDDY_AUTHCODEFAILED,
LOGIN_ERROR_PROTECTIONNUMBER,
LOGIN_ERROR_PROTECTIONNUMBER_REGIST,
LOGIN_ERROR_PROTECTIONNUMBER_INSERT,
};

#define MHSHADOW_OFF	0
#define MHSHADOW_CIRCLE	1
#define MHSHADOW_DETAIL	2

// ��Io��Ec���סˢ�ACE��IiaAu
#define MAX_MUNPAFIELD_MEMBERNUM		30

// Murim Battle
#define	MAX_MURIMFIELD_MEMBERNUM		30

// E��E�������ע�����Io ����oA�ˢ硧���ˢ�I
#define RECOVERTIMEUNIT					3000
// E��E�������ע�����Io �ˢ硧���ˢ�I�ˢ硧��Y
#define RECOVERINTERVALUNIT				10

// A��E����AI �ˢ�icAIAi(CN A��E����AIAC A����I�ˢ硧ua)
// titan maptest
#define fTILE_SIZE 50.f
//#define fTILE_SIZE 100.f

#define SKILLAREA_ATTR_DAMAGE			0x00000001
#define SKILLAREA_ATTR_BLOCK			0x00000002
#define SKILLAREA_ATTR_SAFETYZONE		0x00000003
#define SKILLAREA_ATTR_OTHEREFFECT		0x00000004

typedef BYTE AREATILE;
#define GetAreaTile	GetByte

#define MAX_ITEM_NUM 3000	//confirm
#define MAX_ITEM_OPTION_NUM	90

// INDEXGENERATER ID START INDEX
#define MAX_ITEMICON_NUM	999

#define IG_ITEM_STARTINDEX				2000
#define IG_MUGONG_STARTINDEX			3000
#define IG_MSGBOX_STRARTINDEX			4000
#define IG_STREETSTALLITEM_STARTINDEX	5000	//LBS 03.09.19
#define IG_EXCHANGEITEM_STARTINDEX		6000	//KES EXCHANGE 030920
#define IG_MENUSLOTITEM_STARTINDEX		7000	//KES 040510	//Ao�ˢ硧uY ����uE����u��E��I��E��IU. Ao��E?i�ˢ硧ui.
#define IG_PKLOOTINGDLG_START			8000	//KES PKLOOTING 040720
#define IG_DEBUG_START					10000	//KES DEBUG 040720
#define IG_QUESTITEM_STARTINDEX			15000	//RaMa
#define IG_SHOPITEM_STARTINDEX			18000	//RaMa 04.11.17
#define IG_DEALITEM_START				20000
#define IG_MAINBARICON_START			30000
#define IG_ABILITYICON_START			31000
#define IG_ABILITYPOINT_START			40000	//RaMa Ư��ġ ��й�
#define IG_QUICKITEM_STARTINDEX			60000

#define NEWOBJECT_STARTINDEX			2000001
#define NEWRECALLOBJECT_STARTINDEX		3000001
//////////////////////////////////////////////////////////////////////////
// A��I���̡�Ioy ��E?��E��c�ˢ�ˢ�? AU��Iia
enum TACTICSTART_ERROR_CODE
{
	eTSEC_NotTacticableState,
	eTSEC_NotTacticablePosition,
	eTSEC_AlreadyPartyDoingTactic,
	eTSEC_NoParty,
};
enum TACTICJOIN_ERROR_CODE
{
	eTJEC_NotTacticableState,
	eTJEC_NoPartyIdx,
	eTJEC_NoPartyInstance,
	eTJEC_NotPartyTacticState,
	eTJEC_NotEqualBetweenMSGnPartyTactic,
	eTJEC_NoNaeRyuk,
	eTJEC_NoTacticObject,
	eTJEC_NotEmptyPosition,
};

//----------------- �ˢ硧����I�ˢ碮E����A ----------------------------------
//pjs
// �ˢ硧��E�ˢ�io A��E����AO ����u�ˢ硧u����u��I��I..
enum SEARCH_TYPE
{
	MP_BY_SEARCHPRICE,
	MP_I_REGISTERED,
	MP_I_JOINED,
};
// d�ˢ�ˢ�A A��E����AO ����u�ˢ硧u����u��I��I..
enum SORT_TYPE
{
	MP_BY_WORD,
	MP_BY_PRICE,
	MP_BY_IMMEDIATEPRICE,
	MP_BY_DUEDATE,
	MP_BY_JOINAMOUNT,
};

enum CANCEL_TYPE
{
	MP_REG,
	MP_JOIN,
};
//----------------- �ˢ硧����I�ˢ碮E����A ----------------------------------


//------------------------------------------------//
/*
#define INVENTORY_CELL_NUM			80
#define MUGONG_CELL_NUM				20
#define JINBUB_CELL_NUM				5
#define SIMBUB_CELL_NUM				5
#define WEAR_CELL_NUM				10
#define MAX_MUNPA_WAREITEM_PERTAB_NUM 45
#define MAX_MUNPA_WAREHOUSETAB_NUM 7
#define MUNPAITEM_CELL_NUM		(MAX_MUNPA_WAREHOUSETAB_NUM*MAX_MUNPA_WAREITEM_PERTAB_NUM)	// 7*45 = 315
#define	MAX_PYOGUK_WAREITEM_PERTAB_NUM	30
#define MAX_PYOGUK_NUM				5
#define MAX_PYOGUKITEM_NUM			(MAX_PYOGUK_WAREITEM_PERTAB_NUM*MAX_PYOGUK_NUM)//45*5=225 -> 30*5 = 150
*/
// CAUTION !!
// ITEM & MUGONG SHARING POSITION VALUE 
// ITEM ABSOLUTE POSITION	
// define�ˢ硧�����ע�I ��IoU��I��Aa����u�ˢ硧I ��Io��Ec���סˢ�A ����u���סˢ�AIAU ���סˢ�ˢ�AAo����uCAI ���ע���?CI��E����e Cy����u�ˢ硧uAI��E?��E��c�ˢ硧��O CEE�ˢ�A ��E?�ˢ�ϡ���EOo! <- db ����uod
//CoAc PYOGUKITEM_STARTPOSITION : 586, PYOGUKITEM_ENDPOSITION : 736

/*
#define INVENTORY_STARTPOSITION		((POSTYPE)100)												// 80�ˢ硧����I�Ϩ� 
#define INVENTORY_ENDPOSITION		((POSTYPE)INVENTORY_STARTPOSITION+INVENTORY_CELL_NUM)						//80
#define WEAR_STARTPOSITION			((POSTYPE)INVENTORY_ENDPOSITION)							//10�ˢ硧����I�Ϩ�
#define WEAR_ENDPOSITION			((POSTYPE)WEAR_STARTPOSITION+WEAR_CELL_NUM)					//10			
#define STALL_STARTPOSITION			((POSTYPE)WEAR_ENDPOSITION)						
#define STALL_ENDPOSITION			((POSTYPE)WEAR_ENDPOSITION+20)								//20
#define QUICK_STARTPOSITION			((POSTYPE)STALL_ENDPOSITION)						
#define QUICK_ENDPOSITION			((POSTYPE)STALL_ENDPOSITION+MAX_QUICKITEMPERSHEET_NUM*MAX_QUICKSHEET_NUM)
//------------------------------------------------//
// MUGONG ABSOLUTE POSITION
#define MUGONG_WEAREDPOSITION		((POSTYPE)QUICK_ENDPOSITION)							
#define MUGONG_STARTPOSITION		((POSTYPE)MUGONG_WEAREDPOSITION+1)						
#define MUGONG_ENDPOSITION			((POSTYPE)MUGONG_STARTPOSITION+MUGONG_CELL_NUM)			
#define JINBUB_WEAREDPOSITION		((POSTYPE)MUGONG_ENDPOSITION)							
#define JINBUB_STARTPOSITION		((POSTYPE)JINBUB_WEAREDPOSITION+1)							
#define JINBUB_ENDPOSITION			((POSTYPE)JINBUB_STARTPOSITION+JINBUB_CELL_NUM)	
#define SIMBUB_WEAREDPOSITION		((POSTYPE)JINBUB_ENDPOSITION)							
#define SIMBUB_STARTPOSITION		((POSTYPE)SIMBUB_WEAREDPOSITION+1)						
#define SIMBUB_ENDPOSITION			((POSTYPE)SIMBUB_STARTPOSITION+SIMBUB_CELL_NUM)	

#define MAX_TOTALMUGONG_NUM			((POSTYPE)SIMBUB_ENDPOSITION-MUGONG_WEAREDPOSITION)

//------------------------------------------------//
// MUNPA ABSOLUTE POSITION
#define MUNPAITEM_STARTPOSITION		((POSTYPE)SIMBUB_ENDPOSITION)	
#define MUNPAITEM_ENDPOSITION		((POSTYPE)MUNPAITEM_STARTPOSITION+MUNPAITEM_CELL_NUM)
//------------------------------------------------//
// PYOGUK ABSOLUTE POSITION
#define PYOGUKITEM_STARTPOSITION	((POSTYPE)MUNPAITEM_ENDPOSITION)
#define PYOGUKITEM_ENDPOSITION		((POSTYPE)MUNPAITEM_ENDPOSITION+MAX_PYOGUKITEM_NUM)
*/

// Npc_Role�ˢ硧u��E�������ע������ע�E
enum NPC_ROLE
{
	OBJECT_ROLE = 0,
	DEALER_ROLE = 1,
	AUCTION_ROLE = 2,
	MUNPA_ROLE = 3,
	CHANGGO_ROLE = 4,
	FYOKUK_ROLE = 5,
	TALKER_ROLE = 6,
	WANTED_ROLE = 9,
	SURYUN_ROLE = 10,
	SYMBOL_ROLE = 11,
	CASTLE_ROLE = 12,
	GUIDE_ROLE = 13,	//magi82 - ���ϻ� ���������ý���
	TITAN_ROLE = 14,	// magi82 - Titan(070117)
	BOBUSANG_ROLE = 15,	//SW070626 ���λ�NPC
	FORTWAR_SYMBOL_ROLE = 16,
	BOMUL_ROLE	= 23,
	MAPCHANGE_ROLE = 27,	
};


enum eMoveMode
{
	eMoveMode_Run = 0,
	eMoveMode_Walk = 1,
};


/*********/


////��I�Ϩ��ˢ��?���סˢ碮Ec��E?�ˢ�A ����i��o��Ei��u���סˢ碮Ec�ˢ�ˢ��ˢ硧��AA��E?��E��c����u�ˢ硧I �ˢ�ic��E?eCI��E��IA define  ///////////////////////////////////////////////
#define MAX_PACKEDDATA_SIZE 60000 // for SEND_PACKED_DATA


// AI���ע���I���ע������ע�E��E?��E��c �ˢ�ic��E?e + Client Move��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
enum eMONSTER_ACTION
{
	eMA_STAND,
	eMA_WALKAROUND,
	eMA_PERSUIT,
	eMA_RUNAWAY,
	eMA_ATTACK,
	eMA_REST,
};

enum 
{
	eRunawayType_None,
	eRunawayType_Help = 1,
	eRunawayType_General,
	eRunawayType_Terminate,
};


/// GameResource��E?��E��c �ˢ�ic��E?e��IiC��E��IA define ///////////////////////////////////////////////
enum eQUICKICON_KIND { eMUGONG_ICON=1, eITEM_ICON, eABILITY_ICON,};
enum eMUGONGITEM_KIND{eMUGONGBOOK, eJINBUBBOOK, eSIMBUBBOOK, eERROR_BOOK,};
enum eITEM_KINDBIT 
{ 
/*
AI����uOCsY:
<512 : ��E?��Ii����ua>
<1025~ : ��Io�ˢ�i�ˢ硧����I���̢���u�ˢ硧I>
1025 : �ˢ硧��E��E?U�ˢ硧����I����
1026 : �ˢ硧uC��E?U�ˢ硧����I����
1027 : ��Ii��Ii��E?U�ˢ硧����I����
1028 : A�ˢ�E��E?U�ˢ硧����I����
1029 : �ˢ硧uA��E?U�ˢ硧����I����
1030 : ����uI�ˢ硧ua��E?U�ˢ硧����I����
1031 : E�ˢ硧I��I�Ϩ��ˢ�i�ˢ硧����I����
1032 : ����ui���ע���?i�ˢ�i�ˢ硧����I����
1033 : ��E����n��I�Ϩ��ˢ�i�ˢ硧����I����
1034 : �ˢ硧uY��I�Ϩ��ˢ�i�ˢ硧����I����
1035 : Aa��I�Ϩ��ˢ�i�ˢ硧����I����
1036 : ��Io�ˢ�i��I�Ϩ��ˢ�i�ˢ硧����I����
1037 : ����oE��Ioy
1038 : A��I���̡�Ioy
1039 : �ˢ硧����I�ˢ�ˢ硧����I����
<2049~ : AaA��I���̢���u���סˢ�AIAU>
2049 : ��Io�ˢ�i�ˢ硧ua_�ˢ硧��E
2050 : ��Io�ˢ�i�ˢ硧ua_�ˢ硧uC
2051 : ��Io�ˢ�i�ˢ硧ua_��Ii��Ii
2052 : ��Io�ˢ�i�ˢ硧ua_A�ˢ�E
2053 : ��Io�ˢ�i�ˢ硧ua_�ˢ硧uA
2054 : ��Io�ˢ�i�ˢ硧ua_����uI�ˢ硧ua
2055 : ��E?E
2056 : Ao�ˢ硧u��E����
2057 : ����oA��Io��I����
2058 : E�ˢ�I��E?I
2059 : ��E����AAa
2060 : Ca��E������Ec��EOi
2061 : ��E����n�ˢ硧��EAI
2062 : ��IoYAo
<4097~ : �ˢ硧uaA��E����>
4097 : ���ע�����E��������u��Ec�ˢ�ˢ�u
4098 : Ac�ˢ�ˢ�a�ˢ�ˢ�u
4099 : �ˢ硧uY����uO�ˢ�ˢ�u
4100 : ����u�ˢ硧IAu�ˢ�ˢ�u
4101 : ����uaAE�ˢ�ˢ�u
4102 : �ˢ硧uaA��E����
4103 : ����u���סˢ�AIAU��Iii�ˢ硧u���ע��Ϣ���u�ˢ�(����n�ˢ硧u���ע��Ϣ���u�ˢ硧I
4104 : CO����u���ע������ע�����E����v����u���סˢ�AIAU
4105 : Au����i��o��Ei��u���סˢ碮Ec
*/
	eTITAN_ITEM				= 64,
	eTITAN_ITEM_PAPER		= 65,	// Ÿ��ź ������
	eTITAN_ITEM_RECIPE		= 66,	// Ÿ��ź ������
	eTITAN_ITEM_PARTS		= 67,	// Ÿ��ź ����

	// magi82 - Titan(070206)
	eTITAN_EQUIPITEM		= 128,	// Ÿ��ź ����������
	eTITAN_EQUIPITEM_HELMET	= 129,	// Ÿ��ź ����
	eTITAN_EQUIPITEM_ARMOR	= 130,	// Ÿ��ź ����
	eTITAN_EQUIPITEM_GLOVES	= 131,	// Ÿ��ź �尩
	eTITAN_EQUIPITEM_LEGS	= 132,	// Ÿ��ź �ٸ�
	eTITAN_EQUIPITEM_CLOAK	= 133,	// Ÿ��ź ����
	eTITAN_EQUIPITEM_SHIELD	= 134,	// Ÿ��ź ����
	eTITAN_EQUIPITEM_WEAPON	= 135,	// Ÿ��ź ����
	//////////////////////////////////////////////////////////////////////////

	// RaMa - 04.11.02
	eSHOP_ITEM				= 256,
	eSHOP_ITEM_PREMIUM		= 257,	// �����̾�
	eSHOP_ITEM_CHARM		= 258,	// ����
	eSHOP_ITEM_HERB			= 259,	// ����
	eSHOP_ITEM_INCANTATION	= 260,	// �ֹ���
	eSHOP_ITEM_MAKEUP		= 261,	// ����
	eSHOP_ITEM_DECORATION	= 262,	// �ٹ̱�
	eSHOP_ITEM_SUNDRIES		= 263,	// ��ȭ
	eSHOP_ITEM_EQUIP		= 264,	// ��������
	eSHOP_ITEM_NOMALCLOTHES_SKIN	= 265,	// �Ǻ���Ʈ ��Ų ������
	eSHOP_ITEM_COSTUME_SKIN			= 266,	// �ڽ�Ƭ ��Ų ������
	
	eSHOP_ITEM_TITAN_EQUIP	= 290,	// Ÿ��ź ����

	eSHOP_ITEM_PET			= 300,	// ��
	eSHOP_ITEM_PET_EQUIP	= 310,	// �� ����

	eYOUNGYAK_ITEM			= 512,
	eYOUNGYAK_ITEM_PET		= 513,	// �� ȸ��
	eYOUNGYAK_ITEM_UPGRADE_PET	= 514,	// �� ��ȭ
	eYOUNGYAK_ITEM_REVIVAL_PET	= 515,	// �� ��Ȱ

	eYOUNGYAK_ITEM_TITAN	= 555,	// Ÿ��ź ȸ��

	eMUGONG_ITEM			= 1024,	
	eMUGONG_ITEM_SIMBUB		= 1037,
	eMUGONG_ITEM_JINBUB		= 1038,
	eMUGONG_ITEM_KYUNGGONG	= 1039,
	eMUGONG_ITEM_OPTION		= 1040,
	eMUGONG_ITEM_TITAN		= 1041,	// magi82 - Titan(070910) Ÿ��ź ������ȯ���� ���̴� kind�� Ÿ��ź �������� kind�� ����
	eMUGONG_ITEM_JOB		= 1042,	// 2007. 7. 2. CBH - ������� ����

	eEQUIP_ITEM				= 2048,
	eEQUIP_ITEM_WEAPON		= 2049,
	eEQUIP_ITEM_DRESS		= 2055,
	eEQUIP_ITEM_HAT			= 2056,
	eEQUIP_ITEM_SHOES		= 2057,
	eEQUIP_ITEM_RING		= 2062,
	eEQUIP_ITEM_CAPE		= 2059,
	eEQUIP_ITEM_NECKLACE	= 2061,
	eEQUIP_ITEM_ARMLET		= 2058,
	eEQUIP_ITEM_BELT		= 2060,
	eEQUIP_ITEM_UNIQUE		= 2100,	// magi82 - UniqueItem(070625)

	eEXTRA_ITEM				= 4096,
	eEXTRA_ITEM_JEWEL		= 4097,
	eEXTRA_ITEM_MATERIAL	= 4098,
	eEXTRA_ITEM_METAL		= 4099,
	eEXTRA_ITEM_BOOK		= 4100,
	eEXTRA_ITEM_HERB		= 4101,
	eEXTRA_ITEM_ETC			= 4102,
	eEXTRA_ITEM_COMP		= 4104,
	eEXTRA_ITEM_QUEST		= 4105,
	eEXTRA_ITEM_USABLE		= 4106,
	
	eEXTRA_ITEM_UPGRADE			= 8192,
	eEXTRA_ITEM_UPGRADE_ATTACK	= 8193,
	eEXTRA_ITEM_UPGRADE_DEFENSE	= 8194,

	eQUEST_ITEM			= 16384,
	eQUEST_ITEM_START	= 16385,	//	 
	eQUEST_ITEM_EQUIP	= 16386,	//
	eQUEST_ITEM_PET		= 16400,	// ����Ʈ ��
	eQUEST_ITEM_PET_EQUIP = 16410,	// ����Ʈ �� ����
	
	eCHANGE_ITEM		= 32768,	// eable to change
	eCHANGE_ITEM_LOCK	= 32769,	// not exchange & not streetstall 

	eKIND_ITEM_MAX,
};

#define MAX_QUERY_RESULT 100
#define MAX_JOB		3
#define MAX_ATTACKMUGONG_NUM	10
#define MAX_NAEGONG_NUM	10
#define MAX_KYUNGGONG_NUM	10
#define MAX_MUGONGITEM_NUM	10
#define MAX_YOUNGYAKITEM_NUM	10
#define MAX_EQUIPITEM_NUM	300

#define MAX_PLAYER_NUM	135
#define MAX_PET_NUM		100
// �ʵ庸�� - 05.12 �̿���
// ���� �� Ȯ�� 190 -> 300
// 300 -> 500 Ȯ�� RaMa - 07.01.22
#define MAX_MONSTER_NUM 500
#define MAX_MONSTER_REGEN_NUM 100
#define MAX_NPC_NUM 135
#define MAX_DROPITEM_NUM 20
#define MAX_NPC_REGEN_NUM 100
#define MONSTER_ATTACKTYPE1		0
#define MONSTER_ATTACKTYPE2		1
#define MONSTER_ATTACKTYPE3		2
#define MAX_MONSTERATTACK_TYPE  3

#define MAX_MONSTER_GROUPNUM	200
#define MAX_NPC_GROUPNUM		200

#define MAX_CHARACTER_LEVEL_NUM		121			//����ҵ����ȼ��޸�Ϊ121 ���¹����˰�!!!

#define MAX_PLAYERLEVEL_NUM	121 //�����ʲô��,��ȡplayerxmonstpoint.bin?//ԭ����100��״��Ϊ121
#define MAX_MONSTERLEVELPOINTRESTRICT_NUM	9
#define MONSTERLEVELRESTRICT_LOWSTARTNUM	6

// �ˢ�ic��E?e��IiC��E��IA �ˢ硧���ˢ�A : ��Io�ˢ�i�ˢ硧����I���̢�E������Ec����uO����o���ע���AI��Ii|����o���ע���
enum WP_KIND{WP_GUM=1,WP_GWUN,WP_DO,WP_CHANG,WP_GUNG,WP_AMGI,WP_KEY,WP_EVENT,WP_EVENT_HAMMER,WP_MAX,WP_ERR,WP_WITHOUT=0};
enum WP_ATTR{WP_FIRE=1, WP_WATER, WP_TREE, WP_GOLD, WP_EARTH, WP_NONEATTR};
enum RE_TARGET_RANGE{TR_PERSONAL, TR_WHOLE,};
enum MUGONG_TYPE{MUGONGTYPE_NORMAL, MUGONGTYPE_JINBUB, MUGONGTYPE_SIMBUB, MUGONGTYPE_JOB, MUGONGTYPE_MAX,};

// A���ע��Ϣ��ע�����E���� ��Io��I����E�ˢ�I(1��Io��I����A���ע��Ϣ��ע�����E����AC ��Io��I����E�ˢ�IAI��E��IU,2��Io��I���� A���ע��Ϣ��ע�����E������E��IA +1 3��Io��I����A���ע��Ϣ��ע�����E������E��IA +2....)
#define COMBO_GUM_MIN		1
#define COMBO_GWUN_MIN		7
#define COMBO_DO_MIN		13
#define COMBO_CHANG_MIN		19
#define COMBO_GUNG_MIN		25
#define COMBO_AMGI_MIN		31
#define COMBO_EVENT_MIN		50	//�����̴�����
#define COMBO_EVENT_HAMMER	51
//#define COMBO_EVENT_HARDSNOW	52	//�������̴�����

#define MAX_COMBO_NUM		6
#define SKILL_COMBO_NUM		100
#define MAX_COMBOATTACK_LIST (WP_MAX*MAX_COMBO_NUM)		 // 6(��Io�ˢ�i�ˢ硧ua~�ˢ�ˢ�u) * 6(AO��E��IeA���ע��Ϣ��ע�����E����)

//#define SKILLNUM_TO_TITAN	10000	// ĳ���� ��ų�� ���ϸ� Ÿ��ź ��ų �ε���
// ����o���ע���A��I�Ϩ� ��Io��I����E�ˢ�I
#define MAX_SKILLATTACK_LIST (WP_MAX*50)				// ��Io�ˢ�i�ˢ硧ua~�ˢ�ˢ�u * ��Io�ˢ�i�ˢ硧ua��E������EO��E��IUAC ����o���ע���A��I�Ϩ� ����uo
#define MAX_SUNG_EXP 7									// 7����u���ע����ˢ硧uiAoAC �ˢ硧����I�ˢ�CeA��E��c
#define MAX_SUNG 12									

// SkillResult
#define SKILLRESULT_ATTACK				0x00000001
#define SKILLRESULT_RECOVERLIFE			0x00000010
#define SKILLRESULT_RECOVERNAERYUK		0x00000100

#define SKILLTARGETTYPE_ENEMY			0
#define SKILLTARGETTYPE_TEAMMATE		1
#define SKILLTARGETTYPE_SELF			2
#define SKILLTARGETTYPE_ENEMYANDTEAM	3
#define SKILLTARGETTYPE_NOTENEMYTARGET	4

#define SKILLRESULTKIND_NONE			0
#define SKILLRESULTKIND_POSITIVE		1
#define SKILLRESULTKIND_NEGATIVE		2

#define ATTACKTYPE_MELEE				0
#define ATTACKTYPE_RANGE				0

//////////////////////////////////////////////////////////////////////////
// AI��IoY�ˢ硧����I���̡ˢ硧��Y or ��E��IUA��I�����ˢ硧����I���̡ˢ硧��Y
#define ATTACKTYPE_NORMAL		0
#define ATTACKTYPE_HALFCIRCLE	1
#define ATTACKTYPE_CIRCLE		2
#define ATTACKTYPE_LINE			3
#define ATTACKTYPE_RECTANGLE	4

// ��E��IUA��I�����ˢ硧����I���̡ˢ硧��YAC ��Iou'AC �ˢ硧uaA���ס���a
#define	TARGETTYPE_TARGET	0
#define TARGETTYPE_ATTACKER 1
#define TARGETTYPE_GROUND	2
//////////////////////////////////////////////////////////////////////////

// ����u���סˢ�AIAU �ˢ硧u��E�������ע����ˢ硧����E?��E��c ��E��IeCN enum
/*
enum ICON_GROUP_KINDS
{ 
	eIGK_ATTACK_MUGONG,
	eIGK_DEFENSE_MUGONG, 
	eIGK_ASSIST_MUGONG, 
	eIGK_SPECIAL_MUGONG, 
	eIGK_YOUNGYAK_ITEM, 
	eIGK_MUGONG_ITEM, 
	eIGK_EQUIP_ITEM,
	eIGK_Max,
};*/


enum eMUGONG_KIND
{ 
	eATTACK_MUGONG, 
	eDEFENSE_MUGONG, 
	eASSIST_MUGONG, 
	eSPECIAL_MUGONG, 
	eNAEGONG,
	eKYUNGGONG,
	eMAX_MUGONG, 
	eERROR_MUGONG
};

enum eItemUse_Err
{
	eItemUseSuccess,

	eItemUseErr_PreInsert,
	eItemUseErr_Move,
	eItemUseErr_Combine,
	eItemUseErr_Divide,
	eItemUseErr_Discard,
	eItemUseErr_Mix,
	eItemUseErr_Quest,
	eItemUseErr_Err,
	eItemUseErr_Upgrade,
	eItemUseErr_Lock,
	eItemUseErr_AlreadyUse,
	eItemUseErr_Unabletime,
	eItemUseErr_AlreadyPoint,
	eItemUseErr_AlredyChange,
	eItemuseErr_DontUseToday,
	eItemUseErr_DontDupUse,
	eItemUseErr_UseFull,
	eItemUseErr_TitanPartsMake,	// magi82 - Titan(070118)
	eItemUseErr_TitanItemTime,	// Ÿ��ź epȸ�� ������
};

/// GameResource��E?��E��c �ˢ�ic��E?e��IiC��E��IA define ///////////////////////////////////////////////
#define MAX_YOUNGYAKITEM_DUPNUM			20
#ifdef _JAPAN_LOCAL_
#define MAX_CHARACTER_NUM	5
#elif defined _HK_LOCAL_
#define MAX_CHARACTER_NUM	5
#elif defined _TL_LOCAL_
#define MAX_CHARACTER_NUM	5
#else
#define MAX_CHARACTER_NUM	4
#endif


#define MAX_BOARDTITLE_NUM	10
#define MAX_BOARDTITLE_LENGTH 64
#define MAX_CONTENTS_LENGTH	65535

//#define MAX_STREETSTALL_TITLELEN 38
#define MAX_STREETSTALL_TITLELEN 66

#define MAX_FILE_NAME		32


#define WALKSPEED		200
#define RUNSPEED		400

#define TITAN_WALKSPEED	300

//����u���סˢ�AIAU Define
//��I�Ϩ���I��AAU 
#define ManBlueDress		23010 //���סˢ�o�ˢ�io���ע�����Io
#define ManBlueNothing		23011
#define ManDefaultShoose	27000 //��Io�ˢ�i��E����iE�ˢ硧I 
//��E?����IAU 
//#define WomanRedDress		12210 //���סˢ�o�ˢ�io���ע�����Io
#define WomanRedDress		23010
#define WomanDefalutShoose	27000 //��Io�ˢ�i��E����iE�ˢ硧I

//  [5/27/2003]

// A��I�Ϩ���E����?AIA�ˢ�E��E?��E��c ���סˢ�ˢ�AAIAI ��Ioe���ע������ע�E�ˢ硧��u�ˢ�ˢ�A
enum { GENGOL_POINT, SIMMEK_POINT, MINCHUB_POINT, CHERYUK_POINT, MAX_BTN_POINT};

#define GENDER_MALE 0
#define GENDER_FEMALE 1
#define GENDER_MAX 2

#define MAX_PARTY_LISTNUM 7
#define MAX_PARTY_LOAD_NUM 100
#define MAX_PARTY_NAME 28		//2008. 5. 21. CBH - ���� ���� ���� ����

#define MAX_MUNPA_NAME 20
#define MAX_MUNPALOAD_LIST	100
#define MAX_MUNPA_LIST	10
#define MAX_MUNPA_INTROMSG	568
#define MAX_MUNPA_IMAGE_NUM 32

#define MUNPA_MASTER		20
#define MUNPA_NOTMEMBER		0	//��Io��Ec���סˢ�A��E?��I����AI ����u���סˢ碮E��IN �ˢ�ic��EO�ˢ�A
#define MUNPA_SEMIMEMBER	1	//��Io��Ec���סˢ�A �ˢ硧����E��cAO����oAA�ˢ�iAI
#define MUNPA_MEMBER		2	//��Io��Ec���סˢ�A��E?��I����
#define MUNPA_SENIOR		3	//Aa�ˢ�ˢ�I
#define MUNPA_VICE_MASTER	10	//���ע���I��Io��EcAO

#define MUNPA_KIND_JUNG	1	//d���סˢ�A
#define MUNPA_KIND_SA	2	//�ˢ�ic���סˢ�A
#define MUNPA_KIND_WHANG	3	//E��I��A�ˢ硧uA
#define MUNPA_KIND_MU	4	//��Io�ˢ�i����uO����uO

#define MAX_MUNPA_BOARD_NUM	5	//��Io��Ec���סˢ�A �ˢ硧��O����oA���סˢ�C
#define MAX_MUNPA_BOARD_NAME 16		//�ˢ硧��O����oA���סˢ�C AI��E�����ˢ碮��
#define MAX_MUNPA_REGULARMEMBERLIST_NUM 10  //��E������Ec����i��o��Ei��u���סˢ碮Ec��E?��E��c ���ע�����E������E?����IAo��E��IA ��Io��Ec���סˢ�A��E?��I���̢�Iie ����uo
#define MAX_MUNPA_BATTLEKIND 3
#define MAX_MUNPA_SEMIMEMBERLIST_NUM 10 //��E������Ec����i��o��Ei��u���סˢ碮Ec��E?��E��c ���ע�����E������E?����IAo��E��IA ����oAA�ˢ�iAI��Iie ����uo
#define MAX_MUNPA_BOARD_SUBJECT 64
#define MAX_MUNPA_BOARD_CONTENT 2048
#define MAX_MUNPA_BOARD_LIST 10

#define MAX_MUNPA_WARE_TABNUM 7
#define MAX_PYOGUKLIST_NUM 5
#define MAX_PYOGUKLOAD_ITEM 100
#define MAX_OPTIONLOAD_ITEM 100
#define MAX_SHOPLOAD_ITEM	100
#define MAX_LOAD_ITEM		100
#define MAX_PETINVENLOAD_ITEM 100
#define MAX_PINVENLOAD_ITEM 100
#define MAX_TITANENDURANCELOAD_ITEM 100	// magi82 - Titan(071015) â���� Ÿ��ź ��� ����
#define MAX_FRIEND_NUM 30
#define MAX_FRIEND_LIST 10
#define MAX_FRIEND_PAGE MAX_FRIEND_NUM/MAX_FRIEND_LIST // 30/10 = 10

#define MAX_NOTENUM 40
#define MAX_NOTE_PAGE 4
#define NOTENUM_PER_PAGE MAX_NOTENUM/MAX_NOTE_PAGE // 30/3 = 10
#define MAX_NOTE_LENGTH 255

#define MAX_CHANNEL_NUM 10
#define MAX_CHANNEL_NAME 16

#define MAX_TATIC_ABILITY_NUM 10

enum EWEARED_ITEM
{
	eWearedItem_Hat = 0,
	eWearedItem_Weapon = 1,
	eWearedItem_Dress = 2,
	eWearedItem_Shoes = 3,
	
	eWearedItem_Ring1 = 4,
	eWearedItem_Ring2 = 5,

	eWearedItem_Cape = 6,
	eWearedItem_Necklace = 7,
	eWearedItem_Armlet = 8,
	eWearedItem_Belt = 9,

	eWearedItem_Max,
};


enum TITAN_KIND
{
	TITANKIND_1,
	TITANKIND_MAX,
};


enum TITAN_WEARED_ITEM
{
	eTitanWearedItem_HelMet,
	eTitanWearedItem_Body,
	eTitanWearedItem_Glove,
	eTitanWearedItem_Legs,
	eTitanWearedItem_Cloak,
	eTitanWearedItem_Shield,
	eTitanWearedItem_Weapon,
	eTitanWearedItem_Max,
};

enum WantedBuyErrCode
{
	eBWR_Success,
	eBWR_Err_Err,
	eBWR_Err_FullVolunteer, 
};
#define MAX_WANTED_NUM 20
#define MAX_WANTED_VOLUNTEER 100
#define WANTUNIT_PER_PAGE 12

enum ERROR_GUILD
{
	eGuildErr_Err = 1,
			
	eGuildErr_Create_Name,
	eGuildErr_Create_Intro,
	
	eGuildErr_BreakUp,
	
	eGuildErr_DeleteMember,
	eGuildErr_DeleteMember_NothaveAuthority,
	eGuildErr_DeleteMember_NotMember,
	
	eGuildErr_AddMember_NotPlayer,
	eGuildErr_AddMember_OtherGuild,
	eGuildErr_AddMember_AlreadyMember,
	eGuildErr_AddMember_FullMember,	
	eGuildErr_AddMember_TargetNotDay,
	eGuildErr_AddMember_NothaveAuthority,

	eGuildErr_InviteApt_NoMaster,

	eGuildErr_Mark_NoGuild,
	eGuildErr_Mark_NoMark,

	eGuildErr_LevelUp_NotAbil,
	eGuildErr_LevelUp_NotMoney,
	eGuildErr_LevelUp_Complete,

	eGuildErr_ChangeRank_NoGuild,
	eGuildErr_ChangeRank_NotMember,

	eGuildErr_NoGuild,

	eGuildErr_Nick_Filter,
	eGuildErr_Nick_NotMember,
	eGuildErr_Nick_NotAvailableName,
	eGuildErr_Nick_NotLogIn,

	eGuildErr_AddStudent_TooHighLevelAsStudent,
};
enum
{
	eGuild_Delete,
	eGuild_Secede,		
};
//--------��E?��E��c�ˢ�ˢ�? ��E�������ע��Ϣ���oAAo----------------------------------------------------------
enum DEAL_BUY_ERROR { NOT_ERROR, NOT_MONEY = 101, NOT_SPACE, NOT_EXIST, NOT_PLAYER, NOT_OWNERMONEY, NOT_SAMEPRICE, NOT_SAMEINFO, NO_DEMANDITEM, NOT_REMAINITEM };
enum ERROR_MUNPABOARDCONTENTS {
	ERROR_MUNPABOARDCONTENTS_NOCONTENTS = 1,
	ERROR_MUNPABOARDCONTENTS_NOAUTHORITY,
};

enum ERROR_PARTY 
{
	eErr_Create_DB,
	eErr_Add_NoPlayer,
	eErr_Add_AlreadyinParty,
	eErr_Add_OptionNoParty,
	eErr_Add_DiedPlayer,
	eErr_ChangingMaster,
	eErr_Add_Full, //can not join cause party is full!
	eErr_BrokenParty, // Party has already broken.
	eErr_Create_MapHasParty, //map has party but client doesn't have party
	eErr_Add_NotOurGuild,		// During Guild Tournament
};

//2008. 5. 23. CBH - ���� ��û ����
enum ERROR_PARTY_REQUEST
{
	eErr_Request_NotParty,			//���İ� ����
	eErr_Request_NotMaster,			//�������� ����	
	eErr_Request_PartyExistence,	//��û�ڰ� ��Ƽ�� ���ԵǾ��ִ�.
	eErr_Request_Full,				//�ο� �ʰ�
	eErr_Request_Level,				//������ ���� �ʴ�
	eErr_Request_Public,			//����� ���´�
	eErr_Request_NotState,			//��û�� ���� �� ���� ����
	eErr_Request_Progress,	//��û ������
	eErr_Request_Refusal,	//��û ����
	eErr_Request_TimeExcess,		//�ð��ʰ�	
};

enum PARTY_OPTION
{
	ePartyOpt_Random,
	ePartyOpt_Damage,
	ePartyOpt_Sequence,
};
//--------��E?��E��c�ˢ�ˢ�? ��E�������ע��Ϣ���oAAo----------------------------------------------------------

enum TABNUMBER
{
	TAB_INVENTORY_NUM			= 4,
	TAB_MUNPAWAREHOUSE_NUM		= 7,
	TAB_PYOGUK_NUM				= 5,
	TAB_QUICK_NUM				= 4,
	TAB_ABILITY_NUM				= 4,	//SW051129 Pet 4->5
	TAB_MENUSLOT_NUM			= 4,	//KES 040510
	TAB_QITEMINVEN_NUM			= 4,
	TAB_GUILDWAREHOUSE_NUM		= 2,		// 06.12.15 RaMa - ���������� ������� �ǵ����� �Ѵ�.
	//TAB_GUILDWAREHOUSE_NUM		= 5,		
	TAB_SHOPITEM_NUM			= 5,
	TAB_SHOPINVEN_NUM			= 2,
	TAB_PET_INVENTORY_NUM		= 3,
};

enum CELLPERTAB
{
	TABCELL_INVENTORY_NUM		= 20,
	TABCELL_MUNPAWAREHOUSE_NUM	= 45,
	TABCELL_PYOGUK_NUM			= 30,
	TABCELL_QUICK_NUM			= 10,
	TABCELL_ABILITY_NUM			= 40,
	TABCELL_MENUSLOT_NUM		= 7,	//KES 040510
	TABCELL_QITEMINVEN_NUM		= 20,
	TABCELL_GUILDWAREHOUSE_NUM	= 30,
	TABCELL_SHOPITEM_NUM		= 30,	// RaMa 04.11.03
	TABCELL_SHOPINVEN_NUM		= 20,
	TABCELL_PETINVEN_NUM		= 20,
	TABCELL_SEIGEFORTWAREHOUES_NUM	= 30,
};

#ifdef _JAPAN_LOCAL_

enum
{
	SLOT_INVENTORY_NUM			= TAB_INVENTORY_NUM * TABCELL_INVENTORY_NUM,//80
	SLOT_WEAR_NUM				= 10,

	SLOT_MUGONG_NUM				= 20,
	SLOT_JINBUB_NUM				= 5,
	SLOT_MUGONGTOTAL_NUM		= SLOT_MUGONG_NUM + SLOT_JINBUB_NUM,

	SLOT_MUNPAWAREHOUSE_NUM		= TAB_MUNPAWAREHOUSE_NUM * TABCELL_MUNPAWAREHOUSE_NUM,	//315
	SLOT_PYOGUK_NUM				= TAB_PYOGUK_NUM * TABCELL_PYOGUK_NUM,//150

	SLOT_STREETSTALL_NUM		= 25,
	SLOT_QUICK_NUM				= TAB_QUICK_NUM * TABCELL_QUICK_NUM,//11

	SLOT_ABILITY_NUM			= TAB_ABILITY_NUM * TABCELL_ABILITY_NUM,

	SLOT_QITEMINVEN_NUM			= TAB_QITEMINVEN_NUM * TABCELL_QITEMINVEN_NUM,//100
	
	SLOT_GUILDWAREHOUSE_NUM		= TAB_GUILDWAREHOUSE_NUM * TABCELL_GUILDWAREHOUSE_NUM,	//60

	SLOT_SHOPITEM_NUM			= TAB_SHOPITEM_NUM * TABCELL_SHOPITEM_NUM,
	
	SLOT_SHOPINVEN_NUM			= TAB_SHOPINVEN_NUM * TABCELL_SHOPINVEN_NUM,
	SLOT_WEAR_SHOPITEM_NUM		= 10,

	SLOT_STREETBUYSTALL_NUM		= 5,	

	SLOT_PETINVEN_NUM			= TAB_PET_INVENTORY_NUM * TABCELL_PETINVEN_NUM,	//60
	SLOT_PETWEAR_NUM			= 3,

	SLOT_NPCINVEN_NUM			= 30,

	// ���� & ���â��
	SLOT_SEIGEFORTWARE_NUM		= 90,
};

// < ��E?��I����A�ˢ�E > : taiyo
// TP_INVENTORY_START��E��IA			��IoY��Iia����oA = 0
// TP_MUNPAWAREHOUSE_START��E��IA	��IoY��Iia����oA = TP_SIMBUB_END
// TP_MAX��E��IA						��IoY��Iia����oA = TP_PYOGUK_END,
enum
{
	//ITEMTABLE��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_INVENTORY_START			= 0,
	TP_INVENTORY_END			= TP_INVENTORY_START		+ SLOT_INVENTORY_NUM,	// 80
	TP_WEAR_START				= TP_INVENTORY_END,									// 80
	TP_WEAR_END					= TP_WEAR_START				+ SLOT_WEAR_NUM,		// 90

	TP_PYOGUK_START				= TP_WEAR_END, // 90
	TP_PYOGUK_END				= TP_PYOGUK_START			+ SLOT_PYOGUK_NUM,		// 240

//	TP_QITEMINVEN_START			= TP_PYOGUK_END,
//	TP_QITEMINVEN_END			= TP_QITEMINVEN_START		+ SLOT_QITEMINVEN_NUM,
//	TP_MAX						= TP_QITEMINVEN_END,

	//ItemTableAI��E?U��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_MUNPAWAREHOUSE_START		= TP_PYOGUK_END,
	TP_MUNPAWAREHOUSE_END		= TP_MUNPAWAREHOUSE_START	+ SLOT_MUNPAWAREHOUSE_NUM,

	TP_MUGONG_START				= TP_MUNPAWAREHOUSE_END,
	TP_MUGONG_END				= TP_MUGONG_START			+ SLOT_MUGONG_NUM,
	TP_JINBUB_START				= TP_MUGONG_END,
	TP_JINBUB_END				= TP_JINBUB_START			+ SLOT_JINBUB_NUM,
	
	TP_STREETSTALL_START		= TP_JINBUB_END,
	TP_STREETSTALL_END			= TP_STREETSTALL_START		+ SLOT_STREETSTALL_NUM,

	TP_QUICK_START				= TP_STREETSTALL_END,
	TP_QUICK_END				= TP_QUICK_START			+ SLOT_QUICK_NUM,

	TP_ABILITY_START			= TP_QUICK_END,
	TP_ABILITY_END				= TP_ABILITY_START			+ SLOT_ABILITY_NUM,

	TP_SHOPITEM_START			= TP_PYOGUK_END,	// 240
	TP_SHOPITEM_END				= TP_SHOPITEM_START			+ SLOT_SHOPITEM_NUM,

	TP_SHOPINVEN_START			= TP_SHOPITEM_END,	// 390
	TP_SHOPINVEN_END			= TP_SHOPINVEN_START		+ SLOT_SHOPINVEN_NUM,

	TP_PETINVEN_START			= TP_SHOPINVEN_END,
	TP_PETINVEN_END				= TP_PETINVEN_START			+ SLOT_PETINVEN_NUM			,		// +60

	TP_PETWEAR_START			= TP_PETINVEN_END,
	TP_PETWEAR_END				= TP_PETWEAR_START			+ SLOT_PETWEAR_NUM,			// +3

//	TP_MAX						= TP_PYOGUK_END + SLOT_SHOPITEM_NUM + SLOT_SHOPINVEN_NUM,	// 240+150+40=430
	TP_MAX						= TP_PYOGUK_END + SLOT_SHOPITEM_NUM + SLOT_SHOPINVEN_NUM + SLOT_PETINVEN_NUM + SLOT_PETWEAR_NUM,	// (430) + 60 + 3

	TP_GUILDWAREHOUSE_START		= TP_MAX, //430
	TP_GUILDWAREHOUSE_END		= TP_GUILDWAREHOUSE_START + SLOT_GUILDWAREHOUSE_NUM, //490

	//06.08.08 [PKH]
	TP_MUGONG1_START			= 600,													// 600		// 555
	TP_MUGONG1_END				= TP_MUGONG1_START			+ SLOT_MUGONG_NUM,			// 625		// 580

	TP_MUGONG2_START			= TP_MUGONG1_END,										// 625		// 580
	TP_MUGONG2_END				= TP_MUGONG2_START			+ SLOT_MUGONG_NUM,			// 650		// 605

	// ���� & ���
	TP_SIEGEFORTWARE_START		= 10000,
	TP_SIEGEFORTWARE_END		= TP_SIEGEFORTWARE_START + SLOT_SEIGEFORTWARE_NUM,			// 10090
};

#elif defined _TL_LOCAL_
enum
{
	SLOT_INVENTORY_NUM			= TAB_INVENTORY_NUM * TABCELL_INVENTORY_NUM,//80
	SLOT_WEAR_NUM				= 10,

	SLOT_MUGONG_NUM				= 20,
	SLOT_JINBUB_NUM				= 5,
	SLOT_MUGONGTOTAL_NUM		= SLOT_MUGONG_NUM + SLOT_JINBUB_NUM,

	SLOT_MUNPAWAREHOUSE_NUM		= TAB_MUNPAWAREHOUSE_NUM * TABCELL_MUNPAWAREHOUSE_NUM,	//315
	SLOT_PYOGUK_NUM				= TAB_PYOGUK_NUM * TABCELL_PYOGUK_NUM,//150

	SLOT_STREETSTALL_NUM		= 25,
	SLOT_QUICK_NUM				= TAB_QUICK_NUM * TABCELL_QUICK_NUM,//11

	SLOT_ABILITY_NUM			= TAB_ABILITY_NUM * TABCELL_ABILITY_NUM,

	SLOT_QITEMINVEN_NUM			= TAB_QITEMINVEN_NUM * TABCELL_QITEMINVEN_NUM,//100
	
	SLOT_GUILDWAREHOUSE_NUM		= TAB_GUILDWAREHOUSE_NUM * TABCELL_GUILDWAREHOUSE_NUM,	//60

	SLOT_SHOPITEM_NUM			= TAB_SHOPITEM_NUM * TABCELL_SHOPITEM_NUM,
	
	SLOT_SHOPINVEN_NUM			= TAB_SHOPINVEN_NUM * TABCELL_SHOPINVEN_NUM,
	SLOT_WEAR_SHOPITEM_NUM		= 10,

	SLOT_STREETBUYSTALL_NUM		= 5,	

	//06.08.08 [PKH] 
	SLOT_PETINVEN_NUM			= TAB_PET_INVENTORY_NUM * TABCELL_PETINVEN_NUM,	//60
	SLOT_PETWEAR_NUM			= 3,

	SLOT_NPCINVEN_NUM			= 30,

	// ���� & ���â��
	SLOT_SEIGEFORTWARE_NUM		= 90,
};

// < ��E?��I����A�ˢ�E > : taiyo
// TP_INVENTORY_START��E��IA			��IoY��Iia����oA = 0
// TP_MUNPAWAREHOUSE_START��E��IA	��IoY��Iia����oA = TP_SIMBUB_END
// TP_MAX��E��IA						��IoY��Iia����oA = TP_PYOGUK_END,
enum
{
	//ITEMTABLE��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_INVENTORY_START			= 0,
	TP_INVENTORY_END			= TP_INVENTORY_START		+ SLOT_INVENTORY_NUM,	// 80
	TP_WEAR_START				= TP_INVENTORY_END,									// 80
	TP_WEAR_END					= TP_WEAR_START				+ SLOT_WEAR_NUM,		// 90

	TP_PYOGUK_START				= TP_WEAR_END, // 90
	TP_PYOGUK_END				= TP_PYOGUK_START			+ SLOT_PYOGUK_NUM,		// 240

//	TP_QITEMINVEN_START			= TP_PYOGUK_END,
//	TP_QITEMINVEN_END			= TP_QITEMINVEN_START		+ SLOT_QITEMINVEN_NUM,
//	TP_MAX						= TP_QITEMINVEN_END,

	//ItemTableAI��E?U��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_MUNPAWAREHOUSE_START		= TP_PYOGUK_END,
	TP_MUNPAWAREHOUSE_END		= TP_MUNPAWAREHOUSE_START	+ SLOT_MUNPAWAREHOUSE_NUM,

	TP_MUGONG_START				= TP_MUNPAWAREHOUSE_END,
	TP_MUGONG_END				= TP_MUGONG_START			+ SLOT_MUGONG_NUM,
	TP_JINBUB_START				= TP_MUGONG_END,
	TP_JINBUB_END				= TP_JINBUB_START			+ SLOT_JINBUB_NUM,
	
	TP_STREETSTALL_START		= TP_JINBUB_END,
	TP_STREETSTALL_END			= TP_STREETSTALL_START		+ SLOT_STREETSTALL_NUM,

	TP_QUICK_START				= TP_STREETSTALL_END,
	TP_QUICK_END				= TP_QUICK_START			+ SLOT_QUICK_NUM,

	TP_ABILITY_START			= TP_QUICK_END,
	TP_ABILITY_END				= TP_ABILITY_START			+ SLOT_ABILITY_NUM,

	TP_SHOPITEM_START			= TP_PYOGUK_END,	// 240
	TP_SHOPITEM_END				= TP_SHOPITEM_START			+ SLOT_SHOPITEM_NUM,

	TP_SHOPINVEN_START			= TP_SHOPITEM_END,	// 390
	TP_SHOPINVEN_END			= TP_SHOPINVEN_START		+ SLOT_SHOPINVEN_NUM,

	//06.08.08 [PKH]
	TP_PETINVEN_START			= TP_SHOPINVEN_END,										// 430
	TP_PETINVEN_END				= TP_PETINVEN_START			+ SLOT_PETINVEN_NUM,		// 490

	TP_PETWEAR_START			= TP_PETINVEN_END,										// 490
	TP_PETWEAR_END				= TP_PETWEAR_START			+ SLOT_PETWEAR_NUM,			// 493

	TP_MAX						= TP_PYOGUK_END + SLOT_SHOPITEM_NUM + SLOT_SHOPINVEN_NUM,

	TP_GUILDWAREHOUSE_START		= TP_MAX, //430
	TP_GUILDWAREHOUSE_END		= TP_GUILDWAREHOUSE_START + SLOT_GUILDWAREHOUSE_NUM, //490

	//06.08.08 [PKH]
	TP_MUGONG1_START			= 600,													// 600		// 555
	TP_MUGONG1_END				= TP_MUGONG1_START			+ SLOT_MUGONG_NUM,			// 625		// 580

	TP_MUGONG2_START			= TP_MUGONG1_END,										// 625		// 580
	TP_MUGONG2_END				= TP_MUGONG2_START			+ SLOT_MUGONG_NUM,			// 650		// 605


	// ���� & ���
	TP_SIEGEFORTWARE_START		= 10000,
	TP_SIEGEFORTWARE_END		= TP_SIEGEFORTWARE_START + SLOT_SEIGEFORTWARE_NUM,			// 10090
};

#elif defined _HK_LOCAL_
enum
{
	SLOT_INVENTORY_NUM			= TAB_INVENTORY_NUM * TABCELL_INVENTORY_NUM,//80
	SLOT_WEAR_NUM				= 10,

	SLOT_MUGONG_NUM				= 20,
	SLOT_JINBUB_NUM				= 5,
	SLOT_MUGONGTOTAL_NUM		= SLOT_MUGONG_NUM + SLOT_JINBUB_NUM,

	SLOT_MUNPAWAREHOUSE_NUM		= TAB_MUNPAWAREHOUSE_NUM * TABCELL_MUNPAWAREHOUSE_NUM,	//315
	SLOT_PYOGUK_NUM				= TAB_PYOGUK_NUM * TABCELL_PYOGUK_NUM,//150

	SLOT_STREETSTALL_NUM		= 25,
	SLOT_QUICK_NUM				= TAB_QUICK_NUM * TABCELL_QUICK_NUM,//11

	SLOT_ABILITY_NUM			= TAB_ABILITY_NUM * TABCELL_ABILITY_NUM,

	SLOT_QITEMINVEN_NUM			= TAB_QITEMINVEN_NUM * TABCELL_QITEMINVEN_NUM,//100
	
	SLOT_GUILDWAREHOUSE_NUM		= TAB_GUILDWAREHOUSE_NUM * TABCELL_GUILDWAREHOUSE_NUM,	//60

	SLOT_SHOPITEM_NUM			= TAB_SHOPITEM_NUM * TABCELL_SHOPITEM_NUM,
	
	SLOT_SHOPINVEN_NUM			= TAB_SHOPINVEN_NUM * TABCELL_SHOPINVEN_NUM,
	SLOT_WEAR_SHOPITEM_NUM		= 10,

	SLOT_STREETBUYSTALL_NUM		= 5,	

	//06.08.08 [PKH] 
	SLOT_PETINVEN_NUM			= TAB_PET_INVENTORY_NUM * TABCELL_PETINVEN_NUM,	//60
	SLOT_PETWEAR_NUM			= 3,

	SLOT_NPCINVEN_NUM			= 30,

	// ���� & ���â��
	SLOT_SEIGEFORTWARE_NUM		= 90,
};

// < ��E?��I����A�ˢ�E > : taiyo
// TP_INVENTORY_START��E��IA			��IoY��Iia����oA = 0
// TP_MUNPAWAREHOUSE_START��E��IA	��IoY��Iia����oA = TP_SIMBUB_END
// TP_MAX��E��IA						��IoY��Iia����oA = TP_PYOGUK_END,
enum
{
	//ITEMTABLE��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_INVENTORY_START			= 0,
	TP_INVENTORY_END			= TP_INVENTORY_START		+ SLOT_INVENTORY_NUM,	// 80
	TP_WEAR_START				= TP_INVENTORY_END,									// 80
	TP_WEAR_END					= TP_WEAR_START				+ SLOT_WEAR_NUM,		// 90

	TP_PYOGUK_START				= TP_WEAR_END, // 90
	TP_PYOGUK_END				= TP_PYOGUK_START			+ SLOT_PYOGUK_NUM,		// 240

//	TP_QITEMINVEN_START			= TP_PYOGUK_END,
//	TP_QITEMINVEN_END			= TP_QITEMINVEN_START		+ SLOT_QITEMINVEN_NUM,
//	TP_MAX						= TP_QITEMINVEN_END,

	//ItemTableAI��E?U��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_MUNPAWAREHOUSE_START		= TP_PYOGUK_END,
	TP_MUNPAWAREHOUSE_END		= TP_MUNPAWAREHOUSE_START	+ SLOT_MUNPAWAREHOUSE_NUM,

	TP_MUGONG_START				= TP_MUNPAWAREHOUSE_END,
	TP_MUGONG_END				= TP_MUGONG_START			+ SLOT_MUGONG_NUM,
	TP_JINBUB_START				= TP_MUGONG_END,
	TP_JINBUB_END				= TP_JINBUB_START			+ SLOT_JINBUB_NUM,
	
	TP_STREETSTALL_START		= TP_JINBUB_END,
	TP_STREETSTALL_END			= TP_STREETSTALL_START		+ SLOT_STREETSTALL_NUM,

	TP_QUICK_START				= TP_STREETSTALL_END,
	TP_QUICK_END				= TP_QUICK_START			+ SLOT_QUICK_NUM,

	TP_ABILITY_START			= TP_QUICK_END,
	TP_ABILITY_END				= TP_ABILITY_START			+ SLOT_ABILITY_NUM,

	TP_SHOPITEM_START			= TP_PYOGUK_END,	// 240
	TP_SHOPITEM_END				= TP_SHOPITEM_START			+ SLOT_SHOPITEM_NUM,

	TP_SHOPINVEN_START			= TP_SHOPITEM_END,	// 390
	TP_SHOPINVEN_END			= TP_SHOPINVEN_START		+ SLOT_SHOPINVEN_NUM,
	
	//06.08.08 [PKH]
	TP_PETINVEN_START			= TP_SHOPINVEN_END,										// 430
	TP_PETINVEN_END				= TP_PETINVEN_START			+ SLOT_PETINVEN_NUM,		// 490

	TP_PETWEAR_START			= TP_PETINVEN_END,										// 490
	TP_PETWEAR_END				= TP_PETWEAR_START			+ SLOT_PETWEAR_NUM,			// 493

	TP_MAX						= TP_PYOGUK_END + SLOT_SHOPITEM_NUM + SLOT_SHOPINVEN_NUM,
	
	TP_GUILDWAREHOUSE_START		= 500,													// 500		// 430
	TP_GUILDWAREHOUSE_END		= TP_GUILDWAREHOUSE_START + SLOT_GUILDWAREHOUSE_NUM,	// 560		// 490

	TP_MUGONG_START				= 600,													//600
	TP_MUGONG_END				= TP_MUGONG_START			+ SLOT_MUGONG_NUM,			//620
	TP_JINBUB_START				= TP_MUGONG_END,										//620
	TP_JINBUB_END				= TP_JINBUB_START			+ SLOT_JINBUB_NUM,			//625

	//06.08.08 [PKH]
	TP_MUGONG1_START			= 600,													// 600		// 555
	TP_MUGONG1_END				= TP_MUGONG1_START			+ SLOT_MUGONG_NUM,			// 625		// 580

	TP_MUGONG2_START			= TP_MUGONG1_END,										// 625		// 580
	TP_MUGONG2_END				= TP_MUGONG2_START			+ SLOT_MUGONG_NUM,			// 650		// 605


	// ���� & ���
	TP_SIEGEFORTWARE_START		= 10000,
	TP_SIEGEFORTWARE_END		= TP_SIEGEFORTWARE_START + SLOT_SEIGEFORTWARE_NUM,			// 10090
};

#else

enum
{
	SLOT_INVENTORY_NUM			= TAB_INVENTORY_NUM * TABCELL_INVENTORY_NUM,//80
	SLOT_WEAR_NUM				= 10,

	SLOT_MUNPAWAREHOUSE_NUM		= TAB_MUNPAWAREHOUSE_NUM * TABCELL_MUNPAWAREHOUSE_NUM,	//315
	SLOT_PYOGUK_NUM				= TAB_PYOGUK_NUM * TABCELL_PYOGUK_NUM,//150

	SLOT_QUICK_NUM				= TAB_QUICK_NUM * TABCELL_QUICK_NUM,//11

	SLOT_ABILITY_NUM			= TAB_ABILITY_NUM * TABCELL_ABILITY_NUM,

	SLOT_QITEMINVEN_NUM			= TAB_QITEMINVEN_NUM * TABCELL_QITEMINVEN_NUM,//100
	
	SLOT_GUILDWAREHOUSE_NUM		= TAB_GUILDWAREHOUSE_NUM * TABCELL_GUILDWAREHOUSE_NUM,	//60

	SLOT_SHOPITEM_NUM			= TAB_SHOPITEM_NUM * TABCELL_SHOPITEM_NUM,
	
	SLOT_SHOPINVEN_NUM			= TAB_SHOPINVEN_NUM * TABCELL_SHOPINVEN_NUM,
	SLOT_WEAR_SHOPITEM_NUM		= 10,

	SLOT_STREETSTALL_NUM		= 25,	// ����Ȯ��â���� ����.
	SLOT_STREETBUYSTALL_NUM		= 5,	
	SLOT_MUGONG_NUM				= 25,	
	SLOT_JINBUB_NUM				= 5,		// 5
	// magi82 - Titan(070910) Ÿ��ź ����������Ʈ
	SLOT_TITAN_NUM				= 25,
	SLOT_MUGONGTOTAL_NUM		= SLOT_MUGONG_NUM+SLOT_MUGONG_NUM+SLOT_JINBUB_NUM+SLOT_TITAN_NUM,		// 80	
	
	SLOT_PETINVEN_NUM			= TAB_PET_INVENTORY_NUM * TABCELL_PETINVEN_NUM,	//60
	SLOT_PETWEAR_NUM			= 3,

	SLOT_NPCINVEN_NUM			= 30,

	SLOT_TITANWEAR_NUM		= 7,

	SLOT_TITANSHOPITEM_NUM	= 4,

	SLOT_TITANMUGONG_NUM	= 25,	// magi82 - Titan(070910) Ÿ��ź ����������Ʈ

	// ���� & ���â��
	SLOT_SEIGEFORTWARE_NUM		= 90,
};

// < ��E?��I����A�ˢ�E > : taiyo
// TP_INVENTORY_START��E��IA			��IoY��Iia����oA = 0
// TP_MUNPAWAREHOUSE_START��E��IA	��IoY��Iia����oA = TP_SIMBUB_END
// TP_MAX��E��IA						��IoY��Iia����oA = TP_PYOGUK_END,
/*
enum
{
	//ITEMTABLE��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_INVENTORY_START			= 0,
	TP_INVENTORY_END			= TP_INVENTORY_START		+ SLOT_INVENTORY_NUM,	// 80
	TP_WEAR_START				= TP_INVENTORY_END,									// 80
	TP_WEAR_END					= TP_WEAR_START				+ SLOT_WEAR_NUM,		// 90

	TP_PYOGUK_START				= TP_WEAR_END,										// 90
	TP_PYOGUK_END				= TP_PYOGUK_START			+ SLOT_PYOGUK_NUM,		// 240

	//ItemTableAI��E?U��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_MUNPAWAREHOUSE_START		= TP_PYOGUK_END,										// 240
	TP_MUNPAWAREHOUSE_END		= TP_MUNPAWAREHOUSE_START	+ SLOT_MUNPAWAREHOUSE_NUM,	// 555

	TP_MUGONG1_START			= TP_MUNPAWAREHOUSE_END,								// 555
	TP_MUGONG1_END				= TP_MUGONG1_START			+ SLOT_MUGONG_NUM,			// 580
	
	TP_MUGONG2_START			= TP_MUGONG1_END,										// 580
	TP_MUGONG2_END				= TP_MUGONG2_START			+ SLOT_MUGONG_NUM,			// 605

	TP_QUICK_START				= TP_MUGONG2_END,										// 605
	TP_QUICK_END				= TP_QUICK_START			+ SLOT_QUICK_NUM,			// 645

	TP_ABILITY_START			= TP_QUICK_END,											// 645
	TP_ABILITY_END				= TP_ABILITY_START			+ SLOT_ABILITY_NUM,			// 805

	TP_SHOPITEM_START			= TP_PYOGUK_END,										// 240
	TP_SHOPITEM_END				= TP_SHOPITEM_START			+ SLOT_SHOPITEM_NUM,		// 390

	TP_SHOPINVEN_START			= TP_SHOPITEM_END,										// 390
	TP_SHOPINVEN_END			= TP_SHOPINVEN_START		+ SLOT_SHOPINVEN_NUM,		// 430
	
//	TP_MAX						= TP_PYOGUK_END + SLOT_SHOPITEM_NUM + SLOT_SHOPINVEN_NUM,	// 240+150+40=430
	TP_MAX						= TP_PYOGUK_END + SLOT_SHOPITEM_NUM + SLOT_SHOPINVEN_NUM,// + SLOT_PETINVEN_NUM + SLOT_PETWEAR_NUM,	// (430) + 60 + 3
	
	TP_GUILDWAREHOUSE_START		= TP_MAX,													// 430
	TP_GUILDWAREHOUSE_END		= TP_GUILDWAREHOUSE_START + SLOT_GUILDWAREHOUSE_NUM,		// 490

	TP_JINBUB_START				= TP_GUILDWAREHOUSE_END,									// 490
	TP_JINBUB_END				= TP_JINBUB_START			+ SLOT_JINBUB_NUM,				// 495

	TP_PETINVEN_START			= 850,
	TP_PETINVEN_END				= TP_PETINVEN_START			+ SLOT_PETINVEN_NUM,		// +60

	TP_PETWEAR_START			= TP_PETINVEN_END,
	TP_PETWEAR_END				= TP_PETWEAR_START			+ SLOT_PETWEAR_NUM,			// +3
};
*/
enum
{
	//ITEMTABLE��E?��E��c����u�ˢ硧I �ˢ�ic��E?e
	TP_INVENTORY_START			= 0,
	TP_INVENTORY_END			= TP_INVENTORY_START		+ SLOT_INVENTORY_NUM,		// 80
	TP_WEAR_START				= TP_INVENTORY_END,										// 80
	TP_WEAR_END					= TP_WEAR_START				+ SLOT_WEAR_NUM,			// 90

	TP_PYOGUK_START				= TP_WEAR_END,											// 90
	TP_PYOGUK_END				= TP_PYOGUK_START			+ SLOT_PYOGUK_NUM,			// 240

	TP_MUNPAWAREHOUSE_START		= TP_PYOGUK_END,										// 240
	TP_MUNPAWAREHOUSE_END		= TP_MUNPAWAREHOUSE_START	+ SLOT_MUNPAWAREHOUSE_NUM,	// 555

	TP_MUGONG_START				= TP_MUNPAWAREHOUSE_END,								// 555
	TP_MUGONG_END				= TP_MUGONG_START			+ SLOT_MUGONG_NUM,			// 580

	TP_SHOPITEM_START			= TP_PYOGUK_END,										// 240
	TP_SHOPITEM_END				= TP_SHOPITEM_START			+ SLOT_SHOPITEM_NUM,		// 390

	TP_SHOPINVEN_START			= TP_SHOPITEM_END,										// 390
	TP_SHOPINVEN_END			= TP_SHOPINVEN_START		+ SLOT_SHOPINVEN_NUM,		// 430

	TP_PETINVEN_START			= TP_SHOPINVEN_END,										// 430
	TP_PETINVEN_END				= TP_PETINVEN_START			+ SLOT_PETINVEN_NUM,		// 490

	TP_PETWEAR_START			= TP_PETINVEN_END,										// 490
	TP_PETWEAR_END				= TP_PETWEAR_START			+ SLOT_PETWEAR_NUM,			// 493

	TP_TITANWEAR_START			= TP_PETWEAR_END,										// 493
	TP_TITANWEAR_END			= TP_TITANWEAR_START		+ SLOT_TITANWEAR_NUM,		// 500

	TP_TITANSHOPITEM_START		= TP_TITANWEAR_END,										// 500
	TP_TITANSHOPITEM_END		= TP_TITANSHOPITEM_START	+ SLOT_TITANSHOPITEM_NUM,	// 504

	TP_MAX						= TP_TITANSHOPITEM_END,									// 504
//	TP_MAX						= TP_PYOGUK_END + SLOT_SHOPITEM_NUM + SLOT_SHOPINVEN_NUM,// + SLOT_PETINVEN_NUM + SLOT_PETWEAR_NUM,	// (430) + 60 + 3

	TP_GUILDWAREHOUSE_START		= 510,													// 510		// 430
	TP_GUILDWAREHOUSE_END		= TP_GUILDWAREHOUSE_START + SLOT_GUILDWAREHOUSE_NUM,	// 570		// 490
	
	TP_MUGONG1_START			= 600,													// 600		// 555
	TP_MUGONG1_END				= TP_MUGONG1_START			+ SLOT_MUGONG_NUM,			// 625		// 580

	TP_MUGONG2_START			= TP_MUGONG1_END,										// 625		// 580
	TP_MUGONG2_END				= TP_MUGONG2_START			+ SLOT_MUGONG_NUM,			// 650		// 605

	TP_JINBUB_START				= TP_MUGONG2_END,										// 650		// 490
	TP_JINBUB_END				= TP_JINBUB_START			+ SLOT_JINBUB_NUM,			// 655		// 495

	// magi82 - Titan(070910) Ÿ��ź ����������Ʈ
	TP_TITANMUGONG_START		= TP_JINBUB_END,										// 655
	TP_TITANMUGONG_END			= TP_TITANMUGONG_START		+ SLOT_TITANMUGONG_NUM,		// 680

	TP_QUICK_START				= 700,													// 700		// 605
	TP_QUICK_END				= TP_QUICK_START			+ SLOT_QUICK_NUM,			// 740		// 645

	TP_ABILITY_START			= TP_QUICK_END,											// 745		// 645
	TP_ABILITY_END				= TP_ABILITY_START			+ SLOT_ABILITY_NUM,			// 905		// 805


	// ���� & ���
	TP_SIEGEFORTWARE_START		= 10000,
	TP_SIEGEFORTWARE_END		= TP_SIEGEFORTWARE_START + SLOT_SEIGEFORTWARE_NUM,			// 10090
};

#endif // _JAPAN_LOCAL_

enum eITEMTABLE
{
	eItemTable_Inventory,
	eItemTable_Weared,
	eItemTable_Pyoguk,
	eItemTable_Shop,
	eItemTable_ShopInven,
	eItemTable_PetInven,
	eItemTable_PetWeared,
	eItemTable_Titan,
	eItemTable_TitanShopItem,	// magi82 - Titan(070227)
	eItemTable_TableMax,
	eItemTable_MunpaWarehouse,

	eItemTable_StreetStall,
	eItemTable_Exchange,
	eItemTable_Mix,
	eItemTable_RecvExchange,
	
	eItemTable_Mugong,
	eItemTable_Quick,
	eItemTable_Deal,
	eItemTable_Ability,

	eItemTable_Max,
};

//temp
#define eItemTable_GuildWarehouse eItemTable_MunpaWarehouse

enum MONEY_FLAG {
	MF_NOMAL = 0, MF_OBTAIN, MF_LOST, MF_NONE, MF_PUTINGUILD = 20, MF_PUTOUTGUILD
};



enum EMAP_NUMBER
{
	eMapNum_BukKyung_Town,
	eMapNum_BukKyung_Field,
	eMapNum_BukKyung_Dungeon,

	eMapNum_NakYang_Town,
	eMapNum_NakYang_Field,
	eMapNum_NakYang_Dungeon,

	eMapNum_GaeBong_Town,
	eMapNum_GaeBong_Field,
	eMapNum_GaeBong_Dungeon,
		
	eMapNum_JangAn_Town,
	eMapNum_JangAn_Field,
	eMapNum_JangAn_Dungeon,
	
	eMapNum_Max,
};

enum EAPPEAR_PART
{
	eAppearPart_Hair,
	eAppearPart_Face,
	eAppearPart_Body,
	eAppearPart_Hand,
	eAppearPart_Foot,
	eAppearPart_Weapon,
	
	eAppearPart_Max
};

// ---- ��Io�ˢ�i�ˢ硧����I����A�ˢ�E ����u���סˢ�AIAU AI��Ii|����o���ע��� ����u��I�Ϩ�d -----------------------------
#define MIN_MUGONG_INDEX		100
#define MAX_MUGONG_INDEX		4000
#define MIN_NAEGONG_INDEX		6000
#define MAX_NAEGONG_INDEX		7000
#define MIN_KYUNGGONG_INDEX		7000
#define MAX_KYUNGGONG_INDEX		8000
//----- ����u����u���ע���I ��Io�ˢ�i�ˢ硧����I����AI��Ii|����o���ע��� ����u��I�Ϩ�d ----------------------------------
#define MIN_ATTACK_MUGONG_INDEX		100
#define MAX_ATTACK_MUGONG_INDEX		2000
#define MIN_DEFENSE_MUGONG_INDEX	2000
#define MAX_DEFENSE_MUGONG_INDEX	3000
#define MIN_ASSIST_MUGONG_INDEX		3000
#define MAX_ASSIST_MUGONG_INDEX		4000
#define ATTACK_MUGONG_INTERVAL		200
#define DEFENSE_MUGONG_INTERVAL		100
#define ASSIST_MUGONG_INTERVAL		100

#define MIN_GRIDITEM_AREA		0
#define MAX_GRIDITEM_AREA		1000


// ---- AI���ע����ˢ�I���סˢ碮Ec��E������EcA�ˢ�E ����u���סˢ�AIAU AI��Ii|����o���ע��� ����u��I�Ϩ�d -----------------------------
#define MIN_YOUNGYAKITEM_INDEX	8000
#define MAX_YOUNGYAKITEM_INDEX	9000
#define MIN_MUGONGITEM_INDEX	9000
#define MAX_MUGONGITEM_INDEX	11000
#define MIN_EQUIPMENTITEM_INDEX 11000
#define MAX_EQUIPMENTITEM_INDEX 32000
// ---- AbilityIndex
#define MIN_ABILITY_INDEX	60000
#define MAX_ABILITY_INDEX	61000


#define WALKSPEED		200
#define RUNSPEED		400


#define MOVE_LIMIT_ERRORDISTANCE	1000

#define TEMPITEMDBIDX 0xffffffff
#define MAX_SERVERINFO_LIST		100

#define MAX_NAME_LENGTH				16
#define MAX_MONSTER_NAME_LENGTH		60
#define MAX_CHAT_LENGTH				127
#define	MAX_CHANNELTITLE_LENGTH		64
#define	MAX_PLAYROOMTITLE_LENGTH	64
#define MAX_ITEMNAME_LENGTH			32
#define MAX_PETNAME_LENGTH			32		//�ӽ÷� ��������	

#define MAX_CHXNAME_LENGTH			32

//#define MAX_CHARACTER_NUM			4
#define MIN_GRIDITEM_AREA			0
#define MAX_GRIDITEM_AREA			1000
 
#define MAX_PICKITEM_DISTANCE	500

#define MAX_AUCTIONBOARD_DISTANCE 500


#define MAX_BOARDTITLE_NUM	10
#define MAX_BOARDTITLE_LENGTH 64
#define MAX_CONTENTS_LENGTH	65535


#define MAX_TARGET_NUM		300

//-----��Io�ˢ�i�ˢ硧����I���� �ˢ硧����I���̡ˢ硧��Y--------------------------------------
enum {GENERAL_ATTACK=0,MUGONG_ATTACK=1};
//#define GENERAL_ATTACK_DISTANCE		200
#define POINT_VALID_DISTANCE		3000			// 10m: ���סˢ�ˢ�AAI���סˢ碮Ec E��Io��Ii��I�ˢ� /E�ˢ�I ��Iou'


#define REMOVEFADE_START_TIME		2500
#define MONSTERREMOVE_TIME			5000
#define PLAYERREVIVE_TIME			5000
#define	PLAYERREVIVE_TIME_IN_BOSSMAP	60000

enum BASICEFFECT
{
	eEffect_NewCharacter=1,	//A��I�Ϩ���E����?AI ��IiiAa
	eEffect_Standard,		//A��I�Ϩ���E����?AI �ˢ硧ua���ע����ˢ�i(��Io�ˢ�i�ˢ硧ua����u��I����AI)
	eEffect_Walk,	//�ˢ硧��E�ˢ硧ua (��Io�ˢ�i�ˢ硧ua ����u��I����AI)
	eEffect_Run,	//��EOU�ˢ硧ua (��Io�ˢ�i�ˢ硧ua����u��I����AI)
	eEffect_BattleWalk_Gum,		//AuAo �ˢ硧��E�ˢ硧ua(�ˢ硧��E_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleWalk_Gwun,		//AuAo �ˢ硧��E�ˢ硧ua(�ˢ硧uC��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleWalk_Do,		//AuAo �ˢ硧��E�ˢ硧ua(��Ii��Ii_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleWalk_Chang,		//AuAo �ˢ硧��E�ˢ硧ua(A�ˢ�E_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleWalk_Gung,		//AuAo �ˢ硧��E�ˢ硧ua(�ˢ硧uA_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleWalk_Amgi,		//AuAo �ˢ硧��E�ˢ硧ua(����uI�ˢ硧ua_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleRun_Gum,		//AuAo ��EOU�ˢ硧ua(�ˢ硧��E_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleRun_Gwon,		//AuAo ��EOU�ˢ硧ua(�ˢ硧uC_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleRun_Do,		//AuAo ��EOU�ˢ硧ua(��Ii��Ii_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleRun_Chang,		//AuAo ��EOU�ˢ硧ua(A�ˢ�E��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleRun_Gung,		//AuAo ��EOU�ˢ硧ua(�ˢ硧uA_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_BattleRun_Amgi,		//AuAo ��EOU�ˢ硧ua(����uI�ˢ硧ua_��Io�ˢ�i�ˢ硧uaAaA��I����)
	eEffect_UngijosikStart,	//��E?i�ˢ硧uav����oA(����uE��E��IA��Ii��E?AU)
	eEffect_Ungijosik,	//��E?i�ˢ硧uav����oA
	eEffect_Ungijosik_End,	//��E?i�ˢ硧uav����oA (��I�Ϩ���E��c��I�Ϩ��ˢ硧IEA)
	eEffect_UngijosikDamage,	//��E?i�ˢ硧uav����oA A��I������E?��E��c ��Ii�ˢ�I��IoIAo
	eEffect_StallStart,	//��I�Ϩ�ea�ˢ�io ��Ii��E?AU1 (����uE�ˢ硧ua)
	eEffect_Stall,	//��I�Ϩ�ea�ˢ�io ��Ii��E?AU1 (����uE����u���סˢ� AO�ˢ硧ua)
	eEffect_StallEnd,	//��I�Ϩ�ea�ˢ�io ��Ii��E?AU1 (����u�ˢ硧I�ˢ硧ua)
	eEffect_Damage1,		//��Ii�ˢ�I��IoIAo1 (AA)
	eEffect_Damage2,		//��Ii�ˢ�I��IoIAo2 (��E?i)
	eEffect_Damage3,		//��Ii�ˢ�I��IoIAo3 (EA)
	eEffect_Die1,		//A�ˢ��?= 1
	eEffect_Died1,		//A�ˢ��?����ui����u�ˢ硧I ��E��I����I��E?o AO��E��IA ��E������I��I����oA 1
	eEffect_Die2,		//A�ˢ��?= 2
	eEffect_Died2,		//A�ˢ��?����ui����u�ˢ硧I ��E��I����I��E?o AO��E��IA ��E������I��I����oA 2
	eEffect_LevelUpSentence,	//�ˢ�ˢ���Ii��o��Ei��u�ˢ碮������u�ˢ�A �ˢ硧uUAU
	eEffect_MouseMovePoint,	//��E������EO��E?i����o���ע��� AI��Ii��E?���סˢ�ˢ�AAI���סˢ碮Ec
	
	eEffect_GetMoney = 28,	// ��E�����ˢ�AA�ˢ��?��E?����I����u�ˢ硧I ��Ii�ˢ�ˢ� ����uo�ˢ硧ua
	eEffect_GetItem_Accessory = 30,	// ��E�����ˢ�AA�ˢ��?��E?����I����u�ˢ硧I ����u���סˢ�AIA���סˢ� ����uo�ˢ硧ua

	eEffect_Emoticon_Bow,	//AI�ˢ�ic
	eEffect_Emoticon_Thanks,	//�ˢ硧���ˢ碮���ˢ�ic
	eEffect_Emoticon_Yes,	//�ˢ硧uad 
	eEffect_Emoticon_No,	//���ע���Id
	eEffect_Emoticon_Pleasure,	//�ˢ硧ua�ˢ�iY
	eEffect_Emoticon_Sadness,	//����o����oCA
	eEffect_Emoticon_Love,	//�ˢ�ic��EOu
	eEffect_Emoticon_Hostility,	//Au��E��Ie
	eEffect_Emoticon_Disappoint,	//����oC��E����A
	eEffect_Emoticon_Angry,	//���ע������ע�E��I�Ϩ�e
	eEffect_Emoticon_Suprised,	//��I�Ϩ�i��EO�ˢ�A
	eEffect_Emoticon_Evasion,	//E��E����CC
	eEffect_Emoticon_Declare,	//����u�ˢ硧u����u��I��I
	eEffect_Defence_Gum,		//��Ii��o|i��������ui��Ii��E?AU(�ˢ硧��E)
	eEffect_Defence_Gwun,		//��Ii��o|i��������ui��Ii��E?AU(�ˢ硧uC)
	eEffect_Defence_Do,		//��Ii��o|i��������ui��Ii��E?AU(��Ii��Ii)
	eEffect_Defence_Chang,		//��Ii��o|i��������ui��Ii��E?AU(A�ˢ�E)
	eEffect_Defence_Gung,		//��Ii��o|i��������ui��Ii��E?AU(�ˢ硧uA)
	eEffect_Defence_Amgi,		//��Ii��o|i��������ui��Ii��E?AU(����uI�ˢ硧ua)

	eEffect_Mugong_GetMugong = 68,		// ���� ȹ��
	eEffect_UseItem_LifeYoungYak = 70,	// ������ ȸ��
	eEffect_UseItem_ManaYoungYak,
	eEffect_GetItem_Weapon,				//��Ii��o|i��������ui��Ii��E?AU(����uI�ˢ硧ua)
	eEffect_GetItem_Dress,
	eEffect_GetItem_Quest,
	eEffect_GetItem_Youngyak,
	// magi82(4) - Titan(071023) Ÿ��ź�϶� ����Ʈ ó��(enum�� ����)
	eEffect_Mugong_ChangeMugong,		// ���ʹ��� ȹ��
	eEffect_UseItem_HosinYoungYak,		// ���ʹ��� ȹ��
	eEffect_UseItem_FullYoungYak,		// ����ȸ������ ���

	eEffect_MonRegen = 80,
	eEffect_PK_LootExp,					// PK ���� ����ġ
	eEffect_ChangeStage_Hwa = 98,
	eEffect_ChangeStage_Geuk = 99,

	eEffect_KyungGong1 = 200,			// ���1
	eEffect_KyungGong2 = 203,			// ���2
	eEffect_KyungGong3 = 206,			// ���3
	eEffect_Jackpot = 1034,				// ���̴�÷

	eEffect_PetSummon = 3151,			// ���ȯ
	eEffect_PetSeal		= 3152,			// �����
	eEffect_FeedUp		= 3153,			// �����
	eEffect_PetWarp		= 3154,			// �����

	eEffect_ShopItem_Life = 3200,		// ��â��	
	eEffect_ShopItem_Shield,			// �׿�û	
	eEffect_ShopItem_NaeRuyk,			// ����	
	eEffect_ShopItem_Revive,			// ��Ȱ�ֹ���
	eEffect_ShopItem_Boom1,				// ����(����)
	eEffect_ShopItem_Boom2,				// ����(��)
	eEffect_ShopItem_Hair1,				// ����1	
	eEffect_ShopItem_Hair2,				// ����2	
	eEffect_ShopItem_Hair3,				// ����3	
	eEffect_ShopItem_Teleport1,			// �ڷ���Ʈ1
	eEffect_ShopItem_Teleport2,			// �ڷ���Ʈ2
	eEffect_ShopItem_Teleport3,			// �ڷ���Ʈ3
	eEffect_ShopItem_MaxLife,			// ������ ����
	eEffect_ShopItem_MaxShield,			// ȣ�Ű����� ����
	eEffect_ShopItem_MaxNaeRuyk,		// ������ ����

	eEffect_Titan_Damage1 = 3264,		// Ÿ��ź ������1
	eEffect_Titan_Damage2,				// Ÿ��ź ������2
	eEffect_Titan_Damage3,				// Ÿ��ź ������3
	eEffect_Titan_GetMoney = 3268,		// Ÿ��ź �� ȹ��
	eEffect_Titan_GetItem_Accessory = 3270,	// Ÿ��ź �Ǽ��縮������ ȹ��
	eEffect_TitanLevelUpSentence,			// Ÿ��ź ����������
	eEffect_Titan_GetMugong = 3308,		// Ÿ��ź ���� ȹ��
	eEffect_Titan_UseItem_LifeYoungYak = 3310,	// Ÿ��ź ������ȸ������ ���
	eEffect_Titan_UseItem_ManaYoungYak,			// Ÿ��ź ����ȸ������ ���
	eEffect_Titan_GetItem_Weapon,				// Ÿ��ź ��������� ȹ��
	eEffect_Titan_GetItem_Dress,			// Ÿ��ź �������� ȹ��
	eEffect_Titan_GetItem_Quest,			// Ÿ��ź ����Ʈ������ ȹ��
	eEffect_Titan_GetItem_Youngyak,			// Ÿ��ź ��������� ȹ��
	eEffect_Titan_ChangeMugong,				// Ÿ��ź ���ʹ��� ȹ��
	eEffect_Titan_UseItem_HosinYoungYak,	// Ÿ��ź ȣ�Ű���ȸ������ ���
	eEffect_Titan_UseItem_FullYoungYak,	// Ÿ��ź ����ȸ������ ���
	eEffect_Titan_PK_LootExp,			// Ÿ��ź pk ���� ����ġ

	eEffect_Titan_RideIn = 3624,		// Ÿ��ź ��ȯ
	eEffect_Titan_GetOff,
	eEffect_Titan_ForcedGetOff,

	eEffect_Titan_KyungGong1 = 3629,	// Ÿ��ź ���1
	eEffect_Titan_KyungGong2 = 3632,	// Ÿ��ź ���2
	eEffect_Titan_KyungGong3 = 3635,	// Ÿ��ź ���3

	eEffect_Titan_Shop_KyungGong1 = 3671,	// Ÿ��ź ���1(�������� �����)
	eEffect_Titan_Shop_KyungGong2 = 3673,	// Ÿ��ź ���2(�������� �����)
	eEffect_Titan_Shop_KyungGong3 = 3675,	// Ÿ��ź ���3(�������� �����)
};


// ��E����E AI��E�����ˢ碮�� ����u�ˢ硧u����u��I��I
enum
{
	bukkyung = 1,	//���ע���I�ˢ硧����I�ˢ�				bukkyung
	taesan,			//AA�ˢ�ie				taesan
	namdae,			//��I�Ϩ���I��A��E��IeCI			namdae
	sanhea,			//��E������E������E������EcAa����u���ע���			sanhea
	nakyang_siege,		//�ˢ硧����I���̡�Io|				nakyang_siege
	nakyang,		//��I�Ϩ��ˢ�i����uc				nakyang
	gaebong,		//�ˢ硧����I�Ϩ����ע���A				gaebong
	jungju,			//dAO				jungju
	hoangsan,		//E��I��A�ˢ�ie				hoangsan
	sungsan,		//����uO��E������I��A�ˢ�ic			sungsan

	youngmun,		//��E?e��Io��Ec����u��Ec�ˢ硧u����u			youngmun
	jangan,			//Aa����uE				jangan
	hoajung,		//E�ˢ硧IA�ˢ�iAo			hoajung
	malijangsung,			//��E��Ie����uEA����u			malijangsung
	jisi,			//A��I���̢���oAE��Ii��o?i��u���ע�I			jisi
	gaebong_siege,		//�ˢ硧��C��E�������ע�I				gaebong_siege
	nanju,			//��I�Ϩ��ˢ硧IAO				nanju
	donhoang_night,		//��Ii�ˢ�ˢ�E��I��A				donhoang
	okmun,			//��E?y��Ec�ˢ硧��u			okmun
	myungsa,		//��E����i����u���סˢ�ˢ�ie			myungsa

	hyungju,		//CuAO				hyungju
	makgogul2F,		//����uc����uc				makgogul2F
	jangsa,			//Aa�ˢ�ic				jangsa
	bukkyung_siege,		//Au���ע�����Ec				bukkyung_siege
	dongjung,		//��Ii��E?dE�ˢ�I			dongjung
	akyang,			//����uC����uc�ˢ�ˢ�c			akyang
	hangju,			//C�ˢ��?AO				hangju
	Tournament,			//����uOAO				Tournament
	sangju,			//�ˢ�ioAO				sangju
	hogu,			//E�ˢ�I�ˢ硧u��E����				hogu

	bota,			//���ע�����E����A��E�����ˢ�ie			bota
	hopo,			//E�ˢ�I���סˢ�ˢ�A				hopo
	PKEVENTMAP,			//����u���ע�����Ii��Ii				sungdo
	ami,			//����u���סˢ硧IoI�ˢ�ie			ami
	hoangha,		//E��I��ACI				hoangha
	sunyung,		//����u�ˢ硧I��I�Ϩ�c				sunyung
	guul,			//�ˢ硧��A����uo��Io�ˢ�i			guul
	hanbing,		//CN���ע���uA��Ii��I�Ϩ�a�ˢ硧uA		hanbing
	gonryun,		//�ˢ硧��i�ˢ�ˢ�u�ˢ�ie			gonryun
	tabi,			//A����uAI�ˢ�ic			tabi

	hohwa,			//E�ˢ�IE�ˢ硧IE�ˢ�I���סˢ�?			hohwa
	//ordos,			//��E?8�ˢ�I��Ii��Ii����o���ע���			ordos
	SURVIVALMAP = 42,
	daehung,		//��E��IeEi����uE�ˢ�ˢ�E			daehung
	kan,			//A��E��c�ˢ硧ua����o���ע���A�ˢ硧IAC��E��IE		kan
	uru,			//��E?i�ˢ�ˢ�c��Io�ˢ�iA��E��c			uru
	hwayuam,		//E�ˢ硧I��E?�ˢ硧���ˢ�ie			hwayuam
	bokju,			//���ע�����IoAO				bokju
	dwang,			//��Iii��E?O�ˢ硧���ˢ�E			dwang
	dery,			//��E��Ie��E������Ec���ע����ˢ�i����u���ע���			dery
	samtop,			//�ˢ�iiA����u�ˢ�ic			samtop

	aesan,			//����uO�ˢ�ˢ�U�ˢ�ie			aesan
	sussang,		//����u�ˢ硧I����oO���סˢ�C��I�Ϩ���I�Ϩ�			sussang
	whangkua,		//E��I��A�ˢ硧��u����uo���סˢ硧I���̢��סˢ�ˢ�A		whangkua
	gooknae,		//�ˢ硧u��Io��I�Ϩ��ˢ�i����u���ע���			gooknae
	jangbeak,		//Aa��Ioe���סˢ硧I���̢��סˢ�ˢ�A			jangbeak
	janggun,		//Aa�ˢ硧u���ע���AN			janggun
	sangkyung,		//�ˢ�io�ˢ硧����I�ˢ碮E?eA��Ii���ע���I		sangkyung
	dongkyung,		//��Ii��E?�ˢ硧����I�ˢ碮E?e��E?��I���̢��ע���I		dongkyung
	jisi_mid,		//A��I���̢���oAE��Ii��o?i��u���ע�IA��I��������oE���ע���I	jisi_mid

	//SURVIVALMAP = 42,
	wallachun = 60,
	RUNNINGMAP = 61,
	jangan_siege = 62,
	hoangha2	= 64,
	donhoang_day=65,		//A��I���̢���oAE��Ii��o?i��u���ע�IA��I��������oE���ע���I	donhoang_day
	
	hwanghajubyun=66,
	yongmunibgu = 67,

	taesanjungsang = 68,
	taesannorth = 69,
	sorimsa		= 70,

	BOSSMONSTER_MAP = 72,
	QuestRoom = 73,
	BOSSMONSTER_2NDMAP = 75,

	DIMENSION0 = 95,
	DIMENSION = 96,

	TITANMAP_EUNCHUN = 101,
	TITANMAP_TRAVOK	= 102,
	TITANMAP_GODDNESS = 104,
	TITANMAP_RWAIN = 105,
	//����ȱʧ���ٵ�ͼ
	TITANMAP_Central_Plain = 106,

	Nandaihe = 108,
	Huoyandong1 = 109,
	Huoyandong2 = 110,


	MAX_MAP_ENUMCOUNT,
};

enum EObjectState
{
	eObjectState_None,						// ����u���סˢ硧Io�ˢ�i�ˢ�ioAA ����u���סˢ碮E��IO..
	eObjectState_Enter,						// A��I�Ϩ���E����?AI��I�Ϩ����ע�I ��E����o����o���ע���AI�ˢ硧����E��c A��I�Ϩ�= ��IiiAaCO��EO�ˢ碮�� AC �ˢ�ioAA
	eObjectState_Move,						// AAC�ˢ�I AI��Ii��E? �ˢ�ioAA
	eObjectState_Ungijosik,					// ��E?i�ˢ硧ua �ˢ�ioAA
	eObjectState_Tactic,					// A��I���̡�Ioy ����oAAu �ˢ�ioAA
	eObjectState_Rest,						// ��E����o����o���ע���AIAC E���ע��Ϣ���oA �ˢ�ioAA
	
//	eObjectState_Speech,					// Npc��E?I ��E��IeE�ˢ硧I �ˢ�ioAA
	eObjectState_Deal,						// �ˢ�ioa Open �ˢ�ioAA

	eObjectState_Exchange,					// �ˢ硧u��I�Ϩ�E? �ˢ�ioAA
	eObjectState_StreetStall_Owner,			// ��I�Ϩ�ea�ˢ�io Open �ˢ�ioAA
	eObjectState_StreetStall_Guest,			// ��I�Ϩ�ea�ˢ�io ����uO��E��IO �ˢ�ioAA
	eObjectState_PrivateWarehouse,			// �ˢ硧����I�Ϩ�AIA�ˢ�E�ˢ硧��i 10
	eObjectState_Munpa,						//��Io��Ec���סˢ�A ��E������Ec����i��o��Ei��u���סˢ碮Ec��I�Ϩ����ע�I ��Io��Ec���סˢ�AA�ˢ�E

	eObjectState_SkillStart,				// ����o���ע���A��I�Ϩ� ����oAAu �ˢ�ioAA
	eObjectState_SkillSyn,					//
	eObjectState_SkillBinding,				//
	eObjectState_SkillUsing,				//
	eObjectState_SkillDelay,				// A���ע��Ϣ��ע�����E�����ˢ硧����E��c ��I��A�ˢ�!����a; ��EO�ˢ碮��, ��Io�ˢ�i�ˢ硧����I���̢���u��I��A�ˢ硧��i��I�Ϩ��ˢ硧IEAAC ��Iio�ˢ�ˢ���IoAI
	
	eObjectState_TiedUp_CanMove,			// �ˢ硧��a��IoU �ˢ�ioAA
	eObjectState_TiedUp_CanSkill,
	eObjectState_TiedUp,
	
	eObjectState_Die,						// A�ˢ��?: �ˢ�ioAA 20	
	
	eObjectState_BattleReady,
	eObjectState_Exit,						//KES 040707 A��I�Ϩ���E����?AI�ˢ硧����E��c ��I�Ϩ����ע�I�ˢ硧���ˢ�I��EO�ˢ碮��
	eObjectState_Immortal,					//����I��������AI, �ˡ�EAuE?, AuAa'A�ˢ� ��ii��u�����ϩ����̢���a��oA ��o��iAu��ioAA

	eObjectState_Society,					//KES A����A��I���� �좯AU
	
	eObjectState_ItemUse,

	eObjectState_TournamentReady,
	eObjectState_TournamentProcess,
	eObjectState_TournamentEnd,
	eObjectState_TournamentDead,

	eObjectState_Engrave,

	eObjectState_TitanRecall,	//Ÿ��ź ��ȯ

	eObjectState_MAX,	
};

//---�ˢ硧u��I�Ϩ�E?����u����u���ע���I�ˢ�ioAA(CoAc �ˢ�ic��E?e����uECI��E��IAA��I����)
enum eEXCHANGE_STATE
{
	eEXSTT_NONE,
	eEXSTT_APPLYING,	//�ˢ硧u��I�Ϩ�E?����oAA�ˢ�i; CN �ˢ�ioAA
	eEXSTT_APPLIED,		//�ˢ硧u��I�Ϩ�E?����oAA�ˢ�i; ��Io���ע���: �ˢ�ioAA
	eEXSTT_EXCHANGEING,	//�ˢ硧u��I�Ϩ�E?; A��I����CaA��I����AI �ˢ�ioAA : eObjectState_Exchange

	eEXSTT_MAX,
};



// �ˢ硧���ˢ�E~ ID��E?��E��c ��E��IeCN [6/2/2003]
#define EVENT_PET_ID_START		0x6fffffff
#define PET_ID_START			0x3fffffff
#define STATICNPC_ID_START		0xa0000000
#define STATICNPC_ID_MAX		0x10000000
#define TACTICOBJECT_ID_START	0xb0000000
#define TACTICOBJECT_ID_MAX		0x10000000
#define SKILLOBJECT_ID_START	0xc0000000
#define SKILLOBJECT_ID_MAX		0x0000ffff
#define BATTLE_ID_START			0xd0000000
#define BATTLE_ID_MAX			0x0000ffff
#define TEMP_SKILLOBJECT_ID		0xffffffff
#define BATTLE_TEAMEFFECT_ID	0xfffffffe
#define BATTLE_STAGEEFFECT_ID	0xfffffffd
#define STATE_EFFECT_ID			0xfffffffc
#define PK_EFFECT_ID			0xfffffffb
#define NPCMARK_1_ID			0xfffffffa		// !
#define NPCMARK_2_ID			0xfffffff9		// ?
#define PARTYWAR_STAGEEFFECT_ID	0xfffffff8
#define BATTLE_GTTEAM1_EFFECTID	0xfffffff7
#define BATTLE_GTTEAM2_EFFECTID	0xfffffff6
#define JACKPOT_EFFECTID		0xfffffff5
#define ENGRAVE_EFFECTID		0xfffffff4
#define ENGRAVE_ACK_EFFECTID	0xfffffff3
#define ENGRAVE_NACK_EFFECTID	0xfffffff2
#define LIFE_RECOVER_EFFECTID	0xfffffff1
#define MANA_RECOVER_EFFECTID	0xfffffff0
#define SHIELD_RECOVER_EFFECTID	0xffffffef
#define ALL_RECOVER_EFFECTID	0xffffffee
// �Ʒ��� ����� ���� ���� ����Ʈ�� ���̹Ƿ� ������ ������ ����ϱ� �ٶ��ϴ�.
#define SETITEM_STARTEFFECTID	0xf0000000		//2007. 8. 7. CBH - ��Ʈ������ ����Ʈ �߰�
//
#define TITAN_MAINTAIN_EFFECTID	0xff000000
#define TITAN_RECALL_EFFECTID	0xffff0000		//2007. 10. 24. CBH - Ÿ��ź ��ȯ ����Ʈ
#define KYUNGGONG_EFFECTID		0xffffff00		// magi82(11) - Effect(071025) ��� ����Ʈ ID �ο�

#ifdef _JAPAN_LOCAL_
#define MUSSANG_EFFECT_ID		0xffffffed
#else
// 06. 03 �������� - �̿���
#define MUSSANG_EFFECT_ID1		0xffffffed
#define MUSSANG_EFFECT_ID2		0xffffffec
#define MUSSANG_EFFECT_ID3		0xffffffeb
#endif

#define DETECT_EFFECT_ID		0xffffffea

#define MAX_SKILLMAINTAIN_EFFECT	3

// ��Io��I�ˢ�Ca AI��Ii|����i��o��Ei��u��E?��E��c ��E��IeCN ����u��I�Ϩ�d
typedef BYTE DIRINDEX;
#define MAX_DIRECTION_INDEX		32
#define DEGTODIR(angleDeg)	(BYTE)((angleDeg)/360*MAX_DIRECTION_INDEX)
#define DIRTODEG(DirIndex)	(((float)DirIndex)*360/MAX_DIRECTION_INDEX)
#define DIRTORAD(DirIndex)	(DEGTORAD(DIRTODEG(DirIndex)))
#define RADTODIR(angleRad)	(DEGTODIR(RADTODEG(angleRad)))

// A�ˢ�i��E�������ע��Ϣ�EOo ����u��I�Ϩ�d
#define CAMERA_NEAR		100
#define MAX_CHARTARGETPOSBUF_SIZE	15

//system
#define MAX_IPADDRESS_SIZE	16

// type : 0->����oA�ˢ硧���ˢ�I��EO�ˢ碮�����ע����ˢ硧�� ��I�Ϩ���I��A�ˢ硧ue. 1~100->AId�ˢ�ˢ���Ec AI�ˢ�io���ע���?E�ˢ硧I����oA ��I�Ϩ���I��A�ˢ硧ue(�ˢ硧����E��c�ˢ�ie), 101~200->(�ˢ硧���ˢ碮���ˢ�ie)
// 1->�ˢ�ioa��E?��E��c ���סˢ�C��E����A, 2->��I�Ϩ�ea��E?��E��c����u�ˢ硧I ���סˢ�C��E����A, 3->��E����o����o���ע���AI��E?��E��c�ˢ硧��O E��Io��Ii��I�ˢ�, 4->�ˢ硧u��I�Ϩ�E?, 5->A�ˢ�E�ˢ硧��i��E?��E��c����u�ˢ硧I �ˢ�i�ˢ�ϡ���EO�ˢ碮��, 6->��Io��Ec���סˢ�AA�ˢ�E�ˢ硧��i��E?��E��c����u�ˢ硧I �ˢ�i�ˢ�ϡ���EO�ˢ碮��
// 101->�ˢ�ioa��E?��E��c����u�ˢ硧I �ˢ�ii��EO�ˢ碮��, 102->��I�Ϩ�ea��E?��E��c����u�ˢ硧I �ˢ�ii��EO�ˢ碮��, 103->A�ˢ��?����uu;��EO�ˢ碮��, 104->�ˢ硧u��I�Ϩ�E?, 105->A�ˢ�E�ˢ硧��i��E?��E��c ��I�Ϩ�O=, 106->��Io��Ec���סˢ�AA�ˢ�E�ˢ硧��i��E?��E��c ��I�Ϩ�O=, 107->C�ˢ�I�ˢ硧u��Io �ˢ�ii��EO�ˢ碮��
enum eMoneyLogtype
{
	eMoneyLog_Time				= 0,
// �ˢ硧����E��c�ˢ�ie
	eMoneyLog_GetStall			= 1,
	eMoneyLog_GetStreetStall	= 2,
	eMoneyLog_GetMonster		= 3,
	eMoneyLog_GetExchange		= 4,
	eMoneyLog_GetPyoguk			= 5,
	eMoneyLog_GetGuild			= 6,
	eMoneyLog_GetWantedPrize	= 7,
	eMoneyLog_GetPKLooting		= 8,
	eMoneyLog_GetSuryunFail		= 9,
	eMoneyLog_GetPrize			= 10,
	eMoneyLog_GetSWProfit		= 11,
	eMoneyLog_GetFromQuest		= 12,
	eMoneyLog_GetGMTool			= 13,
	
// �ˢ硧���ˢ碮���ˢ�ie
	eMoneyLog_LoseStall			= 101,
	eMoneyLog_LoseStreetStall	= 102,
	eMoneyLog_LoseDie			= 103,
	eMoneyLog_LoseExchange		= 104,
	eMoneyLog_LosePyoguk		= 105,
	eMoneyLog_LoseGuild			= 106,
	eMoneyLog_LosePyogukBuy		= 107,
	eMoneyLog_LoseWantedRegist	= 108,
	eMoneyLog_LoseWantedBuyRight = 109,
	eMoneyLog_LosePKLooted		= 110,
	eMoneyLog_LosePKModeExitPanelty = 111,
	eMoneyLog_LoseBadFrameBail	= 112,
	eMoneyLog_LoseSuryunFee		= 113,
	eMoneyLog_LoseGuildLevelUp	= 114,
	eMoneyLog_LoseChangeLocation	= 115,
	eMoneyLog_LoseGMTool		= 116,

	eMoneyLog_GuildCerebration	= 199,

//
	eMoneyLog_GetCheatMoney = 201,
};

enum eLogMugong
{
	eLog_MugongLearn = 0,
	eLog_MugongDiscard,
	eLog_MugongDestroyByWanted,
	eLog_MugongDestroyByBadFame,
	eLog_MugongLevelup,
	eLog_MugongCheatLevelup,
	eLog_MugongLevelDownByWanted,
	eLog_MugongLevelDownByBadFame,
	eLog_MugongDestroyByGetJulCho,
	eLog_MugongDestroyByBadFameJulChoDown,
	eLog_MugongDestroyByGetNextLevel,		// for japan

	////////////////////////////////////////////////////////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	// ���� ��ȯ
	eLog_MugongOption,
	eLog_MugongOptionClear,
	eLog_MugongDestroyByOptionClear,
	eLog_MugongOptionClearbyShopItem,
	////////////////////////////////////////////////////////////////////////////////
};

enum eLogitemmoney
{
	eLog_ItemDiscard = 1,
	
	//item move
	eLog_ItemMoveInvenToPyoguk = 100,
	eLog_ItemMovePyogukToInven,
	eLog_ItemMoveInvenToGuild,
	eLog_ItemMoveGuildToInven,
	eLog_ItemMoveGuildToGuild,
	eLog_ItemMoveInvenToMunpa,
	eLog_ItemMoveMunpaToInven,
	eLog_ItemMoveMunpaToMunpa,
	
	//item obtain/destory
	eLog_ItemObtainBuy = 200,
	eLog_ItemObtainDivide,
	eLog_ItemObtainCheat,
	eLog_ItemObtainMonster,
	eLog_ItemObtainQuest,
	eLog_ItemObtainPK,
	eLog_ItemObtainDissolution,
	eLog_ItemObtainFromChangeItem,
	eLog_ItemRareObtainCheat,
	eLog_ItemOptionObtainCheat,
	eLog_ItemObtainTitan,	// magi82 - Titan(070130)
	eLog_ItemObtainFromGameEvent,

	eLog_ItemSell = 300,
	eLog_ItemUse,
	
	eLog_StreetStallBuyAll = 400,
	eLog_StreetStallBuyDivide,
	eLog_StreetStallSellDivide,

	eLog_Exchange = 500,

	eLog_ItemReinforceSuccess = 600,
	eLog_ItemReinforceFail,
	eLog_ItemReinforceSuccessWithShopItem,
	eLog_ItemReinforceFailWithShopItem,
	eLog_ItemReinforceProtectWithShopItem,

	eLog_ItemMixSuccess = 700,
	eLog_ItemMixFail,
	eLog_ItemMixBigFail,
	eLog_ItemRareMixFail,
	eLog_ItemRareMixBigFail,

	eLog_ItemDestroyMix = 800,
	eLog_ItemDestroyReinforce,
	eLog_ItemDestroyUpgrade,
	eLog_ItemDestroyByWanted, // eLog_ItemDestroyByBadFame��| AIAEA����I ������A����Ao
	eLog_ItemDestroyByBadFame, //die panelty with bad fame.
	eLog_ItemDestroyDissolution, // dissolve base item
	eLog_ItemDestroyGiveQuest, // by complete quest 
	eLog_ItemDestroyDeleteQuest,
	eLog_ItemDestroybyChangeItem,
	eLog_ItemDestroyReinforceWithShopItem,

	eLog_MoneyObtainPK = 1000,
	//exp obtain
	eLog_ExpObtainPK,

	eLog_ShopItemUse = 1500,	
	eLog_ShopItemUseEnd,
	eLog_ShopItemMoveToInven,
	eLog_ShopItemProtectAll,
	eLog_ShopItemSeal,
	eLog_ShopItem_ReinforceReset,
	eLog_ShopItemGetCheat,
	eLog_ShopItemStatsChange,

	eLog_ShopItemUse_MixSuccess=1600,
	
	eLog_RMTool = 2000,
	
	eLog_ItemMoveInvenToPetInven = 2100,
	eLog_ItemMovePetInvenToInven,

	eLog_ItemObtainFromFortWarWare = 2200,
	eLog_ItemObtainFromSeigeWarWare,

	eLog_Max,
};

enum eLogExppoint
{
	eExpLog_Time,
		
	eExpLog_LevelUp = 10,

	eExpLog_LosebyBadFame = 50, //reduce 
	eExpLog_LosebyReviveLogIn,
	eExpLog_LosebyRevivePresent,
	eExpLog_LosebyLooting,
	eExpLog_LosebyBreakupGuild,
	eExpLog_LosebyReviveExpProtect,
	eExpLog_LosebyReviveReviveOther,
	eExpLog_LosebyGuildFieldWar,
	eExpLog_LosebyExitStateDIE,

	eExpLog_GetbyQuest = 100,
	eExpLog_SExpGetbyQuest = 101,
	
	eExpLog_ProtectExp = 150,
	eExpLog_ReviveExp,
	eExpLog_ReviveExpPeriod,
	eExpLog_NoActionPanelty_Login,
	eExpLog_NoActionPanelty_Present,
    eExpLog_NoActionPanelty_Village,
	eExpLog_NoActionPanelty_Exit,
};

enum eLogToolType	// Tool A����u
{
	eToolLog_GM = 1,
	eToolLog_RM,
	eToolLog_Server,
};

enum eGMLogKind		// GMTool ��I���� A����u
{
	eGMLog_LogIn = 1,
	eGMLog_Move,	
	eGMLog_Item,
	eGMLog_Money,
	eGMLog_Summon,
	eGMLog_SummonDelete,
	eGMLog_MonsterKill,
	eGMLog_PKAllow,
	eGMLog_Disconnect_Map,
	eGMLog_Disconnect_User,
	eGMLog_Block,
	eGMLog_Event,
	eGMLog_Jackpot,
	eGMLog_PlusTime,

	eGMLog_Skill,
	eGMLog_Ability,
	eGMLog_Chat_Map,
	eGMLog_Chat_User,
	eGMLog_PK,
	eGMLog_Regen,
};

enum eRMLogKind		// RMTool ��I���� A����u
{
	eRMLog_OperInsert = 1,
	eRMLog_OperDelete,
	eRMLog_IpInsert,
	eRMLog_IpDelete,
	/// 060829 PKH - gm�� �������� ���
	eRMLog_GMIpInsert,
	/// 060829 PKH - gm�� ��� �ƾ��� ����
	eRMLog_GMIpDelete,

	eRMLog_ChangeCharacterInfo = 100,
	eRMLog_ChangeWareInfo,
	eRMLog_SkillInsert,
	eRMLog_SkillDelete,
	eRMLog_SkillUpdate,

	eRMLog_AbilityUpdate,

	eRMLog_QuestDelete,
	
	eRMLog_ChangeLoginPoint,
	
	eRMLog_ChangeItemPosition,
	eRMLog_ChangeWareItemPosition,
	eRMLog_ChangeSkillPosition,

	eRMLog_ItemInsert = 200,
	eRMLog_ItemDelete,
	eRMLog_ItemUpdate,	

	eRMLog_ChangeUserLevel = 300,
	eRMLog_RecoverCharacter,

	eRMLog_ChangePetInfo = 400,			//06.08.04 - pkh [��]rm���α� ���
};

enum ePetLogType
{
	//ePetLog_Regular,
	ePetLog_MasterDie,
	ePetLog_PetDie,
	ePetLog_PetRevival,
	ePetLog_Exchange,
	ePetLog_GradeUp,
	ePetLog_UpGradeFailed,
	ePetLog_StaminaZero,
	ePetLog_StaminaFromZero,
};


// �����ʸ�Ʈ �α�
enum eGTLogKind
{
	eGTLogKind_None=0,
	eGTLogKind_Regist,				// ���
	eGTLogKind_Win,					// ��
	eGTLogKind_Lose,				// ��
};


// ������ �α�
enum eSWLogKind
{
	eSWLogKind_None=0,
	eSWLogKind_DefenceProposal,			// �������� ��û
	eSWLogKind_Attack,					// ��������
	eSWLogKind_Defence,					// ������ ����
	eSWLogKind_DefenceAccept,			// �������� ������� �������� ����
	eSWLogKind_DefenceUnion,			// ���ͰἺ���� �������� ����
	eSWLogKind_CancelByCastleGuild,		// �����İ� ����Ż��� �ٸ��������� �������� ���
	eSWLogKind_CancelByUnionSecession,	// �����Ŀ��� ���� Ż��� �������� ���
	eSWLogKind_StartSiegeWar,			// ��������
	eSWLogKind_SucceedCastleGuild,		// ������������ ���� ������ ����
	eSWLogKind_SucceedMoveToDefence,	// ������������ ���� ������ ������ ������ �������� �̵�
	eSWLogKind_SucceedAttack,			// �������� ������ ���� �״���� ����
	eSWLogKind_SucceedMoveToAttack,		// �������� ������������ ����->�������� �Űܰ� ����
	eSWLogKind_EndCastleGuild,			// ���������� ���� ������ ����
	eSWLogKind_EndDefenceUnionGuild,	// ���������� �����̾ �������� ��ϵǴ� ����
	eSWLogKind_EngraveSyn,				// ���νõ�
	eSWLogKind_EngraveNack,				// ���ν��� �¾� ����
	eSWLogKind_EngraveNackCancel,		// ���ν��� ���
	eSWLogKind_EngraveNackLogOut,		// ���ν��� �α׾ƿ�
	eSWLogKind_SiegeWarSucceed,			// ���μ���
	eSWLogKind_EndSiegeWar,				// ��������
};


// Au�ϩ�I
#define MAX_JOURNAL_ITEM			100
#define JOURNALVIEW_PER_PAGE		5

enum 
{
	eJournal_Quest,
	eJournal_Wanted,
	eJournal_Levelup,
	
	eJournal_Update,
	eJournal_Delete,
};
enum
{
	eJournal_Wanted_Doing,
	eJournal_Wanted_Succeed,
	eJournal_Wanted_FailbyOther, // other player completed
	eJournal_Wanted_FailbyDelChr, //wanted player is deleted
		
	eJournal_MurderedbyChr, //murder by PK Player
	eJournal_Wanted_FailbyBeWantedChr, // player is registed wanted
	eJournal_Wanted_FailbyTime, // destroy wanted info by time 
	eJournal_Wanted_FailbyDie, // die by wanted player cause wanted right can use one time.
};


// ¦®¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¯
// ¦­ÁÖ¹®¼­ ¾Æ?ÌÅÛ  RaMa - 04.11.09                            ¦­
// ¦±¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦°
enum eShopItemIncantation
{
	eIncantation_mugiup,
	eIncantation_armorup,
	eIncantation_mapmove,
	eIncantation_backhome,
	eIncantation_recovery,
};


//-----------------------------------------------------------------------
// PreLoadData
//-----------------------------------------------------------------------

#define MAX_KIND_PERMAP		150

enum
{
	ePreLoad_Monster,
	ePreLoad_Item,

	ePreLoad_Max,
};


struct sPRELOAD_INFO
{
	int		MapNum;
	int		Count[ePreLoad_Max];
	int		Kind[ePreLoad_Max][MAX_KIND_PERMAP];

	sPRELOAD_INFO()
	{
		MapNum		= 0;
		memset(Count, 0, sizeof(int)*ePreLoad_Max);
		memset(Kind, 0, sizeof(int)*ePreLoad_Max*MAX_KIND_PERMAP);
	}
};

// �����ˡ���u��c����Y
//#define BAIL_PRICE				1000
//#define MIN_BADFAME_FOR_BAIL	100
// �Ǹ�ġ ���� ����
#define BAIL_PRICE				10000
#define MIN_BADFAME_FOR_BAIL	100//0 100����Ǯϴ��������Сֵ

//
#define MAX_ALLOWANCE	10



//-----------------------------------------------------------------------
// MiniMap
//-----------------------------------------------------------------------

enum
{
	eMINIMAPMODE_SMALL,
	eMINIMAPMODE_FULL,

	eMINIMAPMODE_MAX,
};
enum
{
	eMNMICON_SELF,
	eMNMICON_PARTYMEMBER,
	eMNMICON_LOGIN,
	eMNMICON_MAPCHANGE,
	eMNMICON_WEAPON,
	eMNMICON_DRESS,
	eMNMICON_DOCTOR,
	eMNMICON_BOOK,
	eMNMICON_WAREHOUSE,
	eMNMICON_PYOGUK,
	eMNMICON_MUNPA,
	eMNMICON_YEONMU,
	eMNMICON_CASTLEMANAGER,

	eMNMICON_QUEST_EXCLAMATION1,
	eMNMICON_QUEST_EXCLAMATION2,
	eMNMICON_QUEST_EXCLAMATION3,

	eMNMICON_CAMERA,

	eMNMICON_MAX,
};

enum eBIGMAP_ICON
{
	eBMICON_SELF,
	eBMICON_PARTYMEMBER,
	eBMICON_LOGIN,
	eBMICON_MAPCHANGE,
	eBMICON_WEAPON,
	eBMICON_DRESS,
	eBMICON_DOCTOR,
	eBMICON_BOOK,
	eBMICON_WAREHOUSE,
	eBMICON_PYOGUK,
	eBMICON_MUNPA,
	eBMICON_YEONMU,
	eBMICON_ACCESSARY,
	eBMICON_GWANJOL,
	eBMICON_CASTLEMANAGER,
	eBMICON_ETC,

	eBMICON_QUEST_EXCLAMATION1,
	eBMICON_QUEST_EXCLAMATION2,
	eBMICON_QUEST_EXCLAMATION3,

	eBMICON_CAMERA,

	eBMICON_MAX,
};

#define MAX_GUILD_NAME 16
#define MAX_GUILD_INTRO		60
// 06. 03. ���İ��� - �̿���
#define MAX_GUILD_NOTICE	150
#define MAX_GUILD_UNION		7
#define MAX_GUILD_NICKNAME	16
#define MAX_MASTERNAME_LENGTH	32

#define GUILD_MASTER		50
#define GUILD_VICEMASTER	40
#define GUILD_SENIOR		30
#define GUILD_MEMBER		20
#define GUILD_STUDENT		10
#define GUILD_NOTMEMBER		0

#define GUILD_1LEVEL		1
#define GUILD_2LEVEL		2
#define GUILD_3LEVEL		3
#define GUILD_4LEVEL		4
#define GUILD_5LEVEL		5

#define MAX_GUILD_LEVEL		5
#define MAX_GUILD_STEP		5

#define GUILD_CREATELEVEL	15

#ifdef _CHINA_LOCAL_
#define MAX_GUILDMEMBER_NUM 50	//�߱��� �ִ� 50��
#else
//#define MAX_GUILDMEMBER_NUM 30
#define MAX_GUILDMEMBER_NUM 55 //30 + 25(�������ϻ�)
#endif




enum
{
	eShopItem_PeneltyCountZero,
};


//-----------------------------------------------------------------------------------
// Avatar Item
//-----------------------------------------------------------------------------------
enum
{
	eAvatar_Hat=0,			// ����
	eAvatar_Hair,			// �Ӹ�
	eAvatar_Face,			// ��
	eAvatar_Mask,			// ����
	eAvatar_Glasses,		// �Ȱ�
	eAvatar_Mustache,		// ����
	eAvatar_Dress,			// ��
	eAvatar_Shoulder,		// ���
	eAvatar_Back,			// ��
	eAvatar_Shoes,			// �Ź�
	eAvatar_Effect,			// ����Ʈ
	eAvatar_Hand,			// ��

	eAvatar_Weared_Hair,	// �Դ� ������ �Ӹ�
	eAvatar_Weared_Face,	// �Դ� ������ ��
	eAvatar_Weared_Hat,		// �Դ� ������ �Ӹ���	
	eAvatar_Weared_Dress,	// �Դ� ������ ��
	eAvatar_Weared_Shoes,	// �Դ� ������ �Ź�	
	eAvatar_Weared_Gum,		// ���빫�� ��
	eAvatar_Weared_Gwun,	// ���빫�� ��
	eAvatar_Weared_Do,		// ���빫�� ��
	eAvatar_Weared_Chang,	// ���빫�� â
	eAvatar_Weared_Gung,	// ���빫�� ��
	eAvatar_Weared_Amgi,	// ���빫�� �ϱ�	
    
	eAvatar_Max,
};

enum
{
	eAvatar_NotUse=0,
	eAvatar_Use,	
};

enum
{
	eAvatar_Attach_Head,
	eAvatar_Change_Dress,
	
};



enum
{
	eIncantation_TownMove			= 55301,		// �����̵�
	eIncantation_MemoryMove			= 55302,		// ����̵�
	eIncantation_TownMove15			= 55303,		// �����̵�15��
	eIncantation_MemoryMove15		= 55304,		// ����̵�15��
	eIncantation_TownMove7			= 57508,		// �����̵�7��
	eIncantation_TownMove7_NoTrade	= 57509,		// �����̵�7�� ��ȯ�Ұ�
	eIncantation_MemoryMove7		= 57510,		// ����̵�7��
	eIncantation_MemoryMove7_NoTrade = 57511,		// ����̵�7�� ��ȯ�Ұ�
	eIncantation_MoneyProtect		= 55311,		// ���� �ູ
	eIncantation_ExpProtect			= 55312,		// ������ �ູ
	eIncantation_ProtectAll			= 55313,		// ��ȣ���� �ֹ���
	eIncantation_StatePoint			= 55299,		// ��������Ʈ ��й�
	eIncantation_StatePoint_30		= 55321,		// ��������Ʈ ��й�(30)
	eIncantation_StatePoint_50		= 57782,		// ��������Ʈ ��й�(50)
	eIncantation_StatePoint_100		= 57783,		// ��������Ʈ ��й�(100)
	eIncantation_StatusPointReset_71	= 55392,		// ��������Ʈ �ʱ�ȭ(71~)
	eIncantation_StatusPointReset_1_50	= 55393,		// ��������Ʈ �ʱ�ȭ(1~50)
	eIncantation_StatusPointReset_51_70	= 55394,		// ��������Ʈ �ʱ�ȭ(51~70)
	eIncantation_SkillPoint_Reset	= 55391,		// ����ġ����Ʈ �ʱ�ȭ
	eIncantation_MixUp				= 55322,		// ����Ȯ�� ���
	eIncantation_OtherRevive		= 55331,		// ��Ȱ	
	eIncantation_MemoryStone		= 55601,		// ��Ｎ
//	eIncantation_SkPointRedist_2	= 55322,
	eIncantation_SkPointRedist_4	= 55323,
	eIncantation_SkPointRedist		= 55300,	
	eIncantation_ShowPyoguk			= 55351,		// â����ȯ �ֹ���15��
	eIncantation_ShowPyoguk7		= 57506,		// â����ȯ �ֹ���7��
	eIncantation_ShowPyoguk7_NoTrade = 57507,		// â����ȯ �ֹ���7�� ��ȯ�Ұ�
	eIncantation_ChangeName			= 55352,		// �̸�����
	eIncantation_Tracking			= 55353,		// ����� ����15��
	eIncantation_Tracking7			= 57504,		// ����� ����7��
	eIncantation_Tracking7_NoTrade	= 57505,		// ����� ����7�� ��ȯ�Ұ�
	eIncantation_Tracking_Jin 		= 55387,		// �������ֹ���
	eIncantation_CharChange			= 55354,		// ������ȯ�ֹ���
	eIncantation_ShapeChange		= 55355,
	eIncantation_ItemSeal			= 55356,		// ����� �ֹ���
	eIncantation_ProtectAll15		= 55358,		// ��ȣ���� �ֹ���(15��)
	eIncantation_ChangeJob			= 55360,		// ȭ��/�ظ� ���� �ֹ���
	eIncantation_MugongExtend		= 55361,		// ����â Ȯ�� �ֹ���
	eIncantation_MugongExtend2		= 57957,		// ����â Ȯ�� �ֹ���(��ȯ�Ұ�)
	eIncantation_ReinforceReset		= 55364,		// ��ȭ�ʱ�ȸ �ֹ���
	eIncantation_MemoryMoveExtend30	= 58010,		// ���âȮ�� �̵��ֹ���(30��)
	eIncantation_MemoryMoveExtend	= 55365,		// ���âȮ�� �̵��ֹ���(15��)
	eIncantation_MemoryMoveExtend7	= 55390,		// ���âȮ�� �̵��ֹ���(7��)
	eIncantation_MemoryMove2		= 55371,		// ����̵� 2��
	eIncantation_ProtectAll1		= 55372,		// ��ȣ�� 1��

	eIncantation_PetRevival			= 55382,		// �� ��Ȱ �ֹ���
	eIncantation_PetGrade2 			= 55384,		// �� ���� �ֹ��� 2�ܰ�
	eIncantation_PetGrade3 			= 55385,		// �� ���� �ֹ��� 3�ܰ�
	eIncantation_MugongOptionReset	= 55387,		// ������ȯ �ʱ�ȭ �ֹ���
	eIncantation_PyogukExtend 		= 57544,		// â��Ȯ���ֹ���	
	eIncantation_PyogukExtend2 		= 57960,		// â��Ȯ���ֹ���(��ȯ�Ұ�)
	eIncantation_CharacterSlot 		= 57543,		// ĳ���� �߰� ����
	eIncantation_CharacterSlot2 	= 57959,		// ĳ���� �߰� ����(��ȯ�Ұ�)
	eIncantation_InvenExtend 		= 57542,		// �κ��丮Ȯ�� �ֹ���
	eIncantation_InvenExtend2 		= 57958,		// �κ��丮Ȯ�� �ֹ���(��ȯ�Ұ�)
	eIncantation_ChangeName_Dntrade = 57799,		// �̸�����(�ŷ��ȵǴ�)
	eIncantation_Curse_Cancellation	= 55389,		// ����ũ������ �������� �ֹ���

	eSundries_Boom_Butterfly		= 55621,		// ����(����)
	eSundries_Boom_Star				= 55622,		// ����(��)
	eSundries_Shout					= 55631,		// ��ġ��
	eSundries_Shout_Once			= 55632,		// ��ġ��1ȸ
	eSundries_Shout_Once_NoTrade	= 57593,		// ��ġ��1ȸ �ŷ��Ұ�
	eSundries_RareItemCreate50		= 55640,
	eSundries_RareItemCreate70		= 55641,
	eSundries_RareItemCreate90		= 55642,
	eSundries_RareItemCreate99		= 63084,		//ף��ʯ,91-99������

};


enum
{
	eShopItemUseParam_Realtime = 1,				// ����ð�
	eShopItemUseParam_Playtime = 2,				// �÷��̽ð�
	eShopItemUseParam_Continue = 3,				// ����� ���� ���߿� ȿ���� �ߵ��Ǵ°�
	eShopItemUseParam_Forever = 4,				// ���� ������

	eShopItemUseParam_EquipAvatar = 10,			// ������� �ƹ�Ÿ ������
};


enum
{
	eStateItem_NotUsed,
	eStateItem_Able,
	eStateItem_UnAble,
};


enum
{
	eShopItemErr_Revive_Fail=1,
	eShopItemErr_Revive_NotDead,
	eShopItemErr_Revive_NotUse,
	eShopItemErr_Revive_Refuse,
	eShopItemErr_Revive_TooFar,
	eShopItemErr_Revive_TimeOver,	
	eShopItemErr_Revive_NotReady,
};


//������������������������������������������������������������������
//��Don't Duplication shopItem									  ��
//������������������������������������������������������������������
enum
{
	eShopItemDupType_None=0,
	eShopItemDupType_Charm,
	eShopItemDupType_Herb,
	eShopItemDupType_Incantation,
	eShopItemDupType_Sundries,
	eShopItemDupType_PetEquip,
};

enum DONTDUP_INCANTATION
{
	eDontDupUse_MemoryMove		= 2,
	eDontDupUse_ProtectAll		= 4,
	eDontDupUse_LevelCancel50	= 8,
	eDontDupUse_LevelCancel70	= 16,
	eDontDupUse_LevelCancel90	= 32,
	eDontDupUse_ShowPyoguk		= 64,
	eDontDupUse_Chase			= 128,
	eDontDupUse_TownMove		= 256,

	/*
	eDontDupUse_MemoryMove		= 0x00000001,		// 1
	eDontDupUse_ProtectAll		= 0x00000010,		// 16
	eDontDupUse_LevelCancel50	= 0x00000100,		// 256
	eDontDupUse_LevelCancel70	= 0x00001000,		// 4096
	eDontDupUse_LevelCancel90	= 0x00010000,		// 65536
	eDontDupUse_ShowPyoguk		= 0x00100000,		// 262144
	*/
};

enum DONTDUP_CHARM
{
	eDontDupUse_NaegongDamage	= 2,
	eDontDupUse_WoigongDamage	= 4,
	eDontDupUse_Exppoint		= 8,
	eDontDupUse_Reinforce		= 16,
	eDontDupUse_Kyunggong		= 32,
	eDontDupUse_Ghost			= 64,	//2007. 10. 9. CBH - �䱫�ֹ���
	eDontDupUse_Woigong			= 128,	//2007. 10. 9. CBH - �ܰ��ֹ���
	eDontDupUse_Naegong			= 256,	//2007. 10. 9. CBH - �����ֹ���
	eDontDupUse_Hunter			= 16384,	// ��ɲ� ���� by Stiner(2008/06/25)
	eDontDupUse_ExpDay			= 32768,	// ����ġ ���� by Stiner(2008/06/25)
};

enum DONTDUP_HERB
{
	eDontDupUse_Life			= 2,
	eDontDupUse_Shield			= 4,
	eDontDupUse_Naeruyk			= 8,
	eDontDupUse_GreateLife		= 16,
	eDontDupUse_GreateShield	= 32,
	eDontDupUse_GreateNaeruyk	= 64,
	eDontDupUse_EventSatang		= 128,
	eDontDupUse_Doll			= 256,
};

enum DONTDUP_SUNDRIES
{
	eDontDupUse_StreeStall = 2,
};


enum DONTDUP_PETEQUIP
{
	eDupPetEquip_PomanRing = 2,
};


#define SLOT_SHOPITEM_IMSI		90
#define REVIVE_OTHER_DIST		1000



enum
{
	eMode_NormalNote = 0,
	eMode_PresentNote,
};



#define MAX_SHOUT_LENGTH	60




//��������������������������������������������������������
//��Guild Tournament									��
//��������������������������������������������������������
#define LIMITMEMBER_GUILDTOURNAMENT		5
#define LIMITLEVEL_GUILDTOURNAMENT		2
#define MAXGUILD_INTOURNAMENT			32
#define MAX_EVENTGT_COUNT				10

enum
{
	eGTError_NOERROR=0,					// No Error
	eGTError_NOTREGISTDAY,				// Not a Regist-day
	eGTError_NOTREGISTCANCELDAY,		// Not a Registcancel-day
	eGTError_DONTFINDGUILDINFO,			// Don't find a GuildInfo
	eGTError_MAXGUILDOVER,				// Over MAXGUILD_INTOURNAMENT
	eGTError_ALREADYREGISTE,			// Already Registed GuildTournament
	eGTError_ALREADYCANCELREGISTE,		// Already Cancel Registed GuildTournament
	eGTError_UNDERLIMITEMEMBER,			// Under LIMITMEMBER_GUILDTOURNAMENT
	eGTError_UNDERLEVEL,				// Under LIMITLEVEL_GUILDTOURNAMENT
	eGTError_DELETEAPPLIED,				// CancelError Regited GuildTournament
	eGTError_NOGUILDMASTER,				// Not a GuildMaster
	eGTError_DONTFINDPOSITION,			// GuildTournament-Position Find Error
	eGTError_DONTFINDBATTLE,			// Don't find a Battle	
	eGTError_BATTLEJOINFAIL,			// Failed a join in Battle
	eGTError_NOTENTERTIME,				// Not a time of enter the battle;
	eGTError_STUDENTCANTENTER,			// a guild student can't enter the battle
	eGTError_ERROR,
};

enum
{
	eGTState_BeforeRegist=0,
	eGTState_Regist,
	eGTState_BeforeEntrance,
	eGTState_Entrance,
	eGTState_Process,
	eGTState_Leave,
	eGTState_None,		//SW061220 ���±��NPC���� �߰�. //��� ǥ�� ���ϱ� ���� ����.
};

enum
{
	eGTStanding_1ST = 1,
	eGTStanding_2ND = 2,
	eGTStanding_3RD = 3,
	eGTStanding_4TH = 4,
	eGTStanding_8TH = 8,
	eGTStanding_16TH = 16,
	eGTStanding_32TH = 32,
};

enum
{
	eGTFight_32		= 0,
	eGTFight_16		= 1,
	eGTFight_8		= 2,
	eGTFight_4		= 3,
	eGTFight_3_4	= 4,
	eGTFight_1_2	= 5,

	eGTT_MAX,

	eGTFight_End,
	eGTFignt_Event,
};

enum
{
	eGTDay_SUNDAY=0,
	eGTDay_MONDAY,
	eGTDay_TUESDAY,
	eGTDay_WEDNESDAY,
	eGTDay_THURSDAY,
	eGTDay_FRIDAY,
	eGTDay_SATURDAY,
};

enum
{
	eGTWin_None = 0,
	eGTWin_Unearned,		// ������
	eGTWin_RemainPlayer,	// ���������
	eGTWin_LowLevel,		// �������� ����
	eGTWin_LowExp,			// ����ġ�� ����.
};

enum
{
	eGTEvent_None = 0,
	eGTEvent_Start,
	eGTEvent_NoGuild,
	eGTEvent_CreateBattleFailed,
	eGTEvent_MaxEventOver,
	eGTEvent_AlreadyJoinedGuild,
};

// 06.09.25 RaMa - �߼��̺�Ʈ
enum FULLMOONEVENTRATE
{
	eFULLMOONEVENT_NONE = 0,
	eFULLMOONEVENT_BUN,
	eFULLMOONEVENT_CHINESECAKE,

	eFULLMOONEVENT_MAX
};


//������������������������������������������������������������������
//��SiegeWar													  ��
//������������������������������������������������������������������
#define MAX_CASTLEGATE_NUM		20


//SW061205 ��� NPC
enum eFlagNpc
{
	eGTFlg,
	eSGFlg,
	eFlgKind,
};

//��������������������������������������������������������
//��Npcscript Param										��
//��������������������������������������������������������
enum
{
	eNpcParam_ShowpPyoguk=1,
};




//��������������������������������������������������������
//��Item Parameter										��
//��������������������������������������������������������
#define ITEM_PARAM_SEAL		0x00000001
#define ITEM_PARAM_CHEAT	0x00000010
#define ITEM_PARAM_PRESENT	0x00000100
#define ITEM_PARAM_TITAN_REGIST	0x00001000	// magi82 - Titan(070423) ��ϵ� Ÿ��ź ����






//-----------------------------------------------------------------------------------
// Move Dialog
//-----------------------------------------------------------------------------------
#define MAX_MOVEDATA_PERPAGE		10
#define MAX_MOVEPOINT_PAGE			2
#define MAX_SAVEDMOVE_NAME			21





//-----------------------------------------------------------------------------------
// Item Parameter
//-----------------------------------------------------------------------------------
#define ITEMPARAM_ISLOCK(Param)					( (DWORD)Param>>31	)		// 2147483648
#define ITEMPARAM_SETLOCK(Param, Option)		( Param = ((Param<<1)|(Option<<31)))


enum eCharacterStage
{
	eStage_Normal	= 0,	// �Ϲ�
	eStage_Hwa		= 64,	// ����	  		
	eStage_Hyun		= 65,	// ����	  		
	eStage_Geuk		= 128,	// �ظ�	  		
	eStage_Tal		= 129,	// Ż��	  		
};

enum eItemStage
{
	eItemStage_Normal		= 0,
	eItemStage_ChangeStage	= 32,
	///////////////////////////////
	// 06. 06. 2�� ���� - �̿���
	eItemStage_ChangeStage2	= 33,
	///////////////////////////////
	eItemStage_OverHwa		= 64,
	eItemStage_Hwa			= 65,
	eItemStage_Hyun			= 66,
	eItemStage_OverGeuk		= 128,
	eItemStage_Geuk			= 129,
	eItemStage_Tal			= 130,
};

// guildunion
enum eGuildUnion_Err
{
	eGU_Not_ValidName,
	eGU_Not_InGuild,
	eGU_Not_GuildMaster,
	eGU_Not_Level,
	eGU_Not_GuildUnionMaster,
	eGU_Not_InGuildUnion,
	eGU_Aleady_InGuildUnion,
	eGU_Time,
	eGU_Other_Not_InGuild,
	eGU_Other_Not_InGuildUnion,
	eGU_Other_Not_GuildMaster,
	eGU_Other_Aleady_InGuildUnion,
	eGU_Other_Time,
	eGU_Is_Full,	
	eGU_Is_GuildFieldWar,
	eGU_Not_ValidMap,
};

enum eSWProfit_Err
{
	eSW_Not_Profit,
	eSW_Not_ProfitGuild,
	eSW_Not_ProfitGuildMaster,	
	eSW_No_TaxRate,
	eSW_No_Time,
	eSW_No_Money,
};


//��������������������������������������������������������
//��For Japan_Local										��
//��������������������������������������������������������
enum eCharacter_Attr	// ȭ->��->��->��->��
{
	eCA_None	= 0,
	eCA_Fire	= 1,		
	eCA_Earth	= 2,
	eCA_Iron	= 3,
	eCA_Water	= 4,
	eCA_Tree	= 5,	
};

enum eItem_Attr
{
	eIA_All			= 0,
	eIA_Fire		= 1,
	eIA_Earth		= 2,
	eIA_Iron		= 3,	
	eIA_Water		= 4,	
	eIA_Tree		= 5,	
	eIA_Fire_Only	= 11,
	eIA_Earth_Only	= 12,
	eIA_Iron_Only	= 13,
	eIA_Water_Only	= 14,
	eIA_Tree_Only	= 15,	
};

enum eCharacter_Attr_Relation
{
	eCAR_None				= -1,
	eCAR_Same				= 0,
	eCAR_Give_Help			= 1,	// �����ִ� ���
	eCAR_Strong_Opposite	= 2,	// ���� ���� ���
	eCAR_Weak_Opposite		= 3,	// ���� ������ ���
	eCAR_Need_Help			= 4,	// ����޴� ���	
};

#ifdef _JAPAN_LOCAL_
#define GIVEN_CHARACTER_SLOT		3
#define GIVEN_INVENTORY_SLOT		2
#define GIVEN_PYOGUK_SLOT			3
#define GIVEN_MUGONG_SLOT			10
#define MUGONG_SLOT_ADDCOUNT		5

#elif _HK_LOCAL_
#define GIVEN_CHARACTER_SLOT		2
#define GIVEN_INVENTORY_SLOT		2
#define GIVEN_PYOGUK_SLOT			2
#define GIVEN_MUGONG_SLOT			10
#define MUGONG_SLOT_ADDCOUNT		5

#elif _TL_LOCAL_
#define GIVEN_CHARACTER_SLOT		2
#define GIVEN_INVENTORY_SLOT		2
#define GIVEN_PYOGUK_SLOT			2
#define GIVEN_MUGONG_SLOT			10
#define MUGONG_SLOT_ADDCOUNT		5

#else
#define GIVEN_CHARACTER_SLOT		2
#define GIVEN_INVENTORY_SLOT		2
#define GIVEN_PYOGUK_SLOT			2
#define GIVEN_MUGONG_SLOT			10
#define MUGONG_SLOT_ADDCOUNT		5
#endif

#define EVENT_MONSTER_SANTA1			145			// ���� 99 ��Ÿ�ǿ���
#define EVENT_MONSTER_SANTA2			160			// ���� 15 ��Ÿ�ǿ���
#define EVENT_DAMAGE					2007		// ����� ���ݽ� // ��Ÿ�ǿ����� ������ ���ݽ� �ߴ� ������
#define EVENT_MONSTER_RAT1				296			// �̺�Ʈ��1 (�����̺�Ʈ)
#define EVENT_MONSTER_RAT2				297			// �̺�Ʈ��2 (�����̺�Ʈ)
#define EVENT_MONSTER_RAT3				298			// �̺�Ʈ��3 (�����̺�Ʈ)
#define EVENT_DAMAGE_NEWYEAR			2008		// �㵹�� ���ݽ� ���� ������
#define EVENT_MONSTER_RABBIT1			187			// �̺�Ʈ�䳢1
#define EVENT_MONSTER_RABBIT2			188			// �̺�Ʈ�䳢2
#define EVENT_MONSTER_RABBIT3			189			// �̺�Ʈ�䳢3
#define EVENT_DAMAGE_HAMMER				50			// �̺�Ʈ�и�ġ�� ������
#define EVENT_DAMAGE_RABBIT				100			// �̺�Ʈ�䳢���� ���� ������
#define EVENT_MONSTER_PUMPHEAD1			146
#define EVENT_MONSTER_PUMPHEAD2			190
#define EVENT_MONSTER_PUMPHEAD3			191
#define EVENT_MONSTER_PUMPHEAD4			192
#define EVENT_DAMAGE_PUMPHEAD			200
#define EVENT_FIELDBOSS_SNOWMAN_SM		233			// �̺�Ʈ�ʵ庸�� 15LV
#define EVENT_FIELDSUB_SNOWMAN_SM		234			// ����
#define EVENT_FIELDBOSS_SNOWMAN_MD		235			// �̺�Ʈ�ʵ庸�� 30LV
#define EVENT_FIELDSUB_SNOWMAN_MD		236
#define EVENT_FIELDBOSS_SNOWMAN_LG		237			// �̺�Ʈ�ʵ庸�� 99LV
#define EVENT_FIELDSUB_SNOWMAN_LG		238
#define EVENT_SNOWMAN_SM				193			// �����
#define EVENT_SNOWMAN_MD				194
#define EVENT_SNOWMAN_LG				195
#define EVENT_MONSTER_HONEYPIG1			161
#define EVENT_MONSTER_HONEYPIG2			199
#define EVENT_MONSTER_HONEYPIG3			200
#define EVENT_MONSTER_CARNATION1		301
#define EVENT_MONSTER_CARNATION2		302
#define EVENT_MONSTER_CARNATION3		303

#define EVENT_ITEM_GLOVE			50509			// ����尩	
#define EVENT_ITEM_SNOW				50508			// ������
#define EVENT_ITEM_HARDSNOW			50513			// ��������
#define EVENT_ITEM_FIRECRACKER		53111			// ����

#define EVENT_EFFECT_FIRECRACKER	1036			// ���� ����Ʈ

#define EVENT_SHOPITEM_SNOWMAN_HAT			55572	// �ƹ�Ÿ ������ �����(����)
#define EVENT_SHOPITEM_SNOWMAN_DRESS		55573	// �ƹ�Ÿ ������ �����(��)
#define EVENT_SHOPITEM_RUDOLP_HAT			55574	// �ƹ�Ÿ ������ �絹��(����)
#define EVENT_SHOPITEM_RUDOLP_DRESS			55575	// �ƹ�Ÿ ������ �絹��(��)

#define EVENT_SHOPITEM_SNOWMAN_HAT2			55580	// �ƹ�Ÿ ������ �����(����) �ð���
#define EVENT_SHOPITEM_SNOWMAN_DRESS2		55581	// �ƹ�Ÿ ������ �����(��) �ð���
#define EVENT_SHOPITEM_RUDOLP_HAT2			55582	// �ƹ�Ÿ ������ �絹��(����) �ð���
#define EVENT_SHOPITEM_RUDOLP_DRESS2		55583	// �ƹ�Ÿ ������ �絹��(��) �ð���

#define EVENT_SHOPITEM_SNOWMAN1_HK			57760
#define EVENT_SHOPITEM_SNOWMAN2_HK			57762
#define EVENT_SHOPITEM_SNOWMAN3_HK			57764

#define EVENT_SHOPITEM_RUDOLP1_HK			57761
#define EVENT_SHOPITEM_RUDOLP2_HK			57763
#define EVENT_SHOPITEM_RUDOLP3_HK			57765


// ������
enum eWeatherState
{
	eWS_None,	// 0
	eWS_Snow,	// 1

	eWS_Max,	
};

////////////////////////////////////////////////////////
//06. 06 2�� ���� - �̿���
//����Ʈ ����(����)
enum eSkipEffect
{
	eSkipEffect_None,
	eSkipEffect_Start,

	eSkipEffect_Max,
};
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// 06. 06. 2�� ���� - �̿���
// ����/����
enum eSingleSpecialState
{
	eSingleSpecialState_None,
	
	eSingleSpecialState_Hide,			// ����
	eSingleSpecialState_Detect,		// ����
	eSingleSpecialState_DetectItem,	// ������ ����

	eSingleSpecialState_Max,

	// ��� ���� �ڵ�
	eSingleSpecialState_Error,			// ����
	eSingleSpecialState_Error_NaeRyuk,	// ���� ����
	eSingleSpecialState_Error_Delay,	// ������

};


//////////////////////////////////////////////////////////////////////////
// 06. 06. 2�� ���� - �̿���
// ���� ��ȯ �߰�
#define MAX_SKILLOPTION_COUNT	8	// �ִ� �ɼ� ��

enum eSkillOption_Kind{
	
	eSkillOption_None,				// �ɼ� ����

	eSkillOption_Range,				// 1. ��Ÿ�
	eSkillOption_ReduceNaeRyuk,		// 2. ���¼Ҹ�
	eSkillOption_PhyAtk,			// 3. ��ų��������
	eSkillOption_BaseAtk,			// 4. �Ϲݹ�������
	eSkillOption_AttAtk,			// 5. �Ӽ�����

	eSkillOption_Life,				// 6. �ִ������
	eSkillOption_NaeRyuk,			// 7. �ִ볻��
	eSkillOption_Shield,			// 8. �ִ�ȣ�Ű���
	eSkillOption_PhyDef,			// 9. �������
	eSkillOption_AttDef,			// 10. �Ӽ����

	eSkillOption_Duration,			// 11. ���ӽð�

	eSkillOption_Max,
};
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// 06. 06. 2�� ���� - �̿���
// ���� ��ȯ
// ��ȯ ���� - 0: ��ȯ�Ұ�, 1: ������, 2:������, 3:��ΰ���
enum eSkillChangeKind{
	
	eSkillChange_None,		// ��ȯ�Ұ�
	eSkillChange_Offense,	// ������
	eSkillChange_Defense,	// ������
	eSkillChange_All,		// ��� ����
	eSkillChange_Job,		// ������ų

	eSkillChange_Max,
};
////////////////////////////////////////////////////////

enum eGuildPlusTimeKindList
{
	eSuRyun_OneHalf,
	eMuGong_OneHalf,
	eExp_OneHalf,
	eDamageUp_20Pct,
	eGuildPlusTime_Max,
};

enum eGuildPlustTimeKind
{
	//eGPT_None,
	eGPT_SuRyun,
	eGPT_MuGong,
	eGPT_Exp,
	eGPT_DamageUp,
	eGPT_Kind_Max,
};

struct GUILDUSING_PLUSTIME_INFO
{
	int PlusTimeIdx;
	DWORD PlusTimeEndTime;
};

struct GUILDPOINT_INFO
{
	GUILDPOINT_INFO() {memset(this,0,sizeof(GUILDPOINT_INFO));}

	//��������Ʈ
	int GuildPoint;
	//��ɸ�������
	int GuildHuntedMonsterCount;
	//��ɸ���������
	int GuildHuntedMonsterTotalCount;
	//DB������Ʈ �ð�
	DWORD DBProcessTime;
	//��빮���÷���Ÿ��
	DWORD GuildPlusTimeflg;
	//�����÷���Ÿ�� ����
	GUILDUSING_PLUSTIME_INFO GuildUsingPlusTimeInfo[eGPT_Kind_Max];
	//��빫������(����)
	//DWORD Guild
};

enum {eServerOnly, eServerNClient};

enum eItemOptions	//MSG_LINKITEMOPTIONS
{
	eOPTS_ItemOption = 4,
	eOPTS_ItemRareOption = 8,
	eOPTS_PetTotalInfo = 16,
	eOPTS_TitanTotalInfo = 32,
	eOPTS_TitanEquipEndurance = 64,
};

// magi82 - Titan(070420)
enum eTitanNpcMenu
{
	eTITAN_NPC_BuyItem,
	eTITAN_NPC_TitanMix,
	eTITAN_NPC_TitanUpgrade,
	eTITAN_NPC_TitanPartsMake,
	eTITAN_NPC_TitanPartsChange,
	eTITAN_NPC_TitanBreak,
	eTITAN_NPC_TitanRegister,
	eTITAN_NPC_TitanDissolution,
	// eTITAN_NPC_TitanMugongMix,	// magi82 - Titan(070611) Ÿ��ź ������ȯ �ּ�ó��
};

//////////// 2007. 6. 8. CBH - ��Ʈ������ ���� �߰� /////////
enum eSET_ITEM_KIND
{
	eSETITEM_GMGANG = 0,	//�ݰ���Ʈ
	eSETITEM_CHUNRYUNG,		//õ�ɼ�Ʈ
	eSETITEM_YASUWANG,		//�߼��ռ�Ʈ
	eSETITEM_UMYANGJINGI,	//�������⼼Ʈ

	MAX_SETITEM_KIND_NUM,	//��Ʈ �� ����
};
/////////////////////////////////////////////////////////////


#define TITANMAP_START_INDEX 100	//2007. 6. 28. CBH - Ÿ��ź �� ���� ��ȣ

#define MAX_JOBLEVEL_NUM 13			// 2007. 6. 28. CBH - ������� ���� ����
#define MAX_JOBMOB_NUM 13			// 2007. 6. 29. CBH - ������� �� ���� ����
#define JOBSKILL_DEMAGE 300			// 2007. 10. 30. CBH - ������� Ÿ�ݽ� ������ (�������� Ƚ�� ����)

//////////// 2007. 6. 28. CBH - ������� ������ ����Ʈ ����ü �߰� /////////
struct JOB_SKILL_PROBABILITY_INFO
{
	WORD wSkillLevel;
	WORD ProbabilityArray[MAX_JOBMOB_NUM];
};
////////////////////////////////////////////////////////////////////////////

#define UNIQUE_ITEM_MIX_RESULT_INDEX 5	// magi82 - UniqueItem(070709)

//2007. 10. 25. CBH - Object.h���� �ű�
//������ Ŭ���̾�Ʈ���� ���ε��� ���ǵ��־ �������� �ű��
//������ KIND���� �����ϸ鼭 ������ Ŭ���̾�Ʈ�� ���� Ʋ����
//�������� �ʵ��� ���� �������� �������� �ű�.
enum EObjectKind
{
	eObjectKind_Player = 1,
	eObjectKind_Npc		=	2,
	eObjectKind_Item	=	4,
	eObjectKind_Tactic	=	8,
	eObjectKind_SkillObject=16,
	eObjectKind_Monster	=	32,
	eObjectKind_BossMonster=33,
	eObjectKind_SpecialMonster=34,

	// �ʵ庸�� - 05.12 �̿���
	eObjectKind_FieldBossMonster=35,
	eObjectKind_FieldSubMonster=36,
	eObjectKind_ToghterPlayMonster=37,
	//2007. 7. 2. CBH - ������� ������Ʈ ���� �߰�
	eObjectKind_Mining	= 38,	
	eObjectKind_Collection = 39,	
	eObjectKind_Hunt	= 40,	
	//2007. 10. 30. CBH - Ÿ��ź ����
	eObjectKind_TitanMonster	= 41,	

	eObjectKind_MapObject	= 64,
	eObjectKind_CastleGate	= 65,
	eObjectKind_Pet			= 128,
	// ���� 255 �Ѿ�� �ȵ�... ��.�� => kind�� �޴� ������ BYTE���̶�
	eObjectKind_Titan		= 255,	
};

//2007. 10. 25. CBH - ������Ʈ �׷� ���� �߰�
//�����θ� ���� enum���� ��һ�
enum eObjectKindGroup
{
	eOBJECTKINDGROUP_NONE,	//�ش����� �ʴ´�
	eOBJECTKINDGROUP_JOB,	//������� ���� ������Ʈ �׷�
};

#define SKINITEM_LIST_MAX 3	//2008. 1. 15. CBH - ��Ų ����Ʈ �ε����� �ִ� ������ ����

//2007. 12. 4. CBH - ��Ų ���� �߰�
enum eSKINITEM_EQUIP_KIND
{
	eSkinItem_Hat = 0,			//����
	eSkinItem_Mask,				//��
	eSkinItem_Dress,			//��
	eSkinItem_Shoulder,			//���
	eSkinItem_Shoes,			//�Ź�

	eSkinItem_Max,	
};

//2007. 12. 10. CBH - ��Ų ���� ó�� ��� ��ȯ enum ��
enum SKIN_RESULT
{
	eSkinResult_Success = 0,	//����
	eSkinResult_Fail,			//ó�� ����
	eSkinResult_DelayFail,		//������ ����
	eSkinResult_LevelFail,		//�������� ����
};

// magi82(35) MapChange State
// ���̵� �Ҷ� � ������ ���̵����� üũ �ϱ� ���� ��
enum MapChange_State
{
    eMapChange_General = 0,
	eMapChange_Item,
	eMapChange_SOS,
};

// magi82(37) �� �Ӽ� ������
enum eMapKindState
{
	eMapView			= 64,	// �ش� �ʺ���(�������̽���) - �̴ϸ�, ���

	eEventMap			= 128,	// �̺�Ʈ��
	eRunningMap,

	eSiegeWarMap		= 256,	// ������

	eBossMap			= 512,	// ������

	eTitanMap			= 1024,	// ��Ÿ����

	eQuestRoom			= 2048,	// �⿬�ǹ�

	eTournament			= 4096,	// ��ʸ�Ʈ

	eSurvivalMap		= 8192, // �����̹�
};

enum eAutoNoteError
{
	eAutoNoteError_None = 0,
	eAutoNoteError_CantUse,			// �����ü�� ����� �� ����.
	eAutoNoteError_CantUseMap,
	eAutoNoteError_AlreadyAsking,
	eAutoNoteError_CantFind,
	eAutoNoteError_AlreadyAsked,
	eAutoNoteError_NotProperState,
};

// �����
// ���������� �����ε����� �� ��
// ���� �� ���� + ���� �ʹ�ȣ (EX - ������� �������̸� ���� 2100000019)
#define SEIGEWARWAREIDX		2100000000
#define	FORTWARWAREIDX_1	2100000001
#define FORTWARWAREIDX_2	2100000002
#define FORTWARWAREIDX_3	2100000003

enum eFortWarEngraveError
{
	eFortWarEngraveError_None = 0,
	eFortWarEngraveError_NotMap,			// ����� ���� �ƴϴ�
	eFortWarEngraveError_NotIng,			// ����� �������� �ƴϴ�
	eFortWarEngraveError_AlreadyPlayer,		// �ٸ� ĳ���Ͱ� �������̴�
	eFortWarEngraveError_NotGuildMaster,
};

enum eFortWarWareError
{
	eFortWarWareError_None = 0,
	eFortWarWareError_NotMap,				// â���� �̿��� �� �ִ� ���� �ƴϴ�.
	eFortWarWareError_NotTime,				// â���� �̿��� �� �ִ� �ð��� �ƴϴ�.
	eFortWarWareError_NotRight,				// â�� �̿������ ����.
	eFortWarWareError_NotUse,				// �̿��� �� ����.

};


#endif //__COMMONGAMEDEFINE_H__

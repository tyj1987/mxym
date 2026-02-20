// ErrorMsg.h - Server version
// Minimal implementation for server builds
// This file provides compatibility with client-side CommonDefine.h

#ifndef __ERRORMSG_SERVER_H__
#define __ERRORMSG_SERVER_H__

// WriteAssertMsg is implemented in CommonGameFunc.cpp
// Forward declaration matching the implementation
#ifdef __cplusplus
extern "C" {
#endif

void WriteAssertMsg(char* pStrFileName, int Line, char* pMsg);

#ifdef __cplusplus
}
#endif

// Minimal ERRORCODE enum for server compatibility
#ifndef _MHCLIENT_
enum ERRORCODE
{
	EC_GENERAL_ERROR,
	EC_CONNECT_DISTRIBUTESERVER_FAILED,
	EC_CONNECT_AGENTSERVER_FAILED,
	EC_IMAGELOAD_FAILED,
	EC_MSGSEND_FAILED,
	EC_IDPWAUTH_FAILED,
	EC_MAPSERVER_CLOSED,
	EC_CHARACTERLIST_FAILED,
	EC_CHARACTERSELECT_FAILED,
	EC_CHARNAME_EXIST,
	EC_CHARNAME_SUCCESS,
	EC_DONOTNAMEDUPLCHECK,
	EC_CHARACTERMAKE_FAILED,
	EC_MUGONG_ADDNITEMDELETE_FAILED,
	EC_VERSION_CHECK_FAILED,
	EC_IPADDRESS_INVALID_FAILED,
	EC_UNKNOWN_PROTOCOL,
};
#endif

#endif // __ERRORMSG_SERVER_H__

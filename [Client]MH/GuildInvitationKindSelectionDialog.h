#pragma once
#include "./interface/cDialog.h"

class CGuildInvitationKindSelectionDialog :	public cDialog
{
public:
	CGuildInvitationKindSelectionDialog(void);
	virtual ~CGuildInvitationKindSelectionDialog(void);

	void Linking();
	void OnActionEvent(LONG lId, void* p, DWORD we);

};

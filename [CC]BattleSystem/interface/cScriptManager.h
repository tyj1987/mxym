#ifndef _CSCRIPTMANAGER_STUB_H
#define _CSCRIPTMANAGER_STUB_H

class cScriptManager
{
public:
	static cScriptManager* GetInstance() { static cScriptManager mgr; return &mgr; }
};

#endif

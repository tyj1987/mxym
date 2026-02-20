#ifndef _CWINDOWMANAGER_STUB_H
#define _CWINDOWMANAGER_STUB_H

// CC模块客户端编译时的cWindowManager存根

class cWindowManager
{
public:
	static cWindowManager* GetInstance() { static cWindowManager mgr; return &mgr; }
};

#endif

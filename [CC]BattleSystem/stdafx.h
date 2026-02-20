// stdafx.h - BattleSystem module precompiled header
// This file provides common includes for BattleSystem module compilation

#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Include common game headers
#include "..\[CC]Header\CommonIncludes.h"

// Include BattleSystem specific headers
#include "BattleFactory.h"
#include "..\4DyuchiGXGFunc/global.h"

// Client-specific includes
#ifdef _MHCLIENT_LIBRARY_
#include "ClientDefs.h"
#endif

// Forward declarations
class CBattle;
class CBattleSystem;
class CBattleFactory;
class CObject;
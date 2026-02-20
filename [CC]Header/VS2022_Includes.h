#pragma once

// VS2022编译专用头文件
// 解决VS2008到VS2022迁移的头文件路径问题

// YHLibrary头文件（使用绝对路径）
#include "D:\mxym\[Lib]YHLibrary\YHLibrary.h"
#include "D:\mxym\[Lib]YHLibrary\Array.h"
#include "D:\mxym\[Lib]YHLibrary\cLinkedList.h"
#include "D:\mxym\[Lib]YHLibrary\cLooseLinkedList.h"
#include "D:\mxym\[Lib]YHLibrary\cConstLinkedList.h"
#include "D:\mxym\[Lib]YHLibrary\PtrList.h"
#include "D:\mxym\[Lib]YHLibrary\Strclass.h"
#include "D:\mxym\[Lib]YHLibrary\StrTokenizer.h"
#include "D:\mxym\[Lib]YHLibrary\Fileio.h"
#include "D:\mxym\[Lib]YHLibrary\MemoryPool.h"
#include "D:\mxym\[Lib]YHLibrary\MemoryPoolTempl.h"
#include "D:\mxym\[Lib]YHLibrary\hashtable.h"
#include "D:\mxym\[Lib]YHLibrary\connection.h"
#include "D:\mxym\[Lib]YHLibrary\Encryptor.h"
#include "D:\mxym\[Lib]YHLibrary\StaticString.h"
#include "D:\mxym\[Lib]YHLibrary\DEFINE.H"
#include "D:\mxym\[Lib]YHLibrary\BaseNetworkInterface.h"
#include "D:\mxym\[Lib]YHLibrary\DESC_BASENETWORK.h"

// HSEL头文件
#include "D:\mxym\[Lib]HSEL\HSEL.h"
#include "D:\mxym\[Lib]HSEL\HSEL_STREAM.h"

// ZipArchive头文件
#include "D:\mxym\[Lib]ZipArchive\ZipArchive.h"
#include "D:\mxym\[Lib]ZipArchive\ZipString.h"
#include "D:\mxym\[Lib]ZipArchive\ZipException.h"

// [CC]Header头文件
#include "..\..\[CC]Header\vector.h"
#include "..\..\[CC]Header\protocol.h"
#include "..\..\[CC]Header\CommonDefine.h"
#include "..\..\[CC]Header\CommonGameDefine.h"
#include "..\..\[CC]Header\ServerGameDefine.h"
#include "..\..\[CC]Header\CommonGameStruct.h"
#include "..\..\[CC]Header\CommonStruct.h"
#include "..\..\[CC]Header\ServerGameStruct.h"
#include "..\..\[CC]Header\CommonGameFunc.h"

// Windows和标准库
#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <ole2.h>
#include <initguid.h>
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>
#include <time.h>

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

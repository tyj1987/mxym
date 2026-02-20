#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ole2.h>
#include <initguid.h>
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>
#include <mswsock.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

// 常用定义
#define GUID_SIZE 128
#define MAX_STRING_LENGTH 256
typedef void** PPVOID;

// TCP Keepalive相关结构体和常量
#ifndef SIO_KEEPALIVE_VALS
#define SIO_KEEPALIVE_VALS  _WSAIOW(IOC_VENDOR,4)

struct tcp_keepalive
{
    ULONG onoff;
    ULONG keepalivetime;
    ULONG keepaliveinterval;
};
#endif

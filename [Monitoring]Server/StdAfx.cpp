// stdafx.cpp : source file that includes just the standard includes
//	MonitoringServer.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "stdio.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

// WriteAssertMsg 和 LOG 现在由 CommonGameFunc.cpp 和 MonitoringServer.cpp 提供

void LOG( char* msg, ... )
{
	char buff[256];
	
	va_list vl;
	va_start( vl, msg );
	vsprintf( buff, msg, vl );
	va_end( vl );

	FILE* file = fopen( "MS_Debug.txt", "a+" );
	if( file )
	{
		fprintf( file, "%s\n", buff );
		fclose( file );
	}
}
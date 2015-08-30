// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <stdio.h>

extern char curpath[MAX_PATH];
extern char adbpath[MAX_PATH];

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		GetCurrentDirectory(MAX_PATH, curpath);
		sprintf_s(adbpath, MAX_PATH, "%s\\adb.exe", curpath);

		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


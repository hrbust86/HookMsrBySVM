#pragma once
#include <ntdef.h>

typedef struct _HOOK_PARAM
{
	LIST_ENTRY ListEntry;
	unsigned long long SysNum;
	unsigned long long ParamNum;
	unsigned long long * pFun;
}HOOK_PARAM, *PHOOK_PARAM;

typedef
NTSTATUS
(NTAPI *ADDHOOK)(PHOOK_PARAM pList);

typedef
VOID
(NTAPI *RMHOOK)(PHOOK_PARAM pList);

typedef struct _HOOK_EXTENSION
{
	ADDHOOK AddHook;
	RMHOOK RmHook;
}HOOK_EXTENSION, *PHOOK_EXTENSION;
#pragma once

#include "../SvmUtil.h"

//#include "interface.h"

#ifdef  _WIN64
typedef UINT64   uint;
#else
typedef UINT32   uint;
#endif

extern "C" extern ULONG64 NtSyscallHandler64;
extern "C" extern ULONG64 SysCallNum;
extern "C" VOID __stdcall HookPort64(uint pstack, uint param2, uint param3, uint param4);

#define MSR_LSTAR 0xc0000082          /* long mode SYSCALL target */

typedef uint(*_CallStub0)();
typedef uint(*_CallStub1)(uint);
typedef uint(*_CallStub2)(uint, uint);
typedef uint(*_CallStub3)(uint, uint, uint);
typedef uint(*_CallStub4)(uint, uint, uint, uint);
typedef uint(*_CallStub5)(uint, uint, uint, uint, uint);
typedef uint(*_CallStub6)(uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub7)(uint, uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub8)(uint, uint, uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub9)(uint, uint, uint, uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub10)(uint, uint, uint, uint, uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub11)(uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub12)(uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub13)(uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub14)(uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint);
typedef uint(*_CallStub15)(uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint);

#define CallStubX(a)  (_CallStub##a)

typedef enum _LIST_STATUS
{
	Free = 0,
	Running,
	Fixing
}LIST_STATUS;

NTSTATUS SyscallHookEnable();

NTSTATUS SyscallHookDisable();

//NTSTATUS AddHook(PHOOK_PARAM pList);

//NTSTATUS RmHook(PHOOK_PARAM pList);
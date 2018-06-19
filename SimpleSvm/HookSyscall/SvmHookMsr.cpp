#include "SvmHookMsr.h"

#pragma warning( push )
#pragma warning(disable: 4127)

extern "C" VOID MyKiSystemCall64();
extern "C" uint GetRax();
extern "C" uint GetR10();

ULONG64 NtSyscallHandler64 = 0;
ULONG64 SysCallNum = 0;
KSPIN_LOCK g_InterfaceSplock;
LIST_ENTRY g_HookList = {0};
uint g_HookCnt = 0;
long g_ListStatus = LIST_STATUS::Free;
long g_RunningCnt = 0;

VOID __stdcall HookPort64(uint pstack, uint param2, uint param3, uint param4)
{
	uint SysNum = GetRax();
	uint param1 = GetR10();
	
	if (KeGetCurrentIrql() > PASSIVE_LEVEL)
	{
		return;
	}

	if (LIST_STATUS::Fixing == g_ListStatus) // 丢掉一些事件，但是假如死等的话会有死锁的可能，不知道为啥
	{
		return;
	}

	InterlockedIncrement(&g_RunningCnt);

	InterlockedDecrement(&g_RunningCnt);
}

// Enables syscall hook for all processors
NTSTATUS SyscallHookEnable() 
{
	PAGED_CODE();

	if (NtSyscallHandler64 != NULL)
	{
		DbgPrint("[HookMsr]hook already start\n");
		return STATUS_UNSUCCESSFUL;
	}

	NtSyscallHandler64 = (ULONG64)UtilReadMsr64(Msr::kIa32Lstar);

	g_HookCnt = 0;
	g_InterfaceSplock = 0;
	g_ListStatus = LIST_STATUS::Free;
	g_RunningCnt = 0;
	InitializeListHead(&g_HookList);

	return UtilForEachProcessor(
		[](void* context) {
		//UNREFERENCED_PARAMETER(context);
		context = (void *)MyKiSystemCall64;
		return UtilVmCall(HypercallNumber::kHookSyscall, context);
	},
		nullptr);

}

// Disables syscall hook for all processors
NTSTATUS SyscallHookDisable()
{
	PAGED_CODE();
	NTSTATUS status = UtilForEachProcessor(
		[](void* context) {
		UNREFERENCED_PARAMETER(context);
		return UtilVmCall(HypercallNumber::kUnhookSyscall, nullptr);
	},
		nullptr);

	NtSyscallHandler64 = NULL;
	g_HookCnt = 0;
	g_ListStatus = LIST_STATUS::Free;
	g_RunningCnt = 0;

	return status;
}

#pragma warning( pop )
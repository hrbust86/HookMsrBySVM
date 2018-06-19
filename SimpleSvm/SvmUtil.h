#pragma once
#include "SvmHead.h"
#include "SvmStruct.h"
#include "HookSyscall/SvmHookMsr.h"

/// Available command numbers for VMCALL
enum class HypercallNumber : unsigned __int32 {
	kTerminateVmm,            //!< Terminates VMM
	kPingVmm,                 //!< Sends ping to the VMM
	kShEnablePageShadowing,   //!< Calls ShEnablePageShadowing()
	kShDisablePageShadowing,  //!< Calls ShVmCallDisablePageShadowing()
	kHookSyscall,
	kUnhookSyscall,
};

_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
VOID
SvDebugPrint(
	_In_z_ _Printf_format_string_ PCSTR Format,
	...
	);

extern "C" VOID NTAPI AsmSvmCall(_In_ ULONG_PTR hypercall_number,
	_In_opt_ void *context);

NTSTATUS UtilVmCall(HypercallNumber hypercall_number,
	void *context);

_IRQL_requires_same_
VOID
SvInjectGeneralProtectionException(
	_Inout_ PVIRTUAL_PROCESSOR_DATA VpData
	);

void UtilWriteMsr64(Msr msr, ULONG64 value);

ULONG64 UtilReadMsr64(Msr msr);

NTSTATUS UtilForEachProcessor(NTSTATUS(*callback_routine)(void *), void *context);

_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_min_(PASSIVE_LEVEL)
_IRQL_requires_same_
_Check_return_
NTSTATUS
SvVirtualizeAllProcessors(
	VOID
	);

_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_min_(PASSIVE_LEVEL)
_IRQL_requires_same_
VOID
SvDevirtualizeAllProcessors(
	VOID
	);

BOOL StartAmdSvmAndHookMsr();

VOID StopAmdSvm();
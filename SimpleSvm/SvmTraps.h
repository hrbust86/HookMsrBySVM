#pragma once
#include "SvmHead.h"
#include "SvmStruct.h"
#include "SvmUtil.h"

VOID SvHandleVmmcall(
	_Inout_ PVIRTUAL_PROCESSOR_DATA VpData,
	_Inout_ PGUEST_CONTEXT GuestContext);

void VmmpHandleVmCallHookSyscall(
	PVIRTUAL_PROCESSOR_DATA VpData, void * NewSysCallEntry);

VOID SvHandleEFERWrite(
	_Inout_ PVIRTUAL_PROCESSOR_DATA VpData,
	_Inout_ PGUEST_CONTEXT GuestContext);

VOID SvHandleLstrRead(
	_Inout_ PVIRTUAL_PROCESSOR_DATA VpData,
	_Inout_ PGUEST_CONTEXT GuestContext);

void VmmpHandleVmCallUnHookSyscall(PVIRTUAL_PROCESSOR_DATA VpData);
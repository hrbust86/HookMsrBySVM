#include "SvmUtil.h"

/*!
@brief      Sends a message to the kernel debugger.

@param[in]  Format - The format string to print.
@param[in]  arguments - Arguments for the format string, as in printf.
*/
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
VOID
SvDebugPrint(
	_In_z_ _Printf_format_string_ PCSTR Format,
	...
	)
{
	va_list argList;

	va_start(argList, Format);
	vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, Format, argList);
	va_end(argList);
}

NTSTATUS UtilVmCall(HypercallNumber hypercall_number,
	void *context) {
	__try {
		AsmSvmCall(static_cast<ULONG>(hypercall_number), context);
			return STATUS_SUCCESS;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		const auto status = GetExceptionCode();
		HYPERPLATFORM_COMMON_DBG_BREAK();
		HYPERPLATFORM_LOG_WARN_SAFE("Exception thrown (code %08x)", status);
		return status;
	}
}

/*!
@brief          Injects #GP with 0 of error code.

@param[inout]   VpData - Per processor data.
*/
_IRQL_requires_same_
VOID
SvInjectGeneralProtectionException(
	_Inout_ PVIRTUAL_PROCESSOR_DATA VpData
	)
{
	EVENTINJ event;

	//
	// Inject #GP(vector = 13, type = 3 = exception) with a valid error code.
	// An error code are always zero. See "#GP—General-Protection Exception
	// (Vector 13)" for details about the error code.
	//
	event.AsUInt64 = 0;
	event.Fields.Vector = 13;
	event.Fields.Type = 3;
	event.Fields.ErrorCodeValid = 1;
	event.Fields.Valid = 1;
	VpData->GuestVmcb.ControlArea.EventInj = event.AsUInt64;
}

void UtilWriteMsr64(Msr msr, ULONG64 value) {
	__writemsr(static_cast<unsigned long>(msr), value);
}

ULONG64 UtilReadMsr64(Msr msr) {
	return __readmsr(static_cast<unsigned long>(msr));
}

NTSTATUS UtilForEachProcessor(NTSTATUS(*callback_routine)(void *), void *context) {
	PAGED_CODE();

	const auto number_of_processors =
		KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	for (ULONG processor_index = 0; processor_index < number_of_processors;
	processor_index++) {
		PROCESSOR_NUMBER processor_number = {};
		auto status =
			KeGetProcessorNumberFromIndex(processor_index, &processor_number);
		if (!NT_SUCCESS(status)) {
			return status;
		}

		// Switch the current processor
		GROUP_AFFINITY affinity = {};
		affinity.Group = processor_number.Group;
		affinity.Mask = 1ull << processor_number.Number;
		GROUP_AFFINITY previous_affinity = {};
		KeSetSystemGroupAffinityThread(&affinity, &previous_affinity);

		// Execute callback
		status = callback_routine(context);

		KeRevertToUserGroupAffinityThread(&previous_affinity);
		if (!NT_SUCCESS(status)) {
			return status;
		}
	}
	return STATUS_SUCCESS;
}

BOOL StartAmdSvmAndHookMsr()
{
	BOOL ret = FALSE;
	NTSTATUS status;

	SV_DEBUG_BREAK();
	ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

	status = SvVirtualizeAllProcessors();
	if (!NT_SUCCESS(status))
	{
		goto EXIT;
	}

	status = SyscallHookEnable();
	if (!NT_SUCCESS(status))
	{
		SvDevirtualizeAllProcessors();
		goto EXIT;
	}

	ret = TRUE;

EXIT:

	return ret;
}

VOID StopAmdSvm()
{
	SyscallHookDisable();
	SvDevirtualizeAllProcessors();
}
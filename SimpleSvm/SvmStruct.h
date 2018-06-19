#pragma once

#include "SimpleSvm.hpp"

#include <intrin.h>
#include <ntifs.h>
#include <stdarg.h>
#include "SvmHead.h"

#define POOL_NX_OPTIN   1

EXTERN_C
VOID
_sgdt(
	_Out_ PVOID Descriptor
	);

_IRQL_requires_(DISPATCH_LEVEL)
_IRQL_requires_same_
DECLSPEC_NORETURN
EXTERN_C
VOID
NTAPI
SvLaunchVm(
	_In_ PVOID HostRsp
	);

//
// x86-64 defined structures.
//

//
// See "2-Mbyte PML4E有ong Mode" and "2-Mbyte PDPE有ong Mode".
//
typedef struct _PML4_ENTRY_2MB
{
	union
	{
		UINT64 AsUInt64;
		struct
		{
			UINT64 Valid : 1;               // [0]
			UINT64 Write : 1;               // [1]
			UINT64 User : 1;                // [2]
			UINT64 WriteThrough : 1;        // [3]
			UINT64 CacheDisable : 1;        // [4]
			UINT64 Accessed : 1;            // [5]
			UINT64 Reserved1 : 3;           // [6:8]
			UINT64 Avl : 3;                 // [9:11]
			UINT64 PageFrameNumber : 40;    // [12:51]
			UINT64 Reserved2 : 11;          // [52:62]
			UINT64 NoExecute : 1;           // [63]
		} Fields;
	};
} PML4_ENTRY_2MB, *PPML4_ENTRY_2MB,
PDP_ENTRY_2MB, *PPDP_ENTRY_2MB;
static_assert(sizeof(PML4_ENTRY_2MB) == 8,
	"PML4_ENTRY_1GB Size Mismatch");

//
// See "2-Mbyte PDE有ong Mode".
//
typedef struct _PD_ENTRY_2MB
{
	union
	{
		UINT64 AsUInt64;
		struct
		{
			UINT64 Valid : 1;               // [0]
			UINT64 Write : 1;               // [1]
			UINT64 User : 1;                // [2]
			UINT64 WriteThrough : 1;        // [3]
			UINT64 CacheDisable : 1;        // [4]
			UINT64 Accessed : 1;            // [5]
			UINT64 Dirty : 1;               // [6]
			UINT64 LargePage : 1;           // [7]
			UINT64 Global : 1;              // [8]
			UINT64 Avl : 3;                 // [9:11]
			UINT64 Pat : 1;                 // [12]
			UINT64 Reserved1 : 8;           // [13:20]
			UINT64 PageFrameNumber : 31;    // [21:51]
			UINT64 Reserved2 : 11;          // [52:62]
			UINT64 NoExecute : 1;           // [63]
		} Fields;
	};
} PD_ENTRY_2MB, *PPD_ENTRY_2MB;
static_assert(sizeof(PD_ENTRY_2MB) == 8,
	"PDE_ENTRY_2MB Size Mismatch");

//
// See "GDTR and IDTR Format有ong Mode"
//
#include <pshpack1.h>
typedef struct _DESCRIPTOR_TABLE_REGISTER
{
	UINT16 Limit;
	ULONG_PTR Base;
} DESCRIPTOR_TABLE_REGISTER, *PDESCRIPTOR_TABLE_REGISTER;
static_assert(sizeof(DESCRIPTOR_TABLE_REGISTER) == 10,
	"DESCRIPTOR_TABLE_REGISTER Size Mismatch");
#include <poppack.h>

//
// See "Long-Mode Segment Descriptors" and some of definitions
// (eg, "Code-Segment Descriptor有ong Mode")
//
typedef struct _SEGMENT_DESCRIPTOR
{
	union
	{
		UINT64 AsUInt64;
		struct
		{
			UINT16 LimitLow;        // [0:15]
			UINT16 BaseLow;         // [16:31]
			UINT32 BaseMiddle : 8;  // [32:39]
			UINT32 Type : 4;        // [40:43]
			UINT32 System : 1;      // [44]
			UINT32 Dpl : 2;         // [45:46]
			UINT32 Present : 1;     // [47]
			UINT32 LimitHigh : 4;   // [48:51]
			UINT32 Avl : 1;         // [52]
			UINT32 LongMode : 1;    // [53]
			UINT32 DefaultBit : 1;  // [54]
			UINT32 Granularity : 1; // [55]
			UINT32 BaseHigh : 8;    // [56:63]
		} Fields;
	};
} SEGMENT_DESCRIPTOR, *PSEGMENT_DESCRIPTOR;
static_assert(sizeof(SEGMENT_DESCRIPTOR) == 8,
	"SEGMENT_DESCRIPTOR Size Mismatch");

typedef struct _SEGMENT_ATTRIBUTE
{
	union
	{
		UINT16 AsUInt16;
		struct
		{
			UINT16 Type : 4;        // [0:3]
			UINT16 System : 1;      // [4]
			UINT16 Dpl : 2;         // [5:6]
			UINT16 Present : 1;     // [7]
			UINT16 Avl : 1;         // [8]
			UINT16 LongMode : 1;    // [9]
			UINT16 DefaultBit : 1;  // [10]
			UINT16 Granularity : 1; // [11]
			UINT16 Reserved1 : 4;   // [12:15]
		} Fields;
	};
} SEGMENT_ATTRIBUTE, *PSEGMENT_ATTRIBUTE;
static_assert(sizeof(SEGMENT_ATTRIBUTE) == 2,
	"SEGMENT_ATTRIBUTE Size Mismatch");

//
// SimpleSVM specific structures.
//

typedef struct _SHARED_VIRTUAL_PROCESSOR_DATA
{
	PVOID MsrPermissionsMap;
	DECLSPEC_ALIGN(PAGE_SIZE) PML4_ENTRY_2MB Pml4Entries[1];    // Just for 512 GB
	DECLSPEC_ALIGN(PAGE_SIZE) PDP_ENTRY_2MB PdpEntries[512];
	DECLSPEC_ALIGN(PAGE_SIZE) PD_ENTRY_2MB PdeEntries[512][512];
} SHARED_VIRTUAL_PROCESSOR_DATA, *PSHARED_VIRTUAL_PROCESSOR_DATA;

typedef struct _VIRTUAL_PROCESSOR_DATA
{
	union
	{
		//
		//  Low     HostStackLimit[0]                        StackLimit
		//  ^       ...
		//  ^       HostStackLimit[KERNEL_STACK_SIZE - 2]    StackBase
		//  High    HostStackLimit[KERNEL_STACK_SIZE - 1]    StackBase
		//
		DECLSPEC_ALIGN(PAGE_SIZE) UINT8 HostStackLimit[KERNEL_STACK_SIZE];
		struct
		{
			UINT8 StackContents[KERNEL_STACK_SIZE - sizeof(PVOID) * 6];
			UINT64 GuestVmcbPa;     // HostRsp
			UINT64 HostVmcbPa;
			struct _VIRTUAL_PROCESSOR_DATA* Self;
			PSHARED_VIRTUAL_PROCESSOR_DATA SharedVpData;
			//UINT64 Padding1;        // To keep HostRsp 16 bytes aligned
			UINT64 OriginalMsrLstar;
			UINT64 Reserved1;
		} HostStackLayout;
	};

	DECLSPEC_ALIGN(PAGE_SIZE) VMCB GuestVmcb;
	DECLSPEC_ALIGN(PAGE_SIZE) VMCB HostVmcb;
	DECLSPEC_ALIGN(PAGE_SIZE) UINT8 HostStateArea[PAGE_SIZE];
} VIRTUAL_PROCESSOR_DATA, *PVIRTUAL_PROCESSOR_DATA;
static_assert(sizeof(VIRTUAL_PROCESSOR_DATA) == KERNEL_STACK_SIZE + PAGE_SIZE * 3,
	"VIRTUAL_PROCESSOR_DATA Size Mismatch");

typedef struct _GUEST_REGISTERS
{
	UINT64 R15;
	UINT64 R14;
	UINT64 R13;
	UINT64 R12;
	UINT64 R11;
	UINT64 R10;
	UINT64 R9;
	UINT64 R8;
	UINT64 Rdi;
	UINT64 Rsi;
	UINT64 Rbp;
	UINT64 Rsp;
	UINT64 Rbx;
	UINT64 Rdx;
	UINT64 Rcx;
	UINT64 Rax;
} GUEST_REGISTERS, *PGUEST_REGISTERS;

typedef struct _GUEST_CONTEXT
{
	PGUEST_REGISTERS VpRegs;
	BOOLEAN ExitVm;
} GUEST_CONTEXT, *PGUEST_CONTEXT;


//
// x86-64 defined constants.
//
#define IA32_MSR_PAT    0x00000277
#define IA32_MSR_EFER   0xc0000080
#define IA32_MSR_LSTR   0xC0000082

#define EFER_SVME       (1UL << 12)

#define RPL_MASK        3
#define DPL_SYSTEM      0

#define CPUID_FN8000_0001_ECX_SVM                   (1UL << 2)
#define CPUID_FN0000_0001_ECX_HYPERVISOR_PRESENT    (1UL << 31)
#define CPUID_FN8000_000A_EDX_NP                    (1UL << 0)

#define CPUID_MAX_STANDARD_FN_NUMBER_AND_VENDOR_STRING          0x00000000
#define CPUID_PROCESSOR_AND_PROCESSOR_FEATURE_IDENTIFIERS       0x00000001
#define CPUID_PROCESSOR_AND_PROCESSOR_FEATURE_IDENTIFIERS_EX    0x80000001
#define CPUID_SVM_FEATURES                                      0x8000000a
//
// The Microsoft Hypervisor interface defined constants.
//
#define CPUID_HV_VENDOR_AND_MAX_FUNCTIONS   0x40000000
#define CPUID_HV_INTERFACE                  0x40000001

//
// SimpleSVM specific constants.
//
#define CPUID_UNLOAD_SIMPLE_SVM     0x41414141
#define CPUID_HV_MAX                CPUID_HV_INTERFACE

/*!
@brief      Breaks into a kernel debugger when it is present.

@details    This macro is emits software breakpoint that only hits when a
kernel debugger is present. This macro is useful because it does
not change the current frame unlike the DbgBreakPoint function,
and breakpoint by this macro can be overwritten with NOP without
impacting other breakpoints.
*/
#define SV_DEBUG_BREAK() \
    if (KD_DEBUGGER_NOT_PRESENT) \
    { \
        NOTHING; \
    } \
    else \
    { \
        __debugbreak(); \
    } \
    reinterpret_cast<void*>(0)

/// See: MODEL-SPECIFIC REGISTERS (MSRS)
enum class Msr : unsigned int {
	kIa32ApicBase = 0x01B,

	kIa32FeatureControl = 0x03A,

	kIa32SysenterCs = 0x174,
	kIa32SysenterEsp = 0x175,
	kIa32SysenterEip = 0x176,

	kIa32Debugctl = 0x1D9,

	kIa32VmxBasic = 0x480,
	kIa32VmxPinbasedCtls = 0x481,
	kIa32VmxProcBasedCtls = 0x482,
	kIa32VmxExitCtls = 0x483,
	kIa32VmxEntryCtls = 0x484,
	kIa32VmxMisc = 0x485,
	kIa32VmxCr0Fixed0 = 0x486,
	kIa32VmxCr0Fixed1 = 0x487,
	kIa32VmxCr4Fixed0 = 0x488,
	kIa32VmxCr4Fixed1 = 0x489,
	kIa32VmxVmcsEnum = 0x48A,
	kIa32VmxProcBasedCtls2 = 0x48B,
	kIa32VmxEptVpidCap = 0x48C,
	kIa32VmxTruePinbasedCtls = 0x48D,
	kIa32VmxTrueProcBasedCtls = 0x48E,
	kIa32VmxTrueExitCtls = 0x48F,
	kIa32VmxTrueEntryCtls = 0x490,
	kIa32VmxVmfunc = 0x491,

	kIa32Xss = 0xda0,

	kIa32Efer = 0xC0000080,
	kIa32Star = 0xC0000081,
	kIa32Lstar = 0xC0000082,

	kIa32Fmask = 0xC0000084,

	kIa32FsBase = 0xC0000100,
	kIa32GsBase = 0xC0000101,
	kIa32KernelGsBase = 0xC0000102,
	kIa32TscAux = 0xC0000103,
};

EXTERN NtSyscallHandler64:DQ
EXTERN SysCallNum:DQ
EXTERN HookPort64 : PROC

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; macros
;

; Saves all general purpose registers to the stack
PUSHAQ MACRO
    push    rax
    push    rcx
    push    rdx
    push    rbx
    push    -1      ; dummy for rsp
    push    rbp
    push    rsi
    push    rdi
    push    r8
    push    r9
    push    r10
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15
ENDM

; Loads all general purpose registers from the stack
POPAQ MACRO
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdi
    pop     rsi
    pop     rbp
    add     rsp, 8    ; dummy for rsp
    pop     rbx
    pop     rdx
    pop     rcx
    pop     rax
ENDM

USERMD_STACK_GS = 10h
KERNEL_STACK_GS = 1A8h
SYSCALL_MAX_INDEX = 4096
NT_CREATE_FILE = 052h

.CODE

GetRax PROC
	ret
GetRax ENDP

GetR10 PROC
	mov rax, r10
	ret
GetR10 ENDP

; *********************************************************
;
; Return to the original NTOSKRNL syscall handler
; (Restore all old registers first)
;
; *********************************************************
MyKiSystemCall64 PROC
	swapgs                                  ; swap GS base to kernel PCR
    mov       gs:[USERMD_STACK_GS], rsp   ; save user stack pointer
	cmp       rax, SYSCALL_MAX_INDEX      ; Is the index larger than the array size?
	jge         EntryPoint  

	;cmp       eax, NT_CREATE_FILE
	;jne         EntryPoint
	;int         3
	mov       rsp,qword ptr gs:[KERNEL_STACK_GS] ; get stack pointer

	; hook_fun
	PUSHAQ                  ; -8 * 16
	mov rcx,   gs:[USERMD_STACK_GS] ; user stack to param
	;rdx
	;r8
	;r9
	; save volatile XMM registers
    sub rsp, 60h
    movaps xmmword ptr [rsp - 0], xmm0
    movaps xmmword ptr [rsp - 10h], xmm1
    movaps xmmword ptr [rsp - 20h], xmm2
    movaps xmmword ptr [rsp - 30h], xmm3
    movaps xmmword ptr [rsp - 40h], xmm4
    movaps xmmword ptr [rsp - 50h], xmm5

    sub rsp, 200h
	call HookPort64
	add rsp, 200h

    ; restore XMM registers
    movaps xmm0, xmmword ptr [rsp - 0]
    movaps xmm1, xmmword ptr [rsp - 10h]
    movaps xmm2, xmmword ptr [rsp - 20h]
    movaps xmm3, xmmword ptr [rsp - 30h]
    movaps xmm4, xmmword ptr [rsp - 40h]
    movaps xmm5, xmmword ptr [rsp - 50h]
    add rsp, 60h
	POPAQ

EntryPoint:
	mov         rsp, gs:[USERMD_STACK_GS]   ; Usermode RSP
	swapgs                                  ; Switch to usermode GS
	jmp         [NtSyscallHandler64]          ; Jump back to the old syscall handler

MyKiSystemCall64 ENDP

END
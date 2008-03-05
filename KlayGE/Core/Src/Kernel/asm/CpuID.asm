;-----------------------------------------------------------------------------
;   int __cdecl get_cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
;-----------------------------------------------------------------------------

%ifdef KLAYGE_CPU_X86

BITS 32

SECTION .text

global _get_cpuid
	align 16
	_get_cpuid

    push    ebp
    mov     ebp,    esp
    push    ebx
    push    esi
    push    edi
    
    mov     eax,    [ebp +  8]
    cpuid

    mov     esi,    [ebp + 12]
    mov     [esi],  eax

    mov     esi,    [ebp + 16]
    mov     [esi],  ebx

    mov     esi,    [ebp + 20]
    mov     [esi],  ecx

    mov     esi,    [ebp + 24]
    mov     [esi],  edx

    pop     edi
    pop     esi
    pop     ebx
    pop     ebp
    ret

%endif

%ifdef KLAYGE_CPU_X64

BITS 64

SECTION .text

global get_cpuid
%ifdef WIN64
	times 6 nop
	align 16
	get_cpuid
	.startfunc
	%assign unwindcount 0
	%assign framereg 0
%else
	align 16
	get_cpuid
%endif

	db 0x48
    push rbx
	%assign unwindcount unwindcount+1
	.unwind %+ unwindcount EQU $-.startfunc + 0 + (3 << 12)
    endprolog
    
    mov     r10,   r9
    mov     r11,   r8
    mov     r9,    rdx
%ifdef WIN64
    mov     r8,    [rsp+40+8]
%endif    
    
    mov     eax,   ecx
    cpuid

    mov     [r9],  eax
    mov     [r11], ebx
    mov     [r10], ecx
    mov     [r8],  edx

    pop     rbx
    ret
    endfunc

%endif

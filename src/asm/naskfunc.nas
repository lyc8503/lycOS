; 一些用汇编语言完成的函数

[FORMAT "WCOFF"]                ; 制作目标文件模式
[BITS 32]                       ; 制作 32 位机器语言
[INSTRSET "i486p"]              ; 指定 486 及以后的 CPU


; 制作目标文件信息

[FILE "naskfunc.nas"]           ; 源文件名信息
; 程序中包含的函数名
        GLOBAL      _io_hlt, _io_cli, _io_sti, _io_stihlt
        GLOBAL      _io_in8,  _io_in16,  _io_in32
        GLOBAL      _io_out8, _io_out16, _io_out32
        GLOBAL      _io_get_eflags, _io_set_eflags
        GLOBAL      _load_gdtr, _load_idtr
        GLOBAL      _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler20
        GLOBAL      _get_cr0, _set_cr0
        GLOBAL      _memtest_sub
        GLOBAL      _farjmp, _load_tr
        GLOBAL      __alloca
        EXTERN      _int_handler21, _int_handler27, _int_handler2c, _int_handler20  ; C 中的外部函数

; 以下是真正的函数

[SECTION .text]     ; 写了上面那些之后再开始写程序

_io_hlt:            ; 即 C 中的 void io_hlt(void);
        HLT
        RET         ; 返回, 即 return;

_io_cli:            ; void io_cli(void);
        CLI
        RET

_io_sti:            ; void io_sti(void);
        STI
        RET

_io_stihlt:         ; void io_stihlt(void);
        STI
        HLT
        RET

_io_in8:                                ; int io_in8(int port);
        MOV         EDX,[ESP+4]         ; port
        MOV         EAX,0
        IN          AL,DX
        RET

_io_in16:                               ; int io_in16(int port);
        MOV         EDX,[ESP+4]         ; port
        MOV         EAX,0
        IN          AX,DX
        RET

_io_in32:                               ; int io_in32(int port);
        MOV         EDX,[ESP+4]         ; port
        IN          EAX,DX
        RET

_io_out8:                               ; void io_out8(int port, int data);
        MOV         EDX,[ESP+4]         ; port
        MOV         AL,[ESP+8]          ; data
        OUT         DX,AL
        RET

_io_out16:                              ; void io_out16(int port, int data);
        MOV         EDX,[ESP+4]         ; port
        MOV         EAX,[ESP+8]         ; data
        OUT         DX,AX
        RET

_io_out32:                              ; void io_out32(int port, int data);
        MOV         EDX,[ESP+4]         ; port
        MOV         EAX,[ESP+8]         ; data
        OUT         DX,EAX
        RET

_io_get_eflags:                         ; int io_get_eflags(void);
        PUSHFD                          ; PUSH EFLAGS
        POP         EAX
        RET

_io_set_eflags:                         ; void io_set_eflags(int eflags);
        MOV         EAX,[ESP+4]
        PUSH        EAX
        POPFD                           ; POP EFLAGS
        RET

_load_gdtr:                             ; void load_gdtr(int limit, int addr);
        MOV         AX,[ESP+4]          ; limit
        MOV         [ESP+6],AX
        LGDT        [ESP+6]
        RET

_load_idtr:                             ; void load_idtr(int limit, int addr);
        MOV         AX,[ESP+4]          ; limit
        MOV         [ESP+6],AX
        LIDT        [ESP+6]
        RET

_get_cr0:                               ; int get_cr0(void);
        MOV         EAX,CR0
        RET

_set_cr0:                               ; void set_cr0(int cr0);
        MOV         EAX,[ESP+4]
        MOV         CR0,EAX
        RET

_asm_inthandler21:                      ; 处理中断
        PUSH        ES
        PUSH        DS
        PUSHAD
        MOV         EAX,ESP
        PUSH        EAX
        MOV         AX,SS
        MOV         DS,AX
        MOV         ES,AX
        CALL        _int_handler21
        POP         EAX
        POPAD
        POP         DS
        POP         ES
        IRETD

_asm_inthandler27:
        PUSH        ES
        PUSH        DS
        PUSHAD
        MOV         EAX,ESP
        PUSH        EAX
        MOV         AX,SS
        MOV         DS,AX
        MOV         ES,AX
        CALL        _int_handler27
        POP         EAX
        POPAD
        POP         DS
        POP         ES
        IRETD

_asm_inthandler2c:
        PUSH        ES
        PUSH        DS
        PUSHAD
        MOV         EAX,ESP
        PUSH        EAX
        MOV         AX,SS
        MOV         DS,AX
        MOV         ES,AX
        CALL        _int_handler2c
        POP         EAX
        POPAD
        POP         DS
        POP         ES
        IRETD

_asm_inthandler20:
        PUSH ES
        PUSH DS
        PUSHAD
        MOV EAX,ESP
        PUSH EAX
        MOV AX,SS
        MOV DS,AX
        MOV ES,AX
        CALL _int_handler20
        POP EAX
        POPAD
        POP DS
        POP ES
        IRETD

_memtest_sub:       ; unsigned int memtest_sub(unsigned int start, unsigned int end);
        PUSH        EDI                         ; 暂存 EBX, ESI, EDI
        PUSH        ESI
        PUSH        EBX
        MOV         ESI,0xaa55aa55              ; pat0 = 0xaa55aa55;
        MOV         EDI,0x55aa55aa              ; pat1 = 0x55aa55aa;
        MOV         EAX,[ESP+12+4]              ; i = start;
mts_loop:
        MOV         EBX,EAX
        ADD         EBX,0xffc                   ; p = i + 0xffc;
        MOV         EDX,[EBX]                   ; old = *p;
        MOV         [EBX],ESI                   ; *p = pat0;
        XOR         DWORD [EBX],0xffffffff      ; *p ^= 0xffffffff;
        CMP         EDI,[EBX]                   ; if (*p != pat1) goto fin;
        JNE         mts_fin
        XOR         DWORD [EBX],0xffffffff      ; *p ^= 0xffffffff;
        CMP         ESI,[EBX]                   ; if (*p != pat0) goto fin;
        JNE         mts_fin
        MOV         [EBX],EDX                   ; *p = old;
        ADD         EAX,0x1000                  ; i += 0x1000;
        CMP         EAX,[ESP+12+8]              ; if (i <= end) goto mts_loop;
        JBE         mts_loop
        POP         EBX
        POP         ESI
        POP         EDI
        RET
mts_fin:
        MOV         [EBX],EDX                   ; *p = old;
        POP         EBX
        POP         ESI
        POP         EDI
        RET

_load_tr:                                       ; void load_tr(int tr);
        LTR         [ESP+4]
        RET

_farjmp:                                        ; void farjmp(int eip, int cs);
        JMP     FAR [ESP+4]                     ; eip, cs
        RET

__alloca:
        ADD EAX,-4
        SUB ESP,EAX
        JMP DWORD [ESP+EAX] ; 代替RET

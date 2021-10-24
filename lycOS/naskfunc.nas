; 一些用汇编语言完成的函数

[FORMAT "WCOFF"]                ; 制作目标文件模式
[BITS 32]                       ; 制作 32 位机器语言
[INSTRSET "i486p"]              ; 指定 486 及以后的 CPU


; 制作目标文件信息

[FILE "naskfunc.nas"]           ; 源文件名信息
; 程序中包含的函数名
        GLOBAL  _io_hlt, _io_cli, _io_sti, _io_stihlt
        GLOBAL  _io_in8,  _io_in16,  _io_in32
        GLOBAL  _io_out8, _io_out16, _io_out32
        GLOBAL  _io_get_eflags, _io_set_eflags
        GLOBAL	_load_gdtr, _load_idtr


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

_io_get_eflags:                         ; int io_load_eflags(void);
        PUSHFD                          ; PUSH EFLAGS
        POP         EAX
        RET

_io_set_eflags:                         ; void io_store_eflags(int eflags);
        MOV         EAX,[ESP+4]
        PUSH        EAX
        POPFD                           ; POP EFLAGS
        RET

_load_gdtr:		                        ; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		        ; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		                        ; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		        ; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

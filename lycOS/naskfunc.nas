; 一些用汇编语言完成的函数

[FORMAT "WCOFF"]                ; 制作目标文件模式
[BITS 32]                       ; 制作 32 位机器语言
[INSTRSET "i486p"]              ; 指定 486 及以后的 CPU


; 制作目标文件信息

[FILE "naskfunc.nas"]           ; 源文件名信息

        GLOBAL	_io_hlt         ; 程序中包含的函数名


; 以下是真正的函数

[SECTION .text]		; 写了上面那些之后再开始写程序

_io_hlt:            ; 即 C 中的 void io_hlt(void);
        HLT
        RET         ; 返回, 即 return;

; 一些用???言完成的函数

[FORMAT "WCOFF"]                ; 制作目?文件模式
[BITS 32]                       ; 制作 32 位机器?言


; 制作目?文件信息

[FILE "naskfunc.nas"]           ; 源文件名信息

        GLOBAL	_io_hlt         ; 程序中包含的函数名


; 以下是??的函数

[SECTION .text]		; 写了上面?些之后再?始写程序

_io_hlt:            ; 即 C 中的 void io_hlt(void);
        HLT
        RET         ; 返回, 即 return;

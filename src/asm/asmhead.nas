BOTPAK      EQU         0x00280000        ; bootpack 加载位置
DSKCAC      EQU         0x00100000        ; 磁盘缓存位置
DSKCAC0     EQU         0x00008000        ; 磁盘缓存位置(实模式)

; BOOT_INFO相关
CYLS        EQU         0x0ff0            ; 设定启动区
LEDS        EQU         0x0ff1
VMODE       EQU         0x0ff2            ; 颜色位数
SCRNX       EQU         0x0ff4            ; 分辨率 X
SCRNY       EQU         0x0ff6            ; 分辨率 Y
VRAM        EQU         0x0ff8            ; 图像缓冲区开始的内存地址
VBEMODE     EQU         0x103             ; VESA 图形模式

        ORG             0xc200            ; 程序载入的内存起点

[INSTRSET "i486p"]                  ; 使用 486 指令

; 画面设置

; 先尝试使用高分辨率模式, 如果失败 fallback 到 320x200 模式

; 检查 VBE 是否存在
        MOV         AX,0x9000              ; 写入的目标内存 ES:DI
        MOV         ES,AX
        MOV         AX,0x4f00
        INT         0x10
        CMP         AX,0x004f
        JNE         scrn320fallback

; 检查 VBE 版本
        MOV         AX,[ES:DI+4]
        CMP         AX,0x0200
        JB          scrn320fallback         ; AX < 0x0200 时, 使用 320x200 模式

; 获取画面模式信息 (依然写入 ES:DI, 覆盖 VBE 版本信息)
        MOV         CX,VBEMODE
        MOV         AX,0x4f01
        INT         0x10
        CMP         AX,0x004f               ; 0x4f 代表指定的模式可以使用
        JNE         scrn320fallback

; 检查画面模式信息
        CMP         BYTE [ES:DI+0x19],8     ; 8bit 色彩
        JNE         scrn320fallback
        CMP         BYTE [ES:DI+0x1b],4     ; 调色板模式
        JNE         scrn320fallback
        MOV         AX,[ES:DI+0x00]
        AND         AX,0x0080
        JZ          scrn320fallback         ; 属性模式第 7 位为 0, 失败

; 画面模式切换
        MOV         BX,VBEMODE+0x4000
        MOV         AX,0x4f02
        INT         0x10
        MOV         BYTE [VMODE],8          ; 画面信息储存
        MOV         AX,[ES:DI+0x12]
        MOV         [SCRNX],AX
        MOV         AX,[ES:DI+0x14]
        MOV         [SCRNY],AX
        MOV         EAX,[ES:DI+0x28]
        MOV         [VRAM],EAX
        JMP         keystatus

scrn320fallback:
        MOV        AL,0x13              ; VGA, 320x200x8bit 色彩
        MOV        AH,0x00
        INT        0x10
        MOV        BYTE [VMODE],8          ; 画面信息储存
        MOV        WORD [SCRNX],320
        MOV        WORD [SCRNY],200
        MOV        DWORD [VRAM],0x000a0000


; BIOS 获取键盘指示灯状态
keystatus:
        MOV        AH,0x02
        INT        0x16             ; keyboard BIOS
        MOV        [LEDS],AL


;   PIC关闭所有中断
;   根据 AT 兼容机规格, 如果需要初始化 PIC
;   必须要在 CLI 之前进行, 否则有时会挂起
;   随后进行 PIC 初始化

        MOV         AL,0xff
        OUT         0x21,AL
        NOP                         ; 有些机器可能无法连续执行 out 指令
        OUT         0xa1,AL

        CLI                         ; 禁止 CPU 中断

; 为了让 CPU 访问全部内存, 设定 A20GATE
; 发送的信号线是 0xdf 键盘控制电路, 这个端口控制的东西很多, 远远不止键盘

        CALL        waitkbdout
        MOV         AL,0xd1
        OUT         0x64,AL
        CALL        waitkbdout
        MOV         AL,0xdf         ; enable A20, 使 1MB 以上的内存可以访问
        OUT         0x60,AL
        CALL        waitkbdout      ; 等待指令执行完成

; 切换到保护模式

        LGDT        [GDTR0]         ; 临时设定 GDT
        MOV         EAX,CR0
        AND         EAX,0x7fffffff  ; bit31 = 0, 禁止分页
        OR          EAX,0x00000001  ; bit0 = 1, 保护模式
        MOV         CR0,EAX
        JMP         pipelineflush   ; 进入保护模式时下一条指令一定要是 JMP
pipelineflush:                      
        MOV         AX,1*8          ; 可读写的段为 32bit
        MOV         DS,AX           ; 进入 32 位模式后要重新设定一下段寄存器
        MOV         ES,AX
        MOV         FS,AX
        MOV         GS,AX
        MOV         SS,AX

; bootpack 的加载

        MOV         ESI,bootpack     ; 转送源
        MOV         EDI,BOTPAK       ; 转送目的地
        MOV         ECX,512*1024/4
        CALL        memcpy

; 磁盘数据会被加载到它本来对应的位置

; 先加载启动扇区

        MOV         ESI,0x7c00       ; 转送源
        MOV         EDI,DSKCAC       ; 转送目的地
        MOV         ECX,512/4
        CALL        memcpy

; 剩余所有内容

        MOV         ESI,DSKCAC0+512  ; 转送源
        MOV         EDI,DSKCAC+512   ; 转送目的地
        MOV         ECX,0
        MOV         CL,BYTE [CYLS]
        IMUL        ECX,512*18*2/4   ; 从柱面数变为字节数/4
        SUB         ECX,512/4        ; 减去 IPL 大小
        CALL        memcpy

; 以上内容翻译
; memcpy(bootpack, BOTPAK, 512*1024/4)
; memcpy(0x7c00, DSKCAC, 512/4)
; memcpy(DSKCAC0+512, DSKCAC+512, cyls*512*18*2/4-512/4)


; asmhead 工作完成
; 以后工作由 bootpack 完成

; 启动 bootpack

        MOV         EBX,BOTPAK
        MOV         ECX,[EBX+16]
        ADD         ECX,3           ; ECX += 3;
        SHR         ECX,2           ; ECX /= 4;
        JZ          skip            ; 没有需要转移的内容时
        MOV         ESI,[EBX+20]    ; 转送源
        ADD         ESI,EBX
        MOV         EDI,[EBX+12]    ; 转送目的地
        CALL        memcpy
skip:
        MOV         ESP,[EBX+12]    ; 栈初始值
        JMP         DWORD 2*8:0x0000001b

waitkbdout:
        IN          AL,0x64
        AND         AL,0x02
        JNZ         waitkbdout      ; AND 结果!=0, 跳转到 waitkbdout
        RET

memcpy:
        MOV         EAX,[ESI]
        ADD         ESI,4
        MOV         [EDI],EAX
        ADD         EDI,4
        SUB         ECX,1
        JNZ         memcpy          ; 计算结果!=0, 跳转到 memcpy
        RET
        ALIGNB      16              ; 添加 DBO 直至地址%16=0
GDT0:
        RESB        8               ; NULL Selector
        DW          0xffff,0x0000,0x9200,0x00cf     ; 可读写段 32bit
        DW          0xffff,0x0000,0x9a28,0x0047     ; 可执行段 32bit (bootpack用)

        DW          0
GDTR0:
        DW          8*3-1
        DD          GDT0

        ALIGNB      16
bootpack:

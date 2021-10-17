; lycOS 的汇编语言引导部分


CYLS    EQU     10      ; 申明常数 CYLS, 读取扇区数 = 10
        ORG     0x7c00  ; 程序加载位置  也就是 MBR 被 BIOS 读取到的位置, 内存分布图: https://blog.csdn.net/jadeshu/article/details/89038489
        
        JMP     entry


; FAT12 磁盘格式
; TODO: 改为 FAT32, 抛弃软盘格式, 改为 iso
; 这段暂时还是照着打了一遍, 之后可能要研究一下 FAT32 的格式

        DB      0x90
        DB      "lycOSIPL"      ; 启动区名称, 要求一定要正好是 8 个字节
        DW      512             ; 扇区大小 = 512 字节
        DB      1               ; 簇大小 = 1 扇区
        DW      1               ; FAT 起始位置 = 第一个扇区
        DB      2               ; FAT 个数 = 2
        DW      224             ; 根目录大小 = 224 项
        DW      2880            ; 磁盘大小 = 2880 扇区
        DB      0xf0            ; 磁盘种类 0xf0
        DW      9               ; FAT 长度 = 9扇区
        DW      18              ; 1 个磁道扇区数 = 18
        DW      2               ; 磁头数 = 2
        DD      0               ; 不使用分区
        DD      2880            ; 还是磁盘大小
        DB      0,0,0x29        ; 定值
        DD      0xffffffff      ; 卷标号码
        DB      "lycOS      "   ; 磁盘名称, 必须是 11 字节
        DB      "FAT12   "      ; 磁盘格式名称
        RESB    18              ; 空出 18 字节



; 以下是程序主体部分
entry:
        MOV     AX,0        ; 寄存器初始化
        MOV     SS,AX
        MOV     SP,0x7c00
        MOV     DS,AX
        
        ; 读取磁盘
        MOV     AX,0x0820
        MOV     ES,AX
        MOV     CH,0        ; 柱面 0
        MOV     DH,0        ; 磁头 0
        MOV     CL,2        ; 扇区 2

readloop:
        MOV     SI,0        ; 失败次数 0
retry:
        MOV     AH,0x02     ; AH=0x02 读取磁盘
        MOV     AL,1        ; 1 个扇区
        MOV     BX,0
        MOV     DL,0x00     ; A 驱动器
        INT     0x13        ; 调用磁盘 BIOS
        JNC     next        ; 没出错跳转到 next
        ADD     SI,1        ; 向 SI 加 1
        CMP     SI,5        ; SI >=5 时, 跳转到 error
        JAE     error
        MOV     AH,0x00
        MOV     DL,0x00
        INT     0x13        ; 重置驱动器
        JMP     retry
next:
        MOV     AX,ES       ; 内存地址后移 0x200(ES加上 0x20 相当于内存地址加上 0x200, 但 ES 不能直接加, 需要移动到 AX 再加)
        ADD     AX,0x0020
        MOV     ES,AX
        ADD     CL,1        ; CL + 1
        CMP     CL,18
        JBE     readloop    ; CL <= 18 时继续读取
        MOV     CL,1
        ADD     DH,1
        CMP     DH,2
        JB      readloop    ; DH < 2 跳转到 readloop 继续读取
        MOV     DH,0
        ADD     CH,1
        CMP     CH,CYLS
        JB      readloop    ; CH < CYLS 跳转到 readloop 继续读取

        MOV     [0x0ff0],CH ; 把读取到的长度保存到内存的 0x0ff0 位置
        JMP     0xc200      ; 真正程序开始的位置: 0x8000(内存位置) + 0x4200(磁盘位置) = 0xc200

error:
        MOV     SI,msg
putloop:
        MOV     AL,[SI]
        ADD     SI,1        ; SI + 1
        CMP     AL,0
        JE      fin
        MOV     AH,0x0e     ; 定值 0x0e
        MOV     BX,15       ; 颜色代码
        INT     0x10        ; 调用 BIOS 显示
        JMP     putloop
fin:
        HLT                 ; 停止工作
        JMP     fin
msg:
        DB      0x0a, 0x0a  ; 换行*2
        DB      "IPL: read disk error, max retry count exceeded."
        DB      0x0a        ; 换行
        DB      0

        RESB    0x7dfe-$    ; 填充至位置 0x7dfe

        DB      0x55, 0xaa  ; MBR 格式 结束标志

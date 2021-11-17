OBJS_BOOTPACK = bootpack.obj naskfunc.obj ascii_font.obj graphic.obj dsctbl.obj int.obj buffer.obj mouse.obj keyboard.obj memory.obj layer.obj

VPATH := ./gui:./memory:./device:./asm:./int

# 路径
TOOLPATH = ../tools/z_tools/
INCPATH  = ../tools/z_tools/haribote/
QEMU_EXE=../tools/qemu-w64-20210825/qemu-system-i386.exe

# 工具位置
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe

CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe


default:
	make img

# 读入十个扇区的 initial program loader
ipl10.bin: asm/ipl10.nas Makefile
	$(NASK) asm/ipl10.nas ipl10.bin ipl10.lst

# 用于加载下面的 C 语言的汇编代码
asmhead.bin : asm/asmhead.nas Makefile
	$(NASK) asm/asmhead.nas asmhead.bin asmhead.lst

# 将 C 语言代码编译成 gas 汇编源码
%.gas : %.c Makefile
	$(CC1) -o $*.gas $<

# 从 gas 汇编转换为 nask 文件
%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

# 编译生成目标文件(包括 C 和 nasfunc.nas)
naskfunc.obj : asm/naskfunc.nas
	$(NASK) asm/naskfunc.nas naskfunc.obj naskfunc.lst

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

# ascii 字体处理
ascii_font.bin : gui/ascii_font.txt Makefile
	$(MAKEFONT) gui/ascii_font.txt ascii_font.bin

ascii_font.obj : ascii_font.bin Makefile
	$(BIN2OBJ) ascii_font.bin ascii_font.obj _ascfont

# 链接目标文件 生成二进制镜像
bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

# 处理格式 TODO:暂不明确 hrb 文件格式
bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

# 拼接 asmhead 和 bootpack 生成 sys 文件
lycos.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb lycos.sys

# 最终的可引导软盘镜像
lycos.img : ipl10.bin lycos.sys Makefile
	$(EDIMG)   imgin:$(TOOLPATH)fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:lycos.sys to:@: \
		imgout:lycos.img



# 编译动作
img:
	make -r lycos.img

run:
	make img
	$(QEMU_EXE) -fda lycos.img

clean:
	-del *.bin
	-del *.lst
	-del *.gas
	-del *.obj
	-del bootpack.nas
	-del bootpack.map
	-del bootpack.bim
	-del bootpack.hrb
	-del lycos.sys

cleanall:
	make clean
	-del lycos.img	
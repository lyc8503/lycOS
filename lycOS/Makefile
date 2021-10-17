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


default:
	make img

# 读入十个扇区的 initial program loader
ipl10.bin: ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

# 用于加载下面的 C 语言的汇编代码
asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

# 将 C 语言代码编译成 gas 汇编源码
bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c

# 从 gas 汇编转换为 nask 文件
bootpack.nas : bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

# 编译生成目标文件
bootpack.obj : bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

# 编译汇编函数
naskfunc.obj : naskfunc.nas Makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

# 链接目标文件 生成二进制镜像
bootpack.bim : bootpack.obj naskfunc.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj
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
TOOLPATH = ../tools/z_tools/
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del


default:
	make img

ipl.bin: ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

helloos.img: ipl.bin Makefile
	$(EDIMG)  imgin:$(TOOLPATH)fdimg0at.tek \
	  wbinimg src:ipl.bin len:512 from:0 to:0  imgout:helloos.img

img:
	make -r helloos.img

asm:
	make -r ipl.bin

run:
	make img
	..\tools\qemu-w64-20210825\qemu-system-i386.exe -fda helloos.img

clean:
	del ipl.bin
	del ipl.lst

cleanall:
	del helloos.img
	make clean
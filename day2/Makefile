default:
	make img

ipl.bin: ipl.nas Makefile
	../tools/z_tools/nask.exe ipl.nas ipl.bin ipl.lst

helloos.img: ipl.bin Makefile
	../tools/z_tools/edimg.exe  imgin:../tools/z_tools/fdimg0at.tek \
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
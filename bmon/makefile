CC = c68
AS = a68
LD = l68
START = 10000
ROM = fe0000
# from router makefile (killed rt.obj) references
MATH=/usr/ee/lib/crt68.obj 
#MATH=rt.obj
CFLAGS = -DOMBYTE -DALTROM

OFILES=	main.obj boot.obj scan.obj ar.obj acia.obj ec.obj il.obj \
	strio.obj cksum.obj tftp.obj ip.obj  doprnt.obj \
	udp.obj net.obj charmap.obj halloc.obj autoconf.obj led.obj

LFILES=	main.obj, boot.obj, scan.obj, ar.obj, acia.obj, ec.obj, il.obj, \
	strio.obj, cksum.obj, tftp.obj, ip.obj, doprnt.obj, \
	udp.obj, net.obj, charmap.obj, halloc.obj, autoconf.obj, led.obj

all: bmon

bmon: vmch.obj $(OFILES) end.obj
	$(LD) "bmon-o0:x$(START)-m:x$(START)-e,bmon,bmon=vmch.obj, \
	       $(MATH), $(LFILES),end.obj"
	       /usr/router/ombyte/util/mkhdr $(START) > bmon
	       cat bmon.mem >> bmon
	       cp bmon /tmp/b; chmod a+r /tmp/b

bmon.run: mch.obj $(OFILES) end.obj
	$(LD) "bmon-o0:x$(ROM)-e,bmon,bmon=mch.obj, $(MATH), $(LFILES), end.obj"

bversa.run: vmch.obj $(OFILES) end.obj
	$(LD) "bversa-o0:xfe4000-e,bversa,bversa=vmch.obj,$(MATH), $(LFILES), end.obj"

fakerom.h: fakerom.rom
	mkrom -cq fakerom.h fakerom.rom

.SUFFIXES: .obj .c .asm

.c.obj:
	@echo $(CC) $(CFLAGS) -S $*.c
	@$(CC) $(CFLAGS) -S $*.c > /dev/null
	@$(AS) $*,AS/$*,AS/$*=$*.s > /dev/null 2>&1
	@rm $*.s

.asm.obj:
	@echo "$(AS) $*,$*,$*=$*"
	@$(AS) $*,$*,$*=NOLIST,$*

autoconf.obj: fakerom.h

clean:
	rm -f *.BAK *.CKP *.obj *.lst *.crf

clobber: clean
	rm -f bmon bmon.run bversa.run *.map

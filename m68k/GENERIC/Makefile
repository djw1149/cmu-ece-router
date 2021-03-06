######################################################################
#
#  Makefile for CMU IP Router System
#
######################################################################
#
# History:
#	Nov 13, 1984 | Gregg Lebovitz at cmu-ee-maxwell
#	1) made changes for the 68000. This makefile cannot be used
#	   with the pdp11 code. 
#	2) all the 68000 object files end in .obj
#	3) all the 68000 assembler files end in .asm
#	4) the makedepend script now takes the extension on the
#	   command line.
#
###################################################################
C68=/usr/ee/bin/c68
A68=/usr/ee/bin/a68
L68=/usr/ee/bin/l68
HOME=../..

START=1000
MACHINE=M68K
COPTS=-I. -I../h -I${STD}h -D${MACHINE}
MAKESYM=${HOME}/m68k/util/makesym
MAKERULES=${HOME}/m68k/util/makerules
MAKEDEPEND=${HOME}/m68k/util/makedepend
MATH=/usr/ee/lib/crt68.obj

STD=../../
EXP=../
EXPSTD=../../

H=h/
SYS=sys/
MCH=m68k/mch/
DEV=m68k/dev/
GENERIC=m68k/GENERIC/
UTIL=m68k/util/



#  The following symbol definitions are generated automatically by
#  "make makesym".  Do not adjust them manually or your changes will
#  be lost.

MAKESRC=\
    ../../m68k/util/Makefile
 
HFILES=\
    ../../m68k/dev/ab.h ../../m68k/dev/abreg.h ../../h/aconf.h ../../h/arp.h \
    ../../h/arpquery.h ../../m68k/mch/cable.h ../../h/chaos.h \
    ../../m68k/mch/device.h ../../h/devstats.h ../../m68k/mch/dma.h \
    ../../m68k/dev/ec.h ../../m68k/dev/ecreg.h ../../h/errorlog.h \
    ../../m68k/mch/fakerom.h ../../h/globalsw.h ../../h/icmp.h \
    ../../m68k/dev/il.h ../../m68k/dev/ilreg.h ../../m68k/mch/intr.h \
    ../../h/ip.h ../../h/logbad.h ../../h/malloc.h ../../m68k/mch/mch.h \
    ../../m68k/mch/memory.h ../../h/packet.h ../../m68k/mch/patch.h \
    ../../h/proc.h ../../m68k/mch/procu.h ../../h/profile.h ../../h/proto.h \
    ../../m68k/dev/ptm.h ../../m68k/dev/ptmreg.h ../../h/queue.h \
    ../../h/rcp.h ../../h/rtypes.h ../../h/time.h ../../m68k/mch/trap.h \
    ../../h/troll.h ../../m68k/dev/ttreg.h ../../h/udp.h ../../m68k/dev/vv.h \
    ../../m68k/dev/vvreg.h

CTFILES=\
    ../../sys/version.c

CSFILES=\
    ../../m68k/dev/ab.c ../../m68k/dev/ab_io.c ../../sys/arp.c \
    ../../sys/arpquery.c ../../m68k/mch/confrom.c ../../sys/device.c \
    ../../sys/doprintf.c ../../m68k/dev/ec.c ../../m68k/dev/ec_io.c \
    ../../sys/errorlog.c ../../sys/globalsw.c ../../sys/icmp.c \
    ../../m68k/mch/ifree.c ../../m68k/dev/il.c ../../m68k/dev/il_io.c \
    ../../m68k/mch/intr.c ../../sys/ip.c ../../sys/main.c ../../sys/malloc.c \
    ../../m68k/mch/memory.c ../../sys/packet.c ../../m68k/mch/patch.c \
    ../../sys/proc.c ../../m68k/mch/procu.c ../../sys/proto.c \
    ../../m68k/dev/ptm.c ../../sys/queue.c ../../sys/rcp.c ../../sys/timer.c \
    ../../m68k/mch/trap.c ../../sys/troll.c ../../m68k/dev/tty.c \
    ../../sys/udp.c ../../m68k/dev/vv.c ../../m68k/dev/vv_io.c

CIFILES=\
    ../../m68k/dev/abI.c ../../sys/aconfI.c ../../sys/arpI.c \
    ../../m68k/dev/driversI.c ../../m68k/dev/ecI.c ../../m68k/mch/firstI.c \
    ../../m68k/dev/ilI.c ../../m68k/mch/intrI.c ../../sys/ipI.c \
    ../../m68k/mch/memoryI.c ../../sys/packetI.c ../../sys/procI.c \
    ../../m68k/dev/ptmI.c ../../sys/rcpI.c ../../m68k/mch/trapI.c \
    ../../sys/trollI.c ../../m68k/dev/ttyI.c ../../sys/udpI.c \
    ../../m68k/dev/vvI.c

VERSION=\
    ../../sys/version.c

OSFILES=\
    ab.obj ab_io.obj arp.obj arpquery.obj confrom.obj device.obj doprintf.obj \
    ec.obj ec_io.obj errorlog.obj globalsw.obj icmp.obj ifree.obj il.obj \
    il_io.obj intr.obj ip.obj main.obj malloc.obj memory.obj packet.obj \
    patch.obj proc.obj procu.obj proto.obj ptm.obj queue.obj rcp.obj \
    timer.obj trap.obj troll.obj tty.obj udp.obj vv.obj vv_io.obj

OIFILES=\
    abI.obj aconfI.obj arpI.obj driversI.obj ecI.obj firstI.obj ilI.obj \
    intrI.obj ipI.obj memoryI.obj packetI.obj procI.obj ptmI.obj rcpI.obj \
    trapI.obj trollI.obj ttyI.obj udpI.obj vvI.obj

LSFILES=\
    ab.obj, ab_io.obj, arp.obj, arpquery.obj, confrom.obj, device.obj, \
    doprintf.obj, ec.obj, ec_io.obj, errorlog.obj, globalsw.obj, icmp.obj, \
    ifree.obj, il.obj, il_io.obj, intr.obj, ip.obj, main.obj, malloc.obj, \
    memory.obj, packet.obj, patch.obj, proc.obj, procu.obj, proto.obj, \
    ptm.obj, queue.obj, rcp.obj, timer.obj, trap.obj, troll.obj, tty.obj, \
    udp.obj, vv.obj, vv_io.obj

LIFILES=\
    abI.obj, aconfI.obj, arpI.obj, driversI.obj, ecI.obj, ilI.obj, intrI.obj, \
    ipI.obj, memoryI.obj, packetI.obj, procI.obj, ptmI.obj, rcpI.obj, \
    trapI.obj, trollI.obj, ttyI.obj, udpI.obj, vvI.obj

SEDTEXT=\
    ../../m68k/util/text.sed

SEDDATA=\
    ../../m68k/util/data.sed

SEDASM=\
    ../../m68k/util/M68K.sed

#  End of automatically generated symbol definitions.


CFILES=	${CTFILES} ${CSFILES} ${CIFILES} ${CRFILES}


#router:: Makefile
router:: mch.obj lastI.obj firstI.obj abasm.obj ${OSFILES} ${OIFILES}
#rm -f version.obj
#make version.obj
	rm -f router.mem router.map
	$(L68) "router-o0:x$(START)-m:x$(START),router,=mch.obj, \
		version.obj, abasm.obj, \
		${LSFILES},${MATH},firstI.obj,${LIFILES},lastI.obj"
	rm -f router
	../util/mkhdr $(START) > router
	cat router.mem >> router
	rm -f *.lst
	chmod 776 router
	cp router /tmp/r

fakerom:
	mkrom -c ../mch/fakerom.h ../mch/fakerom.rom
	rm confrom.obj
	make confrom.obj

# Makefile is made iff it is invoked explicitly as make makefile
makefile: 
	make makesym
	make depend
	make router

ckp:
	rm -f *.CKP

clean: ckp
	rm -f *.obj *.lst *.asm *.s router

version.obj: version

version:
	@date | cat - ../version.h > version.tmp
	rm -f version.h
	@awk \
	  '{if (NR == 1) date = $$3 "-" $$2 "-" substr($$6,3,2) " " $$4};\
	   $$0 ~ /#define VERSMAKE/ {print $$1 " " $$2 " " $$3 + 1}; \
           $$0 ~ /#define VERSDATE/ {print $$1 " " $$2 " " "\"" date "\""};\
	   $$0 ~ /#define VERSMAJOR/ {print $$0};\
	   $$0 ~ /#define VERSMINOR/ {print $$0};' version.tmp > version.h
	@rm -f version.tmp
	@cp version.h ../version.h
	@echo "Router version:
	@cat version.h

lastI.obj: ${HOME}/${MCH}lastI.asm
	rm -f $*.lst $*.obj
	${A68} $*,$*,=${HOME}/${MCH}lastI.asm

mch.obj: ${HOME}/${MCH}mch.asm
	rm -f $*.lst $*.obj
	${A68} $*,$*,=${HOME}/${MCH}mch.asm

abasm.obj: ${HOME}/${DEV}abasm.asm
	rm -f $*.lst $*.obj
	${A68} $*,$*,=${HOME}/${DEV}abasm.asm

print::
	@cat </dev/null >touch
	@chmod 755 touch

print::
	touch print

#  These default rules are used by "make rules" and not by the standard
#  implicit rule invocations.  In particular, the $* macro is expanded
#  differently (by stripping any leading path and a new $ARG macro is
#  available.

.SUFFIXES: .asm .c .obj
.c.obj:
	${C68} ${COPTS} -S $< > /dev/null
	sed -f ${SEDASM} $ARG $*.s > $*.asm
	rm -f $*.lst  $*.obj
	${A68} $*,$*,=$*
	rm -f $*.s $*.asm

.asm.obj:
	rm -f $*.lst  $*.obj
	${A68} $*,$*,=$<

#  End of non-standard implicit rules


depend:
	@${MAKEDEPEND} Makefile ".:../h:${STD}h" ".obj" ${CFILES}

makesym:
	cp ../util/Makefile Makefile
	csh -f ${MAKESYM} X X X X ${EXPSTD}${H}*.h  \
				  ${EXPSTD}${DEV}*.h \
				  ${EXPSTD}${MCH}*.h > HFILES.tmp
	csh -f ${MAKESYM} X X X X ${EXPSTD}${SYS}version.c >CTFILES.tmp
	csh -f ${MAKESYM} X X X X ${EXPSTD}${DEV}*I.c\
				  ${EXPSTD}${SYS}*I.c \
				  ${EXPSTD}${MCH}*I.c >CIFILES.tmp 
	csh -f ${MAKESYM} X X X X ${EXPSTD}${SYS}version.c \
				  ${EXPSTD}${DEV}*I.c \
				  ${EXPSTD}${DEV}*.c \
				  ${EXPSTD}${MCH}*I.c \
				  ${EXPSTD}${MCH}*.c \
	           		  ${EXPSTD}${SYS}*I.c \
	           		  ${EXPSTD}${SYS}*.c >CSFILES.tmp
	csh -f ${MAKESYM} ".*/" "" "c" obj \
		 ${EXPSTD}${SYS}version.c \
		 ${EXPSTD}${MCH}*I.c \
		 ${EXPSTD}${DEV}*I.c \
		 ${EXPSTD}${SYS}*I.c \
		 ${EXPSTD}${MCH}*.c \
		 ${EXPSTD}${DEV}*.c \
		 ${EXPSTD}${SYS}*.c >OSFILES.tmp
	csh -f ${MAKESYM} X X X X ${EXPSTD}${SYS}version.c >VERSION.tmp
	csh -f ${MAKESYM} ".*/" "" c obj \
		${EXPSTD}${SYS}*I.c \
		${EXPSTD}${MCH}*I.c \
		${EXPSTD}${DEV}*I.c >OIFILES.tmp
	csh -f ${MAKESYM} ".*/" "" "c" obj, \
		 ${EXPSTD}${SYS}version.c \
		 ${EXPSTD}${MCH}*I.c \
		 ${EXPSTD}${DEV}*I.c \
		 ${EXPSTD}${SYS}*I.c \
		 ${EXPSTD}${MCH}*.c \
		 ${EXPSTD}${DEV}*.c \
		 ${EXPSTD}${SYS}*.c \
		 | sed -e '$$s/[, ]*$$//' >LSFILES.tmp
	csh -f ${MAKESYM} ".*/" "" c obj, \
		${EXPSTD}${SYS}*I.c \
		${EXPSTD}${MCH}*I.c \
		${EXPSTD}${MCH}firstI.c \
		${EXPSTD}${DEV}*I.c \
		 | sed -e  '$$s/[, ]*$$//' >LIFILES.tmp
	csh -f ${MAKESYM} X X X X ${EXPSTD}${UTIL}text.sed >SEDTEXT.tmp
	csh -f ${MAKESYM} X X X X ${EXPSTD}${UTIL}data.sed >SEDDATA.tmp
	csh -f ${MAKESYM} X X X X ${EXPSTD}${UTIL}${MACHINE}.sed >SEDASM.tmp
	csh -f ${MAKESYM} X X X X ${EXPSTD}${UTIL}Makefile >MAKESRC.tmp
	cat </dev/null >SED.tmp
	for symb in HFILES CIFILES CTFILES CSFILES VERSION \
		    SFILES \
		    OSFILES OIFILES \
		    LSFILES LIFILES \
		    SEDTEXT SEDDATA SEDASM \
		    MAKESRC; do \
	echo '/^X=/{@s/\(X=\).*/\1\\/@p@r X.tmp@: X@n@/^[ 	]*$$/!b X@}' \
	| tr '@' '\12' \
	| sed -e "s;X;$$symb;g" >>SED.tmp; \
	done
	sed <${MAKESRC} -n -f SED.tmp -e p > Makefile.new
	rm -f Makefile.bak
	mv Makefile Makefile.bak
	mv Makefile.new Makefile
	rm -f *.tmp
	make rules



rules:
	echo '' >rules.tmp
	${MAKERULES} "-f ${SEDTEXT}" .obj ${CTFILES} >>rules.tmp
	${MAKERULES} "-f ${SEDDATA}" .obj ${CIFILES} >>rules.tmp
	${MAKERULES} "" .obj ${SFILES} ${CSFILES} >>rules.tmp
	sed -n <Makefile \
	    -e '/^# <<< RULES /{' \
	    -e 'p' \
	    -e 'r rules.tmp' \
	    -e ': dep' \
	    -e 'n' \
	    -e '/^# >>> RULES /!b dep' \
	    -e '}' \
	    -e 'p' \
	> Makefile.new
	rm -f Makefile.rules
	mv Makefile Makefile.rules
	mv Makefile.new Makefile
	rm -f rules.tmp

# <<< RULES - DO NOT DELETE THIS LINE -- make rules uses it

version.obj: ../../sys/version.c
	${C68} ${COPTS} -S ../../sys/version.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/text.sed version.s > version.asm
	rm -f version.lst  version.obj
	${A68} version,version,=version
	rm -f version.s version.asm

abI.obj: ../../m68k/dev/abI.c
	${C68} ${COPTS} -S ../../m68k/dev/abI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed abI.s > abI.asm
	rm -f abI.lst  abI.obj
	${A68} abI,abI,=abI
	rm -f abI.s abI.asm

aconfI.obj: ../../sys/aconfI.c
	${C68} ${COPTS} -S ../../sys/aconfI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed aconfI.s > aconfI.asm
	rm -f aconfI.lst  aconfI.obj
	${A68} aconfI,aconfI,=aconfI
	rm -f aconfI.s aconfI.asm

arpI.obj: ../../sys/arpI.c
	${C68} ${COPTS} -S ../../sys/arpI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed arpI.s > arpI.asm
	rm -f arpI.lst  arpI.obj
	${A68} arpI,arpI,=arpI
	rm -f arpI.s arpI.asm

driversI.obj: ../../m68k/dev/driversI.c
	${C68} ${COPTS} -S ../../m68k/dev/driversI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed driversI.s > driversI.asm
	rm -f driversI.lst  driversI.obj
	${A68} driversI,driversI,=driversI
	rm -f driversI.s driversI.asm

ecI.obj: ../../m68k/dev/ecI.c
	${C68} ${COPTS} -S ../../m68k/dev/ecI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed ecI.s > ecI.asm
	rm -f ecI.lst  ecI.obj
	${A68} ecI,ecI,=ecI
	rm -f ecI.s ecI.asm

firstI.obj: ../../m68k/mch/firstI.c
	${C68} ${COPTS} -S ../../m68k/mch/firstI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed firstI.s > firstI.asm
	rm -f firstI.lst  firstI.obj
	${A68} firstI,firstI,=firstI
	rm -f firstI.s firstI.asm

ilI.obj: ../../m68k/dev/ilI.c
	${C68} ${COPTS} -S ../../m68k/dev/ilI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed ilI.s > ilI.asm
	rm -f ilI.lst  ilI.obj
	${A68} ilI,ilI,=ilI
	rm -f ilI.s ilI.asm

intrI.obj: ../../m68k/mch/intrI.c
	${C68} ${COPTS} -S ../../m68k/mch/intrI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed intrI.s > intrI.asm
	rm -f intrI.lst  intrI.obj
	${A68} intrI,intrI,=intrI
	rm -f intrI.s intrI.asm

ipI.obj: ../../sys/ipI.c
	${C68} ${COPTS} -S ../../sys/ipI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed ipI.s > ipI.asm
	rm -f ipI.lst  ipI.obj
	${A68} ipI,ipI,=ipI
	rm -f ipI.s ipI.asm

memoryI.obj: ../../m68k/mch/memoryI.c
	${C68} ${COPTS} -S ../../m68k/mch/memoryI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed memoryI.s > memoryI.asm
	rm -f memoryI.lst  memoryI.obj
	${A68} memoryI,memoryI,=memoryI
	rm -f memoryI.s memoryI.asm

packetI.obj: ../../sys/packetI.c
	${C68} ${COPTS} -S ../../sys/packetI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed packetI.s > packetI.asm
	rm -f packetI.lst  packetI.obj
	${A68} packetI,packetI,=packetI
	rm -f packetI.s packetI.asm

procI.obj: ../../sys/procI.c
	${C68} ${COPTS} -S ../../sys/procI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed procI.s > procI.asm
	rm -f procI.lst  procI.obj
	${A68} procI,procI,=procI
	rm -f procI.s procI.asm

ptmI.obj: ../../m68k/dev/ptmI.c
	${C68} ${COPTS} -S ../../m68k/dev/ptmI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed ptmI.s > ptmI.asm
	rm -f ptmI.lst  ptmI.obj
	${A68} ptmI,ptmI,=ptmI
	rm -f ptmI.s ptmI.asm

rcpI.obj: ../../sys/rcpI.c
	${C68} ${COPTS} -S ../../sys/rcpI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed rcpI.s > rcpI.asm
	rm -f rcpI.lst  rcpI.obj
	${A68} rcpI,rcpI,=rcpI
	rm -f rcpI.s rcpI.asm

trapI.obj: ../../m68k/mch/trapI.c
	${C68} ${COPTS} -S ../../m68k/mch/trapI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed trapI.s > trapI.asm
	rm -f trapI.lst  trapI.obj
	${A68} trapI,trapI,=trapI
	rm -f trapI.s trapI.asm

trollI.obj: ../../sys/trollI.c
	${C68} ${COPTS} -S ../../sys/trollI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed trollI.s > trollI.asm
	rm -f trollI.lst  trollI.obj
	${A68} trollI,trollI,=trollI
	rm -f trollI.s trollI.asm

ttyI.obj: ../../m68k/dev/ttyI.c
	${C68} ${COPTS} -S ../../m68k/dev/ttyI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed ttyI.s > ttyI.asm
	rm -f ttyI.lst  ttyI.obj
	${A68} ttyI,ttyI,=ttyI
	rm -f ttyI.s ttyI.asm

udpI.obj: ../../sys/udpI.c
	${C68} ${COPTS} -S ../../sys/udpI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed udpI.s > udpI.asm
	rm -f udpI.lst  udpI.obj
	${A68} udpI,udpI,=udpI
	rm -f udpI.s udpI.asm

vvI.obj: ../../m68k/dev/vvI.c
	${C68} ${COPTS} -S ../../m68k/dev/vvI.c > /dev/null
	sed -f ${SEDASM} -f ../../m68k/util/data.sed vvI.s > vvI.asm
	rm -f vvI.lst  vvI.obj
	${A68} vvI,vvI,=vvI
	rm -f vvI.s vvI.asm

ab.obj: ../../m68k/dev/ab.c
	${C68} ${COPTS} -S ../../m68k/dev/ab.c > /dev/null
	sed -f ${SEDASM}  ab.s > ab.asm
	rm -f ab.lst  ab.obj
	${A68} ab,ab,=ab
	rm -f ab.s ab.asm

ab_io.obj: ../../m68k/dev/ab_io.c
	${C68} ${COPTS} -S ../../m68k/dev/ab_io.c > /dev/null
	sed -f ${SEDASM}  ab_io.s > ab_io.asm
	rm -f ab_io.lst  ab_io.obj
	${A68} ab_io,ab_io,=ab_io
	rm -f ab_io.s ab_io.asm

arp.obj: ../../sys/arp.c
	${C68} ${COPTS} -S ../../sys/arp.c > /dev/null
	sed -f ${SEDASM}  arp.s > arp.asm
	rm -f arp.lst  arp.obj
	${A68} arp,arp,=arp
	rm -f arp.s arp.asm

arpquery.obj: ../../sys/arpquery.c
	${C68} ${COPTS} -S ../../sys/arpquery.c > /dev/null
	sed -f ${SEDASM}  arpquery.s > arpquery.asm
	rm -f arpquery.lst  arpquery.obj
	${A68} arpquery,arpquery,=arpquery
	rm -f arpquery.s arpquery.asm

confrom.obj: ../../m68k/mch/confrom.c
	${C68} ${COPTS} -S ../../m68k/mch/confrom.c > /dev/null
	sed -f ${SEDASM}  confrom.s > confrom.asm
	rm -f confrom.lst  confrom.obj
	${A68} confrom,confrom,=confrom
	rm -f confrom.s confrom.asm

device.obj: ../../sys/device.c
	${C68} ${COPTS} -S ../../sys/device.c > /dev/null
	sed -f ${SEDASM}  device.s > device.asm
	rm -f device.lst  device.obj
	${A68} device,device,=device
	rm -f device.s device.asm

doprintf.obj: ../../sys/doprintf.c
	${C68} ${COPTS} -S ../../sys/doprintf.c > /dev/null
	sed -f ${SEDASM}  doprintf.s > doprintf.asm
	rm -f doprintf.lst  doprintf.obj
	${A68} doprintf,doprintf,=doprintf
	rm -f doprintf.s doprintf.asm

ec.obj: ../../m68k/dev/ec.c
	${C68} ${COPTS} -S ../../m68k/dev/ec.c > /dev/null
	sed -f ${SEDASM}  ec.s > ec.asm
	rm -f ec.lst  ec.obj
	${A68} ec,ec,=ec
	rm -f ec.s ec.asm

ec_io.obj: ../../m68k/dev/ec_io.c
	${C68} ${COPTS} -S ../../m68k/dev/ec_io.c > /dev/null
	sed -f ${SEDASM}  ec_io.s > ec_io.asm
	rm -f ec_io.lst  ec_io.obj
	${A68} ec_io,ec_io,=ec_io
	rm -f ec_io.s ec_io.asm

errorlog.obj: ../../sys/errorlog.c
	${C68} ${COPTS} -S ../../sys/errorlog.c > /dev/null
	sed -f ${SEDASM}  errorlog.s > errorlog.asm
	rm -f errorlog.lst  errorlog.obj
	${A68} errorlog,errorlog,=errorlog
	rm -f errorlog.s errorlog.asm

globalsw.obj: ../../sys/globalsw.c
	${C68} ${COPTS} -S ../../sys/globalsw.c > /dev/null
	sed -f ${SEDASM}  globalsw.s > globalsw.asm
	rm -f globalsw.lst  globalsw.obj
	${A68} globalsw,globalsw,=globalsw
	rm -f globalsw.s globalsw.asm

icmp.obj: ../../sys/icmp.c
	${C68} ${COPTS} -S ../../sys/icmp.c > /dev/null
	sed -f ${SEDASM}  icmp.s > icmp.asm
	rm -f icmp.lst  icmp.obj
	${A68} icmp,icmp,=icmp
	rm -f icmp.s icmp.asm

ifree.obj: ../../m68k/mch/ifree.c
	${C68} ${COPTS} -S ../../m68k/mch/ifree.c > /dev/null
	sed -f ${SEDASM}  ifree.s > ifree.asm
	rm -f ifree.lst  ifree.obj
	${A68} ifree,ifree,=ifree
	rm -f ifree.s ifree.asm

il.obj: ../../m68k/dev/il.c
	${C68} ${COPTS} -S ../../m68k/dev/il.c > /dev/null
	sed -f ${SEDASM}  il.s > il.asm
	rm -f il.lst  il.obj
	${A68} il,il,=il
	rm -f il.s il.asm

il_io.obj: ../../m68k/dev/il_io.c
	${C68} ${COPTS} -S ../../m68k/dev/il_io.c > /dev/null
	sed -f ${SEDASM}  il_io.s > il_io.asm
	rm -f il_io.lst  il_io.obj
	${A68} il_io,il_io,=il_io
	rm -f il_io.s il_io.asm

intr.obj: ../../m68k/mch/intr.c
	${C68} ${COPTS} -S ../../m68k/mch/intr.c > /dev/null
	sed -f ${SEDASM}  intr.s > intr.asm
	rm -f intr.lst  intr.obj
	${A68} intr,intr,=intr
	rm -f intr.s intr.asm

ip.obj: ../../sys/ip.c
	${C68} ${COPTS} -S ../../sys/ip.c > /dev/null
	sed -f ${SEDASM}  ip.s > ip.asm
	rm -f ip.lst  ip.obj
	${A68} ip,ip,=ip
	rm -f ip.s ip.asm

main.obj: ../../sys/main.c
	${C68} ${COPTS} -S ../../sys/main.c > /dev/null
	sed -f ${SEDASM}  main.s > main.asm
	rm -f main.lst  main.obj
	${A68} main,main,=main
	rm -f main.s main.asm

malloc.obj: ../../sys/malloc.c
	${C68} ${COPTS} -S ../../sys/malloc.c > /dev/null
	sed -f ${SEDASM}  malloc.s > malloc.asm
	rm -f malloc.lst  malloc.obj
	${A68} malloc,malloc,=malloc
	rm -f malloc.s malloc.asm

memory.obj: ../../m68k/mch/memory.c
	${C68} ${COPTS} -S ../../m68k/mch/memory.c > /dev/null
	sed -f ${SEDASM}  memory.s > memory.asm
	rm -f memory.lst  memory.obj
	${A68} memory,memory,=memory
	rm -f memory.s memory.asm

packet.obj: ../../sys/packet.c
	${C68} ${COPTS} -S ../../sys/packet.c > /dev/null
	sed -f ${SEDASM}  packet.s > packet.asm
	rm -f packet.lst  packet.obj
	${A68} packet,packet,=packet
	rm -f packet.s packet.asm

patch.obj: ../../m68k/mch/patch.c
	${C68} ${COPTS} -S ../../m68k/mch/patch.c > /dev/null
	sed -f ${SEDASM}  patch.s > patch.asm
	rm -f patch.lst  patch.obj
	${A68} patch,patch,=patch
	rm -f patch.s patch.asm

proc.obj: ../../sys/proc.c
	${C68} ${COPTS} -S ../../sys/proc.c > /dev/null
	sed -f ${SEDASM}  proc.s > proc.asm
	rm -f proc.lst  proc.obj
	${A68} proc,proc,=proc
	rm -f proc.s proc.asm

procu.obj: ../../m68k/mch/procu.c
	${C68} ${COPTS} -S ../../m68k/mch/procu.c > /dev/null
	sed -f ${SEDASM}  procu.s > procu.asm
	rm -f procu.lst  procu.obj
	${A68} procu,procu,=procu
	rm -f procu.s procu.asm

proto.obj: ../../sys/proto.c
	${C68} ${COPTS} -S ../../sys/proto.c > /dev/null
	sed -f ${SEDASM}  proto.s > proto.asm
	rm -f proto.lst  proto.obj
	${A68} proto,proto,=proto
	rm -f proto.s proto.asm

ptm.obj: ../../m68k/dev/ptm.c
	${C68} ${COPTS} -S ../../m68k/dev/ptm.c > /dev/null
	sed -f ${SEDASM}  ptm.s > ptm.asm
	rm -f ptm.lst  ptm.obj
	${A68} ptm,ptm,=ptm
	rm -f ptm.s ptm.asm

queue.obj: ../../sys/queue.c
	${C68} ${COPTS} -S ../../sys/queue.c > /dev/null
	sed -f ${SEDASM}  queue.s > queue.asm
	rm -f queue.lst  queue.obj
	${A68} queue,queue,=queue
	rm -f queue.s queue.asm

rcp.obj: ../../sys/rcp.c
	${C68} ${COPTS} -S ../../sys/rcp.c > /dev/null
	sed -f ${SEDASM}  rcp.s > rcp.asm
	rm -f rcp.lst  rcp.obj
	${A68} rcp,rcp,=rcp
	rm -f rcp.s rcp.asm

timer.obj: ../../sys/timer.c
	${C68} ${COPTS} -S ../../sys/timer.c > /dev/null
	sed -f ${SEDASM}  timer.s > timer.asm
	rm -f timer.lst  timer.obj
	${A68} timer,timer,=timer
	rm -f timer.s timer.asm

trap.obj: ../../m68k/mch/trap.c
	${C68} ${COPTS} -S ../../m68k/mch/trap.c > /dev/null
	sed -f ${SEDASM}  trap.s > trap.asm
	rm -f trap.lst  trap.obj
	${A68} trap,trap,=trap
	rm -f trap.s trap.asm

troll.obj: ../../sys/troll.c
	${C68} ${COPTS} -S ../../sys/troll.c > /dev/null
	sed -f ${SEDASM}  troll.s > troll.asm
	rm -f troll.lst  troll.obj
	${A68} troll,troll,=troll
	rm -f troll.s troll.asm

tty.obj: ../../m68k/dev/tty.c
	${C68} ${COPTS} -S ../../m68k/dev/tty.c > /dev/null
	sed -f ${SEDASM}  tty.s > tty.asm
	rm -f tty.lst  tty.obj
	${A68} tty,tty,=tty
	rm -f tty.s tty.asm

udp.obj: ../../sys/udp.c
	${C68} ${COPTS} -S ../../sys/udp.c > /dev/null
	sed -f ${SEDASM}  udp.s > udp.asm
	rm -f udp.lst  udp.obj
	${A68} udp,udp,=udp
	rm -f udp.s udp.asm

vv.obj: ../../m68k/dev/vv.c
	${C68} ${COPTS} -S ../../m68k/dev/vv.c > /dev/null
	sed -f ${SEDASM}  vv.s > vv.asm
	rm -f vv.lst  vv.obj
	${A68} vv,vv,=vv
	rm -f vv.s vv.asm

vv_io.obj: ../../m68k/dev/vv_io.c
	${C68} ${COPTS} -S ../../m68k/dev/vv_io.c > /dev/null
	sed -f ${SEDASM}  vv_io.s > vv_io.asm
	rm -f vv_io.lst  vv_io.obj
	${A68} vv_io,vv_io,=vv_io
	rm -f vv_io.s vv_io.asm

# >>> RULES - DO NOT DELETE THIS LINE -- make rules uses it

# <<< DEPEND - DO NOT DELETE THIS LINE -- make depend uses it
ab.obj: ../../h/packet.h ../../h/queue.h ../dev/ab.h ../dev/abreg.h
ab.obj: ../mch/device.h cond/ab.h debug/packet.h
abI.obj: ../../h/aconf.h ../../h/ip.h ../../h/packet.h ../../h/queue.h
abI.obj: ../../h/rtypes.h ../dev/ab.h ../dev/abreg.h ../mch/device.h
abI.obj: ../mch/dma.h cond/ab.h debug/packet.h
ab_io.obj: ../../h/arp.h ../../h/packet.h ../../h/proc.h ../../h/proto.h
ab_io.obj: ../../h/queue.h ../dev/ab.h ../dev/abreg.h ../mch/device.h
ab_io.obj: ../mch/mch.h cond/ab.h debug/ab.h debug/packet.h
aconfI.obj: ../../h/aconf.h ../../h/devstats.h ../../h/packet.h ../../h/proto.h
aconfI.obj: ../../h/queue.h ../../h/rtypes.h ../mch/device.h ../mch/intr.h
aconfI.obj: ../mch/mch.h debug/packet.h
arp.obj: ../../h/arp.h ../../h/devstats.h ../../h/errorlog.h ../../h/globalsw.h
arp.obj: ../../h/malloc.h ../../h/packet.h ../../h/profile.h ../../h/proto.h
arp.obj: ../../h/queue.h ../../h/rtypes.h ../../h/time.h ../mch/device.h
arp.obj: ../mch/intr.h ../mch/mch.h cond/ar.h debug/ar.h debug/packet.h
arpI.obj: ../../h/arp.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
arpI.obj: ../../h/time.h ../mch/device.h ../mch/intr.h ../mch/mch.h
arpquery.obj: ../../h/arp.h ../../h/arpquery.h ../../h/packet.h ../../h/proto.h
arpquery.obj: ../../h/queue.h ../../h/rcp.h ../../h/rtypes.h ../../h/time.h
arpquery.obj: ../mch/device.h ../mch/intr.h ../mch/mch.h debug/ar.h
arpquery.obj: debug/packet.h
confrom.obj: ../../h/aconf.h ../../h/devstats.h ../../h/packet.h
confrom.obj: ../../h/proto.h ../../h/queue.h ../../h/rtypes.h ../mch/device.h
confrom.obj: ../mch/fakerom.h ../mch/intr.h ../mch/mch.h debug/packet.h
device.obj: ../../h/packet.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
device.obj: ../mch/device.h ../mch/intr.h ../mch/mch.h debug/packet.h
driversI.obj: ../../h/proto.h ../../h/rtypes.h cond/ab.h cond/ec.h cond/il.h
driversI.obj: cond/ptm.h cond/vv.h
ec.obj: ../../h/devstats.h ../../h/globalsw.h ../../h/packet.h
ec.obj: ../../h/profile.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
ec.obj: ../dev/ec.h ../dev/ecreg.h ../mch/device.h ../mch/intr.h ../mch/mch.h
ec.obj: cond/ec.h debug/packet.h
ecI.obj: ../../h/aconf.h ../../h/devstats.h ../../h/ip.h ../../h/packet.h
ecI.obj: ../../h/proto.h ../../h/queue.h ../../h/rtypes.h ../dev/ec.h
ecI.obj: ../dev/ecreg.h ../mch/cable.h ../mch/device.h ../mch/dma.h
ecI.obj: ../mch/intr.h ../mch/mch.h cond/ec.h debug/packet.h cond/site.h
ec_io.obj: ../../h/devstats.h ../../h/errorlog.h ../../h/packet.h
ec_io.obj: ../../h/profile.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
ec_io.obj: ../dev/ec.h ../dev/ecreg.h ../mch/device.h ../mch/intr.h
ec_io.obj: ../mch/mch.h cond/chaos.h cond/ec.h debug/ec.h debug/packet.h
errorlog.obj: ../../h/errorlog.h ../../h/globalsw.h ../../h/ip.h
errorlog.obj: ../../h/packet.h ../../h/proto.h ../../h/queue.h ../../h/rcp.h
errorlog.obj: ../../h/rtypes.h ../../h/udp.h ../mch/device.h ../mch/intr.h
errorlog.obj: ../mch/mch.h debug/packet.h
firstI.obj: ../../h/malloc.h ../../h/rtypes.h
globalsw.obj: ../../h/arp.h ../../h/globalsw.h ../../h/queue.h ../../h/rtypes.h
icmp.obj: ../../h/devstats.h ../../h/icmp.h ../../h/ip.h ../../h/logbad.h
icmp.obj: ../../h/packet.h ../../h/profile.h ../../h/proto.h ../../h/queue.h
icmp.obj: ../../h/rtypes.h ../../h/time.h ../mch/device.h ../mch/intr.h
icmp.obj: ../mch/mch.h cond/egp.h debug/icmp.h debug/ip.h debug/packet.h
il.obj: ../../h/devstats.h ../../h/globalsw.h ../../h/packet.h
il.obj: ../../h/profile.h ../../h/proto.h ../../h/queue.h ../../h/rcp.h
il.obj: ../../h/rtypes.h ../dev/il.h ../dev/ilreg.h ../mch/device.h
il.obj: ../mch/intr.h ../mch/mch.h cond/il.h debug/packet.h
ilI.obj: ../../h/aconf.h ../../h/devstats.h ../../h/ip.h ../../h/packet.h
ilI.obj: ../../h/proto.h ../../h/queue.h ../../h/rtypes.h ../dev/il.h
ilI.obj: ../dev/ilreg.h ../mch/cable.h ../mch/device.h ../mch/dma.h
ilI.obj: ../mch/intr.h ../mch/mch.h cond/il.h debug/packet.h cond/site.h
il_io.obj: ../../h/devstats.h ../../h/errorlog.h ../../h/packet.h
il_io.obj: ../../h/profile.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
il_io.obj: ../dev/il.h ../dev/ilreg.h ../mch/device.h ../mch/intr.h
il_io.obj: ../mch/mch.h cond/chaos.h cond/il.h debug/il.h debug/packet.h
intr.obj: ../../h/aconf.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
intr.obj: ../mch/device.h ../mch/intr.h ../mch/mch.h
intrI.obj: ../../h/aconf.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
intrI.obj: ../mch/device.h ../mch/intr.h ../mch/mch.h
ip.obj: ../../h/arp.h ../../h/devstats.h ../../h/errorlog.h ../../h/globalsw.h
ip.obj: ../../h/icmp.h ../../h/ip.h ../../h/logbad.h ../../h/packet.h
ip.obj: ../../h/profile.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
ip.obj: ../../h/time.h ../../h/udp.h ../mch/device.h ../mch/intr.h ../mch/mch.h
ip.obj: cond/egp.h debug/ip.h debug/packet.h
ipI.obj: ../../h/icmp.h ../../h/ip.h ../../h/proto.h ../../h/queue.h
ipI.obj: ../../h/rcp.h ../../h/rtypes.h ../../h/time.h ../../h/udp.h
ipI.obj: ../mch/device.h ../mch/intr.h ../mch/mch.h cond/egp.h
main.obj: ../../h/globalsw.h ../../h/malloc.h ../../h/packet.h ../../h/proc.h
main.obj: ../../h/proto.h ../../h/queue.h ../../h/rtypes.h ../../h/time.h
main.obj: ../mch/mch.h ../mch/memory.h ../mch/procu.h debug/packet.h
malloc.obj: ../../h/malloc.h ../../h/rtypes.h
memoryI.obj: ../../h/malloc.h ../../h/rtypes.h ../mch/mch.h ../mch/memory.h
packet.obj: ../../h/packet.h ../../h/queue.h ../../h/rtypes.h ../mch/mch.h
packet.obj: debug/packet.h
packetI.obj: ../../h/aconf.h ../../h/packet.h ../../h/proto.h ../../h/queue.h
packetI.obj: ../../h/rtypes.h ../mch/device.h ../mch/dma.h ../mch/intr.h
packetI.obj: ../mch/mch.h debug/packet.h
patch.obj: ../../h/aconf.h ../../h/devstats.h ../../h/packet.h ../../h/proto.h
patch.obj: ../../h/queue.h ../../h/rtypes.h ../mch/device.h ../mch/intr.h
patch.obj: ../mch/mch.h ../mch/patch.h debug/packet.h
proc.obj: ../../h/packet.h ../../h/proc.h ../../h/queue.h ../../h/rtypes.h
proc.obj: ../mch/mch.h ../mch/procu.h debug/packet.h
procu.obj: ../../h/proc.h ../../h/queue.h ../../h/rtypes.h ../mch/procu.h
proto.obj: ../../h/proto.h ../../h/queue.h ../../h/rtypes.h ../mch/device.h
proto.obj: ../mch/intr.h ../mch/mch.h
ptm.obj: ../../h/proto.h ../../h/queue.h ../../h/rtypes.h ../../h/time.h
ptm.obj: ../dev/ptmreg.h ../mch/device.h ../mch/intr.h ../mch/mch.h
ptmI.obj: ../../h/aconf.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
ptmI.obj: ../../h/time.h ../dev/ptmreg.h ../mch/device.h ../mch/intr.h
ptmI.obj: ../mch/mch.h
queue.obj: ../../h/queue.h
rcp.obj: ../../h/aconf.h ../../h/devstats.h ../../h/errorlog.h
rcp.obj: ../../h/globalsw.h ../../h/ip.h ../../h/packet.h ../../h/profile.h
rcp.obj: ../../h/proto.h ../../h/queue.h ../../h/rcp.h ../../h/rtypes.h
rcp.obj: ../../h/udp.h ../mch/device.h ../mch/dma.h ../mch/intr.h ../mch/mch.h
rcp.obj: cond/rcppswd.h debug/rcp.h debug/packet.h
rcpI.obj: ../../h/ip.h ../../h/proto.h ../../h/queue.h ../../h/rcp.h
rcpI.obj: ../../h/rtypes.h ../../h/udp.h ../mch/device.h ../mch/intr.h
rcpI.obj: ../mch/mch.h
timer.obj: ../../h/proto.h ../../h/queue.h ../../h/rtypes.h ../../h/time.h
timer.obj: ../mch/device.h ../mch/intr.h ../mch/mch.h
trap.obj: ../../h/proc.h ../../h/queue.h ../../h/rtypes.h ../mch/mch.h
trap.obj: ../mch/procu.h ../mch/trap.h
trapI.obj: ../../h/rtypes.h ../mch/mch.h
troll.obj: ../../h/proto.h ../../h/queue.h ../../h/rtypes.h ../../h/troll.h
trollI.obj: ../../h/queue.h ../../h/rtypes.h ../../h/troll.h
tty.obj: ../../h/devstats.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
tty.obj: ../dev/ttreg.h ../mch/device.h ../mch/intr.h ../mch/mch.h
ttyI.obj: ../../h/aconf.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
ttyI.obj: ../dev/ttreg.h ../mch/device.h ../mch/intr.h ../mch/mch.h
udp.obj: ../../h/devstats.h ../../h/errorlog.h ../../h/ip.h ../../h/packet.h
udp.obj: ../../h/profile.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
udp.obj: ../../h/udp.h ../mch/device.h ../mch/intr.h ../mch/mch.h
udp.obj: debug/packet.h
udpI.obj: ../../h/ip.h ../../h/proto.h ../../h/queue.h ../../h/rtypes.h
udpI.obj: ../../h/udp.h ../mch/device.h ../mch/intr.h ../mch/mch.h cond/wt.h
version.obj: version.h
vv.obj: ../../h/devstats.h ../../h/packet.h ../../h/profile.h ../../h/proto.h
vv.obj: ../../h/queue.h ../../h/rtypes.h ../../h/time.h ../dev/vv.h
vv.obj: ../dev/vvreg.h ../mch/device.h ../mch/intr.h ../mch/mch.h cond/vv.h
vv.obj: debug/vv.h debug/packet.h
vvI.obj: ../../h/aconf.h ../../h/ip.h ../../h/packet.h ../../h/proto.h
vvI.obj: ../../h/queue.h ../../h/rtypes.h ../dev/vv.h ../dev/vvreg.h
vvI.obj: ../mch/device.h ../mch/dma.h ../mch/intr.h ../mch/mch.h cond/vv.h
vvI.obj: debug/packet.h
vv_io.obj: ../../h/devstats.h ../../h/errorlog.h ../../h/packet.h
vv_io.obj: ../../h/proc.h ../../h/profile.h ../../h/proto.h ../../h/queue.h
vv_io.obj: ../../h/rtypes.h ../dev/vv.h ../dev/vvreg.h ../mch/device.h
vv_io.obj: ../mch/intr.h ../mch/mch.h cond/chaos.h cond/vv.h debug/vv.h
vv_io.obj: debug/packet.h
# >>> DEPEND - DO NOT DELETE THIS LINE -- make depend uses it

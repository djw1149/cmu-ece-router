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
    ${STD}m68k/util/Makefile
 
HFILES=

CTFILES=

CSFILES=

CIFILES=

VERSION=

OSFILES=

OIFILES=

LSFILES=

LIFILES=

SEDTEXT=

SEDDATA=

SEDASM=

#  End of automatically generated symbol definitions.


CFILES=	${CTFILES} ${CSFILES} ${CIFILES} ${CRFILES}


#router:: Makefile
router:: mch.obj lastI.obj firstI.obj abasm.obj ${OSFILES} ${OIFILES}
#	rm -f version.obj
#	make version.obj
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

# >>> RULES - DO NOT DELETE THIS LINE -- make rules uses it

# <<< DEPEND - DO NOT DELETE THIS LINE -- make depend uses it

# >>> DEPEND - DO NOT DELETE THIS LINE -- make depend uses it

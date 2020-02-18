makerom.obj: makerom.c
	cl -c -Ze makerom.c

mch.obj: ../mch/mch.asm
	masm ..\mch\mch.asm,,mch;

makerom.exe: makerom.obj mch.obj
	link makerom.obj+mch.obj;;;makerom.exe

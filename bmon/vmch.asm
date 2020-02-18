	SECTION 0
*
* djw 9/8/86: changed section 9 to 0 to go before crt68.obj in the output
* 
ROMSTART EQU	$fe0000
LOWMEM	EQU	$1000

S_RAM_OFFSET	EQU	14-8192		* Reserve 8K at top of ram.

	JMP	start
*	DC.L	$400
*	DC.L	start

	XREF main
	XREF init
	XDEF restart
	XDEF start
	XDEF strboot
	XDEF monstart
	XDEF reboot

reboot
		RESET
		MOVE.W	#$2700,SR
		MOVEQ	#1,D7		* Reboot per configuration or default
		JMP	reset$

start
monstart
restart
		RESET
		MOVE.W	#$2700,SR
		MOVEQ	#0,D7		* start the boot monitor
		JMP	reset$

strboot
		RESET
		MOVE.W	#$2700,SR
		MOVE.L	6(SP),D7
		JMP	reset$

	XDEF trap15

trap15		MOVE.L	4(SP),-(SP)
		TRAP	#15
		

	XREF	trap
	XDEF	catchjsr

catchjsr	JSR	catch$
catch$		MOVEM.L	A0-A7/D0-D7,-(SP)
		MOVE.L	64(SP),-(SP)
		JSR	trap
		ADDQ.L	#4,SP
		MOVEM.L	(SP)+,A0-A7/D0-D7
		ADDQ.L	#4,SP
		RTE

reset$		MOVE.L	#LOWMEM,SP
		MOVE.L	#$1000,D2
		MOVE.B	#$FF,D1
		MOVE.L	#L2,$8

L1		MOVE.W	(SP),A1		* save the test location
		MOVE.B	D1,(SP)
		CMP.B	(SP),D1
		BNE	L3
		MOVE.W	A1,(SP)		* restore the test location
		ADD.L	D2,SP
		BRA	L1

L2		ADD.L	#S_RAM_OFFSET,SP	* djw & bob
		MOVE.L	SP,D6			* djw & bob

L3		RESET
		MOVE.W	#$2700,SR
		MOVE.L	D6,-(SP)		* djw & bob: push S_RAM...
		MOVE.L	D7,-(SP)		* push main's arg
		JSR	init
		MOVE.L	#buserr,$8
		JSR	main
		JMP	start

	XDEF	probeb
	XDEF	probew
	XDEF	probel

probeb
	CLR.L	$400
	MOVE.L	A0,-(SP)
	MOVE.L	8(SP),A0
	TST.B	(A0)
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	MOVE.L	(SP)+,A0
	MOVE.L	$400,D0
	RTS

probew
	CLR.L	$400
	MOVE.L	A0,-(SP)
	MOVE.L	8(SP),A0
	TST.W	(A0)
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	MOVE.L	(SP)+,A0
	MOVE.L	$400,D0
	RTS

probel
	CLR.L	$400
	MOVE.L	A0,-(SP)
	MOVE.L	8(SP),A0
	TST.L	(A0)
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	MOVE.L	(SP)+,A0
	MOVE.L	$400,D0
	RTS

buserr	ADDQ.L	#2,SP
	MOVE.L	(SP)+,$400
	ADDQ.L	#2,SP
	RTE

*
*	this is the wait for intr routine
*
	XDEF	wait
wait	STOP	#$2000
	RTS

	XDEF cret
cret		MOVE.L	4(SP),A0
		MOVEM.L	4(A0),D2-D7/A2-A7
		MOVE.L	(A0),(SP)
		MOVEQ	#0,D0
		RTS

	XDEF	csave
csave		MOVE.L	4(SP),A0
		MOVE.L	(SP),D0
		MOVE.L	D0,(A0)
		MOVEM.L	D2-D7/A2-A7,4(A0)
		RTS
*
*
*	stack, pc, and status reg set and save routines
*
	XDEF	spval
spval		MOVE.L	SP,D0
		ADDQ.L	#4,D0
		RTS

	XDEF	spset
spset		MOVE.L	(SP)+,A0
		MOVE.L	(SP)+,SP
		MOVE.L	SP,-(SP)
		MOVE.L  A0,-(SP)
		RTS

	XDEF	srset
srset		MOVE.W	SR,D0
		MOVE.W	6(SP),SR
		RTS

	XDEF	srval
srval		MOVEQ	#0,D0
		MOVE.W	SR,D0
		RTS

*
*	these are the set priority level routines
*
	XDEF spl
spl		MOVEQ	#0,D0
		MOVE.W	SR,D0
		MOVE.L	4(SP),D1
		BSET	#13,D1
		MOVE.W	D1,SR
		RTS

	XDEF spl0
spl0		MOVEQ	#0,D0
		MOVE.W 	SR,D0
		MOVE.W	#$2000,SR
		RTS

	XDEF spl5
spl5		MOVEQ	#0,D0
		MOVE.W	SR,D0
		MOVE.W	#$2500,SR
		RTS

	XDEF spl6
spl6		MOVEQ	#0,D0
		MOVE.W	SR,D0
		MOVE.W	#$2600,SR
		RTS

	XDEF spl7
spl7		MOVEQ	#0,D0
		MOVE.W	SR,D0
		MOVE.W	#$2700,SR
		RTS

*
*	these are the swap byte, word, and long routines
*

	XDEF swab
swab		MOVE.L	4(SP),D0
		ROL.W	#8,D0
		RTS

	XDEF swal
swal		MOVE.L	4(SP),D0
		ROL.W	#8,D0
		SWAP	D0
		ROL.W	#8,D0
		RTS

	XDEF swap
swap		MOVE.L	4(SP),D0
		SWAP	D0
		RTS

*
* exit and return to monitor command
*

	XDEF exit
exit	MOVE.L	ROMSTART,A7
        MOVE.L	ROMSTART+4,A0
	JMP	(A0)

*
*	these are the bstring routines
*

	XDEF bcopy
bcopy		MOVE.L	A2,-(SP)
		MOVE.L	8(SP),A1
		MOVE.L	12(SP),A2
		MOVE.L	16(SP),D1
		BRA	enter1

loop1		MOVE.B (A1)+,(A2)+
enter1		DBF	D1,loop1

end1		MOVE.L	(SP)+,A2
		RTS

	XDEF bzero
bzero		MOVE.L	4(SP),A1
		MOVE.L	8(SP),D1
		CLR.B	D0
		BRA	enter2

loop2		MOVE.B D0,(A1)+
enter2		DBF	D1,loop2

end2		RTS

	XDEF bcmp
bcmp		MOVE.L	A2,-(SP)
		MOVE.L	8(SP),A1
		MOVE.L	12(SP),A2
		MOVE.L	16(SP),D0

		SUB.W	#1,D0
		BLT	end3

loop3		CMPM.B	(A1)+,(A2)+
		DBNE	D0,loop3
		ADDQ.W	#1,D0

end3		MOVE.L	(SP)+,A2
		RTS

		XDEF bswab
bswab		MOVE.L	4(SP),A1
		MOVE.L	8(SP),D1
		BRA	enter4

loop4		MOVE.W (A1),D0
		ROL.W	#8,D0
		MOVE.W	D0,(A1)+
enter4		DBF	D1,loop4

end4		RTS

	END

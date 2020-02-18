	include   cond/abasm.h
*___________________________________________________________________________ 
* modified by david waitzman (djw) 5/21/85 to oasys assembler format
*
* (this file was pieced together from mpp, mppmacros, mppdefs, lapdefs)
*
* lapdefs - equates for ablap part of mpp
*       (.include after mppdefs)
*
* march-april, 1984
* larry kenyon and alan oppenheimer
* version 1.0a  as listed for developer's conference
* version 0.2a  back from lk 4/30/84
* version 0.1a
*
* copyright  1984 apple computer
*
* 
* mpp handler (lap only) 
* 
* alan oppenheimer and larry kenyon 
* march-april, 1984 
* 
* version 1.0a as listed for developer's conference, 4/30/84 
* 
* copyright  1984 apple computer inc. 
* 
* apple computer assumes no responsibility for the correct operation of 
* this software. nor does apple assume responsibility for any errors it 
* may contain, or have any liabilities or obligations arising out of or in 
* connection with the use of this software. 
* 
* history 
* 04/30/84 AO&LK Released in MPP at 1st developer's conference. 
* 07/30/84 croft Distilled LAP only; C callable. 
* 08/06/84 croft Parameterize delay loops. 
* 08/20/84 croft Coexist with SUN board software refresh. 
* 05/21/85 waitzman to oasys assembler format
* 06/17/85 waitzman starting conversion to bob barker's abus board->
*	        will make complete packets and ship to upper levels.
*		code to be similar to standard ece drivers.
* 06/29/85 waitzman put through cusinart
* Copyright  1984, Stanford SUMEX project; may be 
* used but not sold without permission. 
*___________________________________________________________________________ 
		
statcount MACRO
	  ENDM

****
*  symbolic offsets are defined relative to the start of the headers
****
lapdstadr EQU	0               * destination node address
lapsrcadr EQU	1               * source node address
laptype	  EQU	2               * lap type field

****
*  lap error codes (we're using system numbers so we can use moveq's)
****
overrunerr	EQU	-1
crcerr		EQU	-2
underrunerr	EQU	-3
atterr		EQU	-4                  * error in attach call
deterr		EQU	-5                  * error in detach call
excessdefers	EQU	-6
excesscollsns	EQU	-7


****
*  data link protocol values
****

dstadros	EQU	0	* offset in segment
srcadros	EQU	1
laptypeos	EQU	2	* type field

lapenq		EQU	$81	* enquire whether station address in use
lapack		EQU	$82	* say it is
laprts		EQU	$84	* request to send
lapcts		EQU	$85	* clear to send

qlapenq		EQU	-127	* equates for moveq
qlapack		EQU	-126
qlaprts		EQU	-124
qlapcts		EQU	-123

****
*  scc write register equates
****

*___________________*
*                   *
* wr0 commands      *
*___________________*

restxcrc	EQU	$80	* reset transmit crc generator
resetund	EQU	$c0	* reset underrun latch
reseterr	EQU	$30	* reset errors
resetius	EQU	$38	* reset highest outstanding interrupt
reenbint	EQU	$20	* re-enable first character interrupts
resetext	EQU	$10	* reset external interrupts

*___________________*
*                   *
* wr1 commands      *
*___________________*
* wr1 commands - receive interrupt control
extie		EQU	$01	* external (mouse) interrupt enable
rxie		EQU	$08	* first character interrupt enable

*___________________*
*                   *
* wr3 commands      *
*___________________*
* wr3 commands - receiver control
enbrxslv	EQU	$dd	* enable receiver in search mode
disrx		EQU	$d0	* disable receiver

*___________________*
*                   *
* wr4 commands      *
*___________________*

sdlcmode	EQU	$20	* sdlc mode

*___________________*
*                   *
* wr5 commands      *
*___________________*
* wr5 commands - transmitter control
distxrts	EQU	$60	* disable transmitter and drivers
enbtxrts	EQU	$6b	* enable transmitter and drivers
enbrts		EQU	$62	* just enable drivers

*___________________*
*                   *
* wr6 is addr reg   *
*___________________*

addrreg		EQU	$06

*___________________*
*                   *
* wr7 is sdlc flag  *
*___________________*

sdlc		EQU	$7e	* 01111110

*___________________*
*                   *
* wr9 commands      *
*___________________*
* wr9 commands - master resets
reseta		EQU	$80	* reset channel a
mie		EQU	$0a	* master interrupt enable

*___________________*
*                   *
* wr10 commands     *
*___________________*

setfm0		EQU	$e0	* use fm0 transmission
setnrz		EQU	$00	* use nrz (reset state)

*___________________*
*                   *
* wr11 commands     *
*___________________*

clocks		EQU	$70	* dpll receive, baud rate gen. xmit

*___________________*
*                   *
* wr12, wr13 brg    *
*___________________*

bdratelo	EQU	12	* wr12 has low order part
bdratehi	EQU	13	* wr13 has high order part
baudrate	EQU	6	* constant for 230.4 kbaud

*___________________*
*                   *
* wr14 commands     *
*___________________*
* wr14 commands - phase locked loop control
fmmode		EQU	$c0	* fm mode
srchmode	EQU	$23	* enter ppl search mode, enable brg
resetclks	EQU	$43	* reset missing clocks, enable brg

*___________________*
*                   *
* wr15 commands     *
*___________________*
* wr15 commands - external interrupt enables
dcdie		EQU	$08	* dcd (mouse) interrupt enable


*_______________________________________*
*                                       *
*     scc read register status codes    *
*_______________________________________*

*___________________*
*                   *
* rr0 status codes  *
*___________________*

rcabit		EQU	0	* receive character available bit
txemptybit	EQU	2	* transmitter empty bit
hunt		EQU	4	*
undrunbit	EQU	6	* transmitter underrun bit

*___________________*
*                   *
* rr1 status codes  *
*___________________*

ovrrunbit	EQU	5	* receiver overrun bit
crcbit		EQU	6	* receiver crc error bit
eoframebit	EQU	7	* end-of-frame bit
checkbits	EQU	$a0	* eofr or overrun (for quick check)
ovrorcrc	EQU	$60	* overrun or crc (for check at end)


****
*
* From C structures passed to us:
*
* struct abdevdep {
backoff		EQU	0	* current backoff range 
defertries 	EQU	2	* local count of defers in one send
colsntries	EQU	4	* local count of collisions in one send
mynet		EQU	6	* my apple net address 
mynaddr		EQU	8	* my apple node address 
fadrvalid	EQU	9	* my apple node address has been verified 
fadrinuse	EQU	10	* 1 if valid packet is received & fadrvalid=0 
colsnhist	EQU	11	* bit history of collisions last 8 sends 
deferhist	EQU	12	* bit history of defers last 8 sends 
fgoodcts	EQU	13	* set by recvpkt if a valid cts is received 
* }; EQU 14
*
****

	SECTION 14	* ?djw? is this the right section to use?
* Local vars used in each invocation of routines in this file
abusvars	DC.L 0	* used to point to the passed devdep struct 
sccrd		DC.L 0	* the passed address of the SCC
tlen		DC.L 0	* the length of the data to transmit
packet		DC.L 0	* the address of the packet (the data area)
lbackoff	DC.W 0  * local backoff mask
packetout	DC.L 0  * a work packet for control packet sends
torha		DC.L 0  * a read ahead storage area (punt)
save_d2		DC.L 0	* for cheapness (could overlay most other things)
destnode	DC.B 0	* the destination node address
syslapaddr	DC.B 0	* a copy of our node address
fsendenq	DC.B 0  * flag that we are sending an enq lap packet

* scc control and data offsets 
bctl		EQU 0
bdata		EQU 1
actl		EQU 2
adata		EQU 3
sccdata		EQU 1	* the constant will use for putting data away
		
maxdefers  	EQU 32 	* max defers before send routine gives up 
maxcollsns 	EQU 50 	* ibid for collisions  was 32
		
* 
* Delay loop timers. The original code released 4/30 had many 
* cryptic delay constants sprikled through it. On an 8 MHz SUN board, 
* each instruction in a timing loop seems to take about a usec. Most 
* of the delay loops are of the form loop test bit; dbne to loop. 
* So the loop count * 2 gives roughly the number of usecs. 
* 
* Delays were originally set up for a ~6 MHz Macintosh; the multipliers 
* below adjust this for an 8(or ~10) MHz SUN board. 
* 
DLN	EQU	2	* delay numerator 
DLD	EQU	1	* delay denominator 

* 
* SCC access recovery delay 6 * PCLK + 200 = 1.7 usec 
* 
sccDL	MACRO			* was actually 2.7 usec with real seagate
	move.l (SP),(SP)	* added by djw for 10mhz 68k
	ENDM

*
* Delay constants used throughout:
*
DL4maxpack  EQU	45000*DLN/DLD	* (was 45000) about 90msec = ~4 max packets
DL50usec    EQU	32		* (was 27) accurate on my SUN board 
DL400usec   EQU	240*DLN/DLD	* (was 200)
DL2flags    EQU	20*DLN/DLD	* (was 16) one abus byte SHOULD=35usec 
DLtxunder   EQU	48*DLN/DLD	* (was 40) tx underrun if not ready in 60 usec
DLabort     EQU	63*DLN/DLD	* (was 53) abus byte=35usec, about 1.5-2 bytes
DL2bytes    EQU	72*DLN/DLD	* (was 60)
DL1byte	    EQU	50*DLN/DLD	* (was 30)


	
	SECTION 9		
		
*___________________________________________________________________________ 
* 
* routine abuswrite - send a packet on the applebus 
* cdsendenq - see if someone is using the number we want . . . 
* 
* arguments
* A2 (input)-- local variables ptr 
* D2 (input)-- lap destination address 
* A1 (input)-- packet
* D0 (output)-- status 
* uses D1-D3,A0,a1,A3 
* 
* function send the next packet in the queue, adhering to 
* applebus link access protocol. this implements the special 
* provisions for csma/ca on applebus. 
*
* here we use a 4 bit global backoff mask to initialize our 
* local backoff mask which can grow to 8 bits due to collisions 
* while trying to send this packet. the global mask can only 
* change by 1 bit either way per call to this subroutine. 
*___________________________________________________________________________ 

****
*
* write a packet to abus 
* int abuswrite(struct abdevice *, struct abdevdep, struct packet *, length)
*
**** 
	XDEF	abuswrite 
abuswrite
	move.l	16(SP),tlen
	move.l	12(SP),packet
	move.l	8(SP),abusvars		* fetch abusvars pointer from C
	move.l	4(SP),sccrd		* adjust to where the A port is
	addq.l	#2,sccrd		* 	0x30012
	move.l	sccrd,A0		* get scc address

	movem.l	D2/D3/A2/A3,-(SP)	* don't tread on C

	move.l	abusvars,A2		* load abusvars pointer

	move.l	packet,A1		* get dest addr in D2 
	move.b	(A1),D2
	move.b	D2,destnode		* remember destination node 

	move.b	mynaddr(A2),syslapaddr	* put my naddr in a convenient place

	cmp.b	#lapenq,laptype(A1)	* if type is LapEnq
	seq	fsendenq		* set flag saying so

	bsr	abwrite
	movem.l	(SP)+,D2/D3/A2/A3
	rts	

****
*		
* Assembler level only write routines
*
****		
		
* {wait for any packet in progress} 
	LOCAL
abwrite
* cds_pktwait			
* _spl0 	* (mac spl1) 
	btst	#hunt,(A0)	* if carriersense then begin 
	bne	cds_bkoff	* (if still hunting, then line idle) 
	bset	#0,lbackoff	* backoff = max( backoff, 2); 
	bset	#0,deferhist(A2) * deferhist[0] = 1; 
		
	move.l	#DL4maxpack,D0	* {extra long for "in packet" timer} 
_1	btst	#hunt,(A0)	* while carriersense 
	dbne	D0,_1		* do nothing; 
	bne	cds_bkoff	* {br if we're no longer in a packet} 
* end; 
		
	bsr	resetrcvr	* {reset receiver in case of bizarreness} 
	statcount idletocount
		
* {wait for minimum interpacketgap time plus a random backoff} 

	LOCAL

cds_bkoff			
	bsr	randomword	* figure out a random number 4 bit number
	and.w	lbackoff,D0	* then mask with the local backoff
	beq	_2		* br if no random backoff 
		
_1	bsr	wait100usec	* call wait routine (also checks the line) 
	dbeq	D0,_1
	beq	cds_busy	* br if the line is busy . . . 
		
_2			
* _splabus 	* (mac spl3) 
	moveq	#3,D0		* always wait 4 (*100usec) for idg time 
	move.b	#14,(A0)	* reset missing clocks line going in . . . 
	sccDL
	move.b	#resetclks,(A0)
		
_3	bsr	wait100usec	* call wait routine (also checks the line) 
	dbeq	D0,_3
	beq	cds_busy	* br if the line is busy . . . 
		
	move.b	#10,(A0)	* set up to read missing clock reg 
	sccDL
	move.b	destnode,D2	* figure out destination and delay 
	move.b	(A0),D1		* missing clock set since clear? 
	bpl	csendrts	* br if not 
	LOCAL
		
cds_busy			
	statcount defercount
	addq.w	#1,defertries(A2)
	moveq	#excessdefers,D0	* assume exceeded defer max 
*					  excessdefers is an error code
	cmp.w	#maxdefers,defertries(A2)
	bge	cdsendxit		* exit if so 
	bra	abwrite			* otherwise, try again 
		
wait100usec			
	moveq	#1,D1			* 50usec if busy, 100usec if idle 
_2	moveq	#DL50usec,D3
_1	subq.l	#1,D3
	bne	_1
	btst	#hunt,(A0)		* are we outside packet? 
	dbeq	D1,_2
	rts				* bne for ok 
		
* {before sending the rts or enq we send out a 
* synchronizing transition on the bus} 
		
csendrts			
	move.b	#5,(A0)		* address wr5 
	sccDL
	moveq	#qlaprts,D0	* set code for rts (and delay) 
	move.b	#enbrts,(A0)	* enable drivers only 
	sccDL			* ?djw? put in, but is this needed???
* {wait at least one bit time} 
		
	tst.b	fsendenq	* do we want to send enq? 
	beq	_10		* br if not 
	moveq	#qlapenq,D0	* set code for enq then 
* {add check for fadrinuse here . . } 
		
_10	move.b	#5,(A0)		* address wr5 
	sccDL
	move.b	#distxrts,(A0)	* then disable until we really start 
		
	bsr	sendxxx		* go send it 

	tst.b	fsendenq	* did we send an enq? 
	bne	waitack		* then go wait for ack 
		
	cmp.b	#$ff,destnode	* check if broadcast 
	bne	waitcts		* br if not 
		
* {for broadcasts, just idle for 200 usec} 
		
	moveq	#1,D0
_20	bsr	wait100usec	* call our wait routine (which also checks the line) 
	dbeq	D0,_20
	bne	sendpkt		* no, assume all is ok for us 
		
cds_collision			
* 	_spl0 	* (mac spl0) 
	statcount collsncount	* count collisions 
	bset	#0,colsnhist(A2)
	move.w	lbackoff,D0
	or.w	#$10,SR		* set x-bit
	roxl.w	#1,D0
	move.w	D0,lbackoff	* increase local backoff mask to max 8 bits 
		
	addq.w	#1,colsntries(A2)
	moveq	#excesscollsns,D0	* assume exceeded collision max 
	cmp.w	#maxcollsns,colsntries(A2)
	bgt	cdsendxit	* exit if so 
	bra	abwrite		* otherwise, try again 
		
* {wait for cts} 
		
waitcts			
	clr.b	fgoodcts(A2)	* clear flag 
	bsr	getxxx
	tst.b	fgoodcts(A2)	* did we get a good one? 
	beq	cds_collision	* br if not 
		
* {send data packet} 
		
sendpkt			
	bsr	sendpacket	* so send the data packet 
		
cdsendok			
	moveq	#0,D0		* success! 
	statcount xmitcount	* count of good data packets xmitted 
		
cdsendxit			
	rts			* and return 
		
* {wait for cts/ack} 
		
waitack			
	bsr	getxxx
	tst.b	fadrinuse(A2)	* well, have we received anything for this address? 
	bne	cdsendok	* exit if so . . . need to find a new number 
	bra	cds_collision	* otherwise, assume collision . . . 
		
* getxxx - wait for RCA, then recvpkt 
		
getxxx	move.l	#DL400usec,D1	* wait for first byte of message ...
_30	btst	#rcabit,(A0)	* but only wait 400 usec 
	dbne	D1,_30
	bne	recvpkt		* read the message 
* recvpkt sets fgoodcts(A2)=ff if a valid cts is 
* received; and fadrinuse if a valid packet 
* was received and fadrvalid = 00 
	rts			* no rca . . . (failure) 
		
		

		
*___________________________________________________________________________ 
* 
* routine sendpacket, sendxxx subroutines 
* 
* arguments A0 (input) -- scc control register 
* A2 (input) -- our variables 
* 
* A0-A2 are preserved, A3/D0-D3 are trashed. 
* 
* sendxxx 
* 
* D0 (input) -- lap data packet code (cts, rts, enq, ack) 
* D2 (input) -- lap dstaddr 
* 
* 
* sendpacket 
* uses local vars as parameters
* 
* function: routine to send a packet on the bus. no carrier sense is 
*	    done at this level. there is no status returned (the caller 
* 	    should assume that the packet was sent correctly). 
* 
*___________________________________________________________________________ 
		
shdata				* config scc to send header 
	DC.B	5,enbtxrts 	* (/6b) enable transmitter, output buffer 
	DC.B	3,disrx 	* (/D0) disable receiver 
shlth	EQU	*-shdata 
		
stdata				* reconfig scc after sending trailer 
	DC.B	5,distxrts 	* (/e2) turn off drivers 
	DC.B	14,resetclks 	* (/43) reset missing clocks flag 
	DC.B	3,enbrxslv 	* (/dd) enable receiver 
stlth	EQU	*-stdata 
		
* simple little routine to setup work packet and send it 
		
sendxxx			
	lea	packetout,A2		* lap header 
	move.b	D0,laptypeos(A2)	* fill in dltyp, dldst 
	move.b	D2,dstadros(A2)
	moveq	#3,D2			* flag sendxxx (and set byte count) 
	bra	send_enb
		
sendpacket			
	moveq	#0,D2		* flag sendpacket 
	move.l	packet,A2	* lap header + data (see structure note) ?djw?
		
send_enb			
	lea	shdata,A3
	moveq	#shlth,D3
	bsr	writescc	* disablerx; enabletxdrivers; enabletx; 
		
	moveq	#DL2flags,D0	* { empirically determined delay count } 
send_flg			
	dbra	D0,send_flg	* { output at least 2 flags } 
		
	move.l	A2,A3
	move.b	#restxcrc,(A0)	* (/80) reset xmit crc 
	move.w	D2,D3		* lap only packet? 
	bne	send_1st	* skip if so 
		
	move.l	tlen,D3		* D3 = length of first part to write 
	move.l	packet,A3	* A3 = addr of first part to write 
		
send_1st			
	move.b	(A3)+,sccdata(A0) * send the first byte 
	subq.w	#1,D3		* decrement count (one extra for loop) 
	move.b	syslapaddr,(A3)	* 2nd byte is always our node address 
		
* write out the next part 
	LOCAL		
send_dta			
	moveq	#DLtxunder,D0	* timeout just in case . . . 
_10	btst	#txemptybit,(A0) * transmitter buffer empty? 
	dbne	D0,_10
	bne	_20		* if txemptybit set 
	statcount xundcount
	bra	send_to
		
_20	move.b	(A3)+,sccdata(A0) * move the next byte of this part 
	subq.w	#1,D3
	bgt	send_dta	* do all the bytes in this part 
		
* send crc and finish up 
	LOCAL
send_crc			
	move.b	#resetund,(A0)	* (/c0) reset underrun flag 
	sccDL
		
_10	btst	#undrunbit,(A0)	* wait for eom (underrun) 
	beq	_10
		
_20	btst	#txemptybit,(A0) * wait for crc to be sent 
	beq	_20
		
	move.b	#5,(A0)		* disable xmitter, leave on drivers 
	sccDL
	move.l	#DLabort,D0	* delay count 
	move.b	#enbrts,(A0)	* (/62) actually do the disable 
		
send_abort			
	dbra	D0,send_abort	* delay for trailer (min 12, max 18 abort bits) 
		
send_to			
	move.b	#addrreg,(A0)	* config our port address here 
	lea	stdata,A3	* turn off drivers, 
	moveq	#stlth,D3	* re-enable receiver 
	move.b	syslapaddr,(A0)	* just for dynamic capability 
	move.l	abusvars,A2	* restore A2, fall thru to writescc 
		
* writescc - write out data (as indicated by A3 and D3) to the scc 
		
writescc
	move.b	(A3)+,(A0)
	sccDL
	subq.w	#1,D3
	bne	writescc
	rts	
		
		
*___________________________________________________________________________ 
* 
* routine rinthnd, recvpkt 
* 
* arguments A0 (input) -- scc control address 
* 
* function this routine doubles as the receive interrupt handler and 
* as the routine used to read in cts and ack responses to rts 
* and cts packets. this routine is written to handle the arrival 
* of any type packet regardless of whether it was called by the 
* interrupt handler or internally by getxxx. this routine 
* executes differently based upon the laptype field of the 
* incoming packet, whether this node has aquired a lap node 
* address yet (fadrvalid=0 if not), and whether any errors are 
* detected in the incoming packet. 
* 
* if fadrvalid=0 and laptype<>ff 
* then {set fadrinuse flag and exit} 
* 
* else case laptype of 
* 
* lapdataframe 
* {dispatch to appropriate protocol handler which 
* should read in the rest of the packet thru calls 
* to readpacket and readrest. note that no protocol 
* handlers would be installed if fadrvalid=0, 
* so check is made only if no handler is found} 
* 
* lapenqframe 
* {send ack to defend this node's address} 
* 
* laprtsframe 
* {if dstaddr<>ff send cts to srcaddr; wait for 
* receive char available, then loop back to receive 
* the frame - which should usually be a lapdataframe}
* 
* lapackframe 
* {just exit - initial check will set fadrinuse if 
* appropriate} 
* 
* lapctsframe 
* {set fgoodcts and exit} 
* 
* end; {case} 
* 
*___________________________________________________________________________ 
	LOCAL		
_40	move.l	#-13,D0
	bra	skipD0

recvpkt	lea	torha,A3	* keep a copy in memory (in rha) 
	move.b	sccdata(A0),(A3)+ * byte 1 of packet - dstaddr 
	move.l	#DL2bytes,D1	* timeout (for 2 bytes) 
_10	btst	#rcabit,(A0)	* wait for next byte 
	dbne	D1,_10		* (or timeout) 
	beq	pktinto		* br if timeout 
	move.b	sccdata(A0),(A3)+ * byte 2 of packet - srcaddr 

	move.b	#1,(A0)
	sccDL
	moveq	#checkbits-256,D0	* delay, setup error bits
	and.b	(A0),D0			* see if end-of-frame or overrun 
	bne	_40			* branch if so 

_20	btst	#rcabit,(A0)	* wait for next byte 
	dbne	D1,_20		* (or timeout) 
	beq	pktinto		* br if timeout 
	move.b	sccdata(A0),D0
	move.b	D0,(A3)+	* byte 3 of packet - laptype 
	bmi	lapin		* neg codes are for lap control packets 
		
* it's a data packet and we don't have much time 

	lea	torha,A2	* re-store dst/src/type
  	move.l	packet,A3	* get packet pointer
	move.b	(A2),(A3)+	* copy dst/   /
	move.b	1(A2),(A3)+	* copy    /src/
	move.b	2(A2),(A3)+	* copy    /   /type
	move.l	#608,D3		* 3 + 600 + 2 + slop as max length
	bsr	readrest	* read rest of packet 
	or.w	D3,D0		* D0 = error | overrun status 
	move.l	A3,D1		* final buf address 
	move.l	packet,A3
	sub.l	A3,D1
	subq.l	#1,D1		* don't include crc byte 
	tst.w	D0
	bge	_30		* if no errors or overruns 
	moveq	#-8,D1		* set count negative 
_30	move.l	D1,D0
	rts	

		
****
* 
* we come here if it's a lap control
* packet (rts, enq, ack, cts)
*
****		
	LOCAL

lapin	moveq	#127,D2
	and.b	D0,D2		* strip off sign bit and save in D2 
	cmp.b	#(lapcts-$80),D2 * make sure it's a valid lap type 
	bgt	badpktin	* skip it if not 
		
	bsr	readcrc		* get the crc 
	bne	rintexit	* br if overrun, bad crc to an return
		
	cmp.b	#$ff,torha+dstadros * check if it's a broadcast 
	seq	D1		* save info for later 
	beq	_10		* and br if so 
		
	tst.b	fadrvalid(A2)	* have we configured our address yet? 
	bne	_10		* br if so 
	st	fadrinuse(A2)	* otherwise, some other node must have it 
	bra	_20		* (we got a valid, non-broadcast lap packet) 
		
_10
	subq.b	#1,D2		* was it an enq/rts? 
	beq	enqin		* br if enq 
	subq.b	#3,D2
	beq	rtsin		* br if rts 
	subq.b	#1,D2
	seq	fgoodcts(A2)	* set flag if good cts received (shouldn't
*				  have to worry about getting a broadcast)
_20	rts			* rts to getxxx (probably) 
		
badpktin			
	statcount badcount	* count bad packets 
	moveq	#-11,D0		*djw	
	bra	skipD0		*djw	changed from bra skippktin

	LOCAL		
pktinto			
	statcount rundcount	* count underruns 
	moveq	#underrunerr,D0	*djw	
	bra	skipD0		*djw	
* special condition interrupt handler, too. was also labeled scinthnd
skippktin			
	moveq	#-9,D0		* note error in case called from getxxx 
skipD0	bsr	resetrcvr	* reset receiver for next packet (twice) 
*djw^^
resetrcvr			
	move.b	#reseterr,(A0)	* ($30) reset errors 
	sccDL
	move.b	#reenbint,(A0)	* ($20) re-enable 1st character interrupts 
	sccDL
	move.b	#3,(A0)
	sccDL
	move.b	#disrx,(A0)	* disable/enable rx (reenters hunt) 
	sccDL
	move.b	#3,(A0)
	sccDL
	move.b	#enbrxslv,(A0)
	tst.w	D0		* set ccr to D0 
rintexit			
	rts	
		
rtsin	tst.b	D1		* check if it's a broadcast 
	bne	_10		* if so, don't send cts 
		
	moveq	#qlapcts,D0	* set code to send cts 
	bsr	tosendxxx	* share some code with enqin 
		
* ok, we might as well wait for the data . . . 
		
_10	move.l	#DL400usec,D0	* better get here in 400 usec 
_20	btst	#rcabit,(A0)	* wait for first byte of message . . . 
	dbne	D0,_20		* 
	bne	recvpkt		* br if rca . . . 
	statcount nodtacount	* count no data after cts-rts 
	bra	rintexit	* otherwise, give up . . . 
		
enqin	tst.b	D1		* broadcast enq? (shouldn't really get) 
	bne	rintexit	* just exit if so (really a bad packet) 
	moveq	#qlapack,D0	* set code to send ack 
		
tosendxxx			
	move.b	torha+srcadros,D2
	bsr	sendxxx		* now send out an ack 
	rts			* and exit 
		
*___________________________________________________________________________ 
* 
* 
* readrest - read in the rest of the packet, putting the specified number 
* of bytes into the specified buffer, and ignoring the rest. 
* 
* call 
* A0 = control address 
* A2 -> local variables 
* A3 -> buffer to read into 
* A4 -> start of readpacket 
* D3 = byte count to read (word) 
* 
* return 
* D0 = error byte (z bit set in ccr) 
* D1 modified 
* D2 saved (until packet's all in or error) 
* D3 = 0 if exact number of bytes requested were read 
* > 0 indicates number of bytes requested but not read 
* (packet smaller than requested maximum) 
* < 0 indicates number of extra bytes read but not returned 
* (packet larger than requested maximum) 
* A3 -> one past where last character went 
* A4,A5 saved (until packet's all in or error) 
* 
* note crc bytes not included in count (in buffer, however) 
*___________________________________________________________________________ 
		
	LOCAL		
readcrc			
	moveq	#0,D3		* read nothing (no buffer) 
	bra	rdpkcom
		
readrest			
	bset	#31,D3		* indicate not from readcrc 
rdpkcom			
	moveq	#DL1byte,D1	* char timeout 
_10	btst	#rcabit,(A0)	* next byte in? 
	bne	_20		* br to handle the character 
	dbra	D1,_10		* else, wait for rca 
	statcount rundcount	* count number of underruns 
	moveq	#underrunerr,D0
	bra	rdpktexit
		
* character is in fifo, check if its end, or what 
		
_20	move.b	#1,(A0)	* set to read rr1 
	sccDL
	moveq	#checkbits-256,D0	* delay, setup error bits 
	and.b	(A0),D0			* see if end-of-frame or overrun 
	bne	_30			* branch if so 
	move.b	sccdata(A0),D0		* grab the data 
	subq.w	#1,D3			* any more bytes being accepted? 
* note that the above sub was for a word, don't see bit 31
	bmi	rdpkcom			* br if not 
	move.b	D0,(A3)+		* store it away 
	bra	rdpkcom			* and keep going 
_30			
	move.b	#1,(A0)			* re-read the error register 
	sccDL
	move.b	(A0),D1			* D1 = error byte 
	bmi	endframe		* branch if end of frame 
	statcount ovrcount		* must be overrun 
	moveq	#overrunerr,D0
rdpktexit			
*x	tst.l	D3			* did we come from readcrc? 
*x	bpl	rD5			* branch if so 
rdrestore			
*x	tst.w	D0		* errors? 
*x	bne	rD5		* br if so 
*x	statcount rcvcount	* count number of data packets received 
*x*** took out because am not keeping statistics
		
rD5	bra	resetrcvr	* reset receiver and return (sets z bit) 
		
* we got the end of frame - D1 contains error bits 
	LOCAL		
endframe			
	addq.w	#1,D3		* adjust count for crc byte 
	moveq	#0,D0		* assume success 
	and.b	#ovrorcrc,D1	* check for overrun or crc 
	beq	_10		* branch if not 
	statcount crccount	* count number of crc errors 
	moveq	#crcerr,D0	* note the error otherwise 
_10	bra	rdpktexit	* that's it 
		

****
*
* int ab_recv(struct abdevice *, struct abdevdep, struct packet *)
*  returns negative errcode or packet length if it got the packet correctly.
*  ?djw? note that a packet with a length < 3 is in error,
*
*   plus one that is > 603 is bad too.
*
****
	LOCAL
	XDEF	ab_recv
ab_recv	move.l	12(SP),packet
	move.l	8(SP),abusvars
	move.l	4(SP),sccrd
	addq.l	#2,sccrd
	movem.l	D2/D3/A2/A3,-(SP)
	move.l	abusvars,A2
	move.b	mynaddr(A2),syslapaddr
	move.l	sccrd,A0
	jsr	recvpkt
	movem.l	(SP)+,D2/D3/A2/A3
	RTS


****
*
* cbitcount returns the number of bits set in byte 0 of its parameter
* int cbitcount(u_char)
*****
	LOCAL
	XDEF	cbitcount
cbitcount			
	move.l	D2,save_d2
	move.b	4(SP),D2
	moveq	#0,D0
	moveq	#7,D1		* loop cnt, bit number 
		
_1	btst	D1,D2
	beq	_2
	addq.w	#1,D0
_2	dbra	D1,_1
	move.l	save_d2,D2
	rts	

****
* return the 1's complement checksum of the word aligned buffer at s, for n
*   bytes.
* From: gwasm.s
* dochksum(s,n)
* u_long *s; int n;
****
	LOCAL
	XDEF	dochksum
dochksum
	move.l	8(SP),D1
	move.l	4(SP),A0
	asr.l	#1,D1
	subq.l	#1,D1
	clr.l	D0
_1	add.w	(A0)+,D0
	bcc	_2
	addq.w	#1,D0
_2	dbra	D1,_1
	not.w	D0
	rts


****
* routine to generate a random word
****
	XREF	rand 
randomword			
	movem.l	D1/A0/A1,-(SP)
	move.l	#$f,-(SP)
	jsr	rand
	addq.w	#4,SP	* throw away junk from C calling convention
	movem.l	(SP)+,D1/A0/A1
	rts	

	END


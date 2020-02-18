#define C_LED 0 /* don't compile led code in */

/* constants for bob's 4 digit led display */
#define LEDDATA  0xffff40
#define LEDCTRL	 0xffff44
#define LEDINIT	 0xffff
#define LEDBLANK 0xffff
#define LEDWAIT  20000		/* one second delay for display */

#define ledputw(data) (*(short *)LEDDATA = (data))
#define ledoff	      ledputw(LEDBLANK)

/* led error codes:
	EUNDEF		0		* not defined *
	ENOTFOUND	1		* file not found *
	EACCESS		2		* access violation *
	ENOSPACE	3		* disk full or allocation exceeded *
	EBADOP		4		* illegal TFTP operation *
	EBADID		5		* unknown transfer ID *
	EEXISTS		6		* file already exists *
	ENOUSER		7		* no such user *
*/
#define LED_BADFILETYPE	0x10
#define LED_BADFILENAME	0x11
#define LED_BADLOAD	0x12
#define LED_NOHOST	0x13
#define LED_PANIC	0x14
#define LED_TRAP	0x15




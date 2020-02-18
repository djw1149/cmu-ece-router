@device(imagen300)
@make(text)
@begin(verbatim)
@b(
Led Error Codes:

	EUNDEF		0		* not defined *
	ENOTFOUND	1		* file not found *
	EACCESS		2		* access violation *
	ENOSPACE	3		* disk full or allocation exceeded *
	EBADOP		4		* illegal TFTP operation *
	EBADID		5		* unknown transfer ID *
	EEXISTS		6		* file already exists *
	ENOUSER		7		* no such user *

	LED_BADFILETYPE	0x10
	LED_BADFILENAME	0x11
	LED_BADLOAD	0x12
	LED_NOHOST	0x13
	LED_PANIC	0x14
	LED_TRAP	0x15
)
@end


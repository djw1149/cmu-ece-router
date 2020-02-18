/*
*	makerom.c:  makes a rom file.
*
* History:
*	7-29-86	Harold Weisenbloom(htw) at Carnegie-Mellon started history
*
*/

#define	IBMAT

#include <stdio.h>
#include <io.h>

#include "../../h/rtypes.h"
#include "../mch/mch.h"

/*
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/inter.h"
#include "../mch/autoconf.h" 
*/

#define TARGET			"bootcnf.rom"
#define	GATEWAY_IP		0x8002fe24

#define STRUCT_SIZE(num)					\
			(sizeof (struct autorom) +		\
			(sizeof (struct autoromdev) * num))	\

#define RESERVED	0x000000
#define	MAX		15			/*	max length of an IP */

struct autorom {
    char rom_id[8];
    long rom_num;
    char rom_gway[4];
    long rom_boot;	/* fake pointer (really the offset) to the boot str */
};

struct autoromdev {
    char rom_ipaddr[4];
    long rom_interface;	/* interface identifier, to be mapped into cables */
    long rom_reserved[6];
};

struct  {
	struct autorom header;
	struct autoromdev one;
	struct autoromdev two;
	struct autoromdev three;
	struct autoromdev four;
	struct autoromdev five;
}rom =
{
		"TCP/IP__",
		5,
		0x80,0x02,0xfe,0x24,		/*	pre swabed	*/
		0,

		0,0,0,0,
		1,
		0,0,0,0,0,0,

		0,0,0,0,
		2,
		0,0,0,0,0,0,

		0,0,0,0,
		3,
		0,0,0,0,0,0,

		0,0,0,0,
		4,
		0,0,0,0,0,0,

		0,0,0,0,
		5,
		0,0,0,0,0,0
};


ip_addr(ip_string,ip)
	char	ip_string[MAX],ip[4];
{
	char	*c,*d;

	for(c= ip_string,d = ip,*d = 0; *c; c++)
		if((*c <= '9') && (*c >= '0'))
			*d = (*d) * 10 + (*c - '0');
		else if(*c = '.'){
			d++;
			*d=0;
		}
		else
			printf("error");
}


main()
{
char	ip_string[MAX];
char	ip[4];
long	i,l;
FILE	*fopen(), *fp;

	fp = fopen(TARGET,"w");
	printf("ip address?");
	scanf("%s",ip_string);
	ip_addr(ip_string,ip);
	rom.header.rom_num = htonl(rom.header.rom_num);
	rom.one.rom_interface = htonl(rom.one.rom_interface);
	rom.two.rom_interface = htonl(rom.two.rom_interface);
	rom.three.rom_interface = htonl(rom.three.rom_interface);
	rom.four.rom_interface = htonl(rom.four.rom_interface);
	rom.five.rom_interface = htonl(rom.five.rom_interface);
	for(l = 0;l < 4;l++){
		rom.one.rom_ipaddr[l] = ip[l];
		rom.two.rom_ipaddr[l] = ip[l];
		rom.three.rom_ipaddr[l] = ip[l];
		rom.four.rom_ipaddr[l] = ip[l];
		rom.five.rom_ipaddr[l] = ip[l];
}
		
	fwrite(&rom,STRUCT_SIZE(5), 1,fp);
}

/*
 *  Packet definitions.
 *
 **********************************************************************
 * HISTORY
 * 17-Jul-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added declarations for new PSIZE, POFF, and PLEN variables which
 *	are now calculated during initialization.
 *
 * 26-Jun-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Revised extended memory mapping to use UPAR7; added new mappuser()
 *	macro
 *
 * 18-Feb-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */


/* #include <../h/types.h> */
/* #include <../h/queue.h> */

#include "debug/packet.h"


/*
 *  Packet structure definition
 *
 *  The overhead information associated with each packet is stored
 *  in its associated packet structure.  The actual data area of the packet (as
 *  received from and supplied to the I/O devices) is stored in extended
 *  memory.  Fields in the packet structure indicate the physical location of
 *  the packet buffer in extended memory and retrival information needed to
 *  access it.  Additonal fields are maintained when consistency checks have
 *  been enabled to attempt to catch cases of clobbered data structures,
 *  multiply freed packets, etc.
 *  
 *  The data portion of each packet is defined by two fields.  An offset from
 *  the beginning of the data portion of the packet at which the current
 *  contents begins, and a length indicating the number of bytes it contains.
 *  Encapsulating/decapsulation of protocol headers is accomplished simply by
 *  adjusting these fields appropriately.  To encapsulate the contents in a new
 *  protocol header, the offset is decreased and the length increased by the
 *  header size and the reverse procedure is used to decapsulate the protocol
 *  data from a protocol header.
 */

struct packet
{
    struct qentry p_link;	/* pointer to next packet in queue */
    int		  p_off;	/* offset to data in this buffer */
    int		  p_len;	/* number of data bytes in this buffer */
    p_char	  p_ba;		/* bus address for DMA operations */
    p_char        p_data;	/* data address for access when PAR set */
    int		  p_map;	/* page address register for access */
    pf_int	  p_done;	/* packet processing complete routine */
    char	  p_flag;	/* flags () */
    p_device      p_dv;		/* associated device */
#ifdef	PCHECK
    int		  p_trace;	/* trace value (PC of routine which last */
				/*  checked the packet consistency) */
    int		  p_maxlen;	/* maximum packet length */
    p_packet      p_check;	/* consistency check word - points to itself */
				/*  when the packet is allocated, 0 when */
				/*  free */
#endif	PCHECK
};

/*
 *  Packet flags
 */
#define	P_BCAST		01	/* packet was received as a broadcast */
#define	P_TRACE		02	/* packet is being traced */
#define	P_URGENT	04	/* packet should be given high priority */
				/*  handling by device output routine */
				/*  (e.g. added to the head rather than */
				/*  the tail of the device output queue) */
#define	P_10		010	/* (unused) */
#define	P_MINE		020	/* packet was originated by us */
#define	P_DEVICE	040	/* device dependent flag */


#define	MAXQ	100
#define	MINFREE	3

#ifndef	VAXCOM
/*
 *  The remaining structures and definitions are needed only for internal
 *  purposes and aren't exported for inclusion in maintenance programs.
 */



extern struct queue pfreeq;	/* the free packet queue */


/*
 *  Macro to test if a particular queue is "full"
 *  
 *  For the purposes of this macro, a queue is full it it contains at least as
 *  many packets as are currently free.  This test is generally made by device
 *  drivers before adding packets to a queue and prevents more than half the
 *  available packets from accumulating on these queues.
 */

#define	pfull(q)				\
(						\
    (lengthqueue(q) > MAXQ) ||			\
    ((lengthqueue(&pfreeq) < MINFREE) &&	\
    (lengthqueue(q) >= lengthqueue(&pfreeq)))	\
)

/* pcheck must have a value (0) or we */
/* will upset the compiler since there will */
/* an empty comma operator in the poff macro */

#ifndef	PCHECK
#define	pcheck(p) 0	/* null checks if not enabled */
#endif	PCHECK


/*
 *  Macro which creates a pointer (of the specified cast type) to the current
 *  packet data as defined by its offset and length fields
 *  
 *  This macro also checks the packet for consistency.  Since most paths
 *  through the device driver and protocol hierarchy use this macro to generate
 *  the appropriate pointer into the packet buffer data area at each level,
 *  inconsistencies will usually be detected close to the routine which
 *  is is incorrect.
 */

#define	poff(cast, p)							\
(									\
    pcheck(p),								\
    ((struct cast *)&(p)->p_data[(p)->p_off])				\
)


/*
 *  Macro to make the packet data area accessible by setting the appropriate
 *  extended memory relocation value (the old value is returned so that it may
 *  be squirreled away if necessary)
 */

#define	mapp(p)		\
    remap((p)->p_map)


/*
 *  Macro to make the packet data area accessible by setting the appropriate
 *  extended memory relocation value and chaning to user mode.  N.B.  This
 *  changes stack pointers out from under the C compiler and must be used
 *  carefully.
 */

#define	mappuser(p)	\
    mapuser((p)->p_map)


/*
 *  Macro to adjust the packet offset and length fields to
 *  encapulate/decapsulate protocol headers
 *  
 *  A positive byte count adjusts the fields to include space for a new header
 *  of the desired size preceding the current data.  A negative byte count
 *  adjusts the fields to remove the space of a no longer needed header of the
 *  specified size so that the packet now "points" to its encapsulated data.
 *  The packet consistency is checked before this operation (from paranoia) and
 *  afterward in order to catch any attempts to add space below the physical
 *  beginning of the packet data area or past its end.
 */

#define	padjust(p, bytes)	\
{				\
    pcheck(p);			\
    (p)->p_off -= (bytes);	\
    (p)->p_len += (bytes);	\
    pcheck(p);			\
}



extern struct packet *palloc();		/* allocate a new packet */
extern pfree();				/* free an allocated packet */

extern int PSIZE;			/* maximum packet size */
extern int POFF;			/* default packet offset */
extern int PLEN;			/* default packet length (PSIZE-PLEN) */
extern int MAP;				/* current memory relocation value */

#endif	VAXCOM


struct icmp {				/* icmp header */
	u_char ic_type;				/* icmp message type */
	u_char ic_code;				/* icmp message sub-type */
	u_short ic_sum;	 			/* checksum */
	u_short ic_id;
	u_short ic_seq;
};

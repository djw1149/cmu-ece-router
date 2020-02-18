struct s_ram_element {
    int lower_guard;	/* Should be zero. */
    int success;	/* Successful attempts to load boot program. */
    int fail;		/* Failed attempts to load boot program. */
    int total;		/* Sum of successes and falures. */
    unsigned int cksm;	/* Twos comp of sum of other elements in struct.  */
    int upper_guard;	/* Should be zero. */
};

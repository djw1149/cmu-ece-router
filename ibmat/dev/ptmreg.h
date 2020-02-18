#define PTM_SCALE	10000	/* 1MHZ/100HZ */

struct ptmdevice {
#define ptm_csr3 ptm_csr1
    u_char ptm_csr1;
    u_char skip1;
    u_char ptm_csr2;
    u_char skip2;
    u_char ptm_msb1;
    u_char skip3;
    u_char ptm_cnt1;
    u_char skip4;
    u_char ptm_msb2;
    u_char skip5;
    u_char ptm_cnt2;
    u_char skip6;
    u_char ptm_msb3;
    u_char skip7;
    u_char ptm_cnt3;
    u_char skip8;
};

/* counter modes */
#define PTM_RESET	0x00
#define PTM_NORESET	0x10	/* write to latches does not reset counter */
#define PTM_CONT 	0x00	/* continuous mode */
#define PTM_SINGLE	0x08	/* single shot mode */

/* waveform modes */
#define PTM_LESS	0x00
#define PTM_GREATER	0x08	/* interrupt if pulse/freq  > counter value */
#define PTM_FREQ	0x20	/* freq compare mode */
#define PTM_PULSE	0x30	/* pulse compare mode */

#define PTM_DISABLE	0x1	/* bit 0 reg 1 */
#define PTM_REG3	0x0	/* bit 0 reg 2 */
#define PTM_REG1	0x1
#define PTM_PRESCALE	0x1	/* bit 0 reg 3 */

#define PTM_INTERN	0x02	/* use internal E clock */
#define PTM_8BIT	0x04	/* dual 8 bit counter mode */
#define PTM_IRQENA	0x40
#define PTM_OUTENA	0x80

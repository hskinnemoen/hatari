/*
 * dsp.h - Atari DSP56001 emulation code - declaration
 *
 * Copyright (c) 2001-2004 Petr Stehlik of ARAnyM dev team
 * Adaption to Hatari (C) 2006 by Thomas Huth
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ARAnyM; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _DSP_H
#define _DSP_H

#include <SDL.h>
#include "araglue.h"

#define DSP_RAMSIZE 32768

/* Dsp State */
#define DSP_BOOTING			0	/* Dsp loads bootstrap code */
#define DSP_RUNNING			1	/* Execute instructions */
#define DSP_WAITHOSTWRITE	2	/* Dsp waits for host to write data */
#define DSP_WAITHOSTREAD	3	/* Dsp waits for host to read data */
#define DSP_HALT			4	/* Dsp is halted */
#define DSP_STOPTHREAD		5	/* Stop emulation thread */
#define DSP_STOPPEDTHREAD	6	/* Dsp thread stopped and finished */

/* Host port, CPU side */
#define CPU_HOST_ICR	0x00
#define CPU_HOST_CVR	0x01
#define CPU_HOST_ISR	0x02
#define CPU_HOST_IVR	0x03
#define CPU_HOST_RX0	0x04
#define CPU_HOST_RXH	0x05
#define CPU_HOST_RXM	0x06
#define CPU_HOST_RXL	0x07
#define CPU_HOST_TX0	0x04
#define CPU_HOST_TXH	0x05
#define CPU_HOST_TXM	0x06
#define CPU_HOST_TXL	0x07

#define CPU_HOST_ICR_RREQ	0x00
#define CPU_HOST_ICR_TREQ	0x01
#define CPU_HOST_ICR_HF0	0x03
#define CPU_HOST_ICR_HF1	0x04
#define CPU_HOST_ICR_HM0	0x05
#define CPU_HOST_ICR_HM1	0x06
#define CPU_HOST_ICR_INIT	0x07

#define CPU_HOST_CVR_HC		0x07

#define CPU_HOST_ISR_RXDF	0x00
#define CPU_HOST_ISR_TXDE	0x01
#define CPU_HOST_ISR_TRDY	0x02
#define CPU_HOST_ISR_HF2	0x03
#define CPU_HOST_ISR_HF3	0x04
#define CPU_HOST_ISR_DMA	0x06
#define CPU_HOST_ISR_HREQ	0x07

/* Host port, DSP side, DSP addresses are 0xffc0+value */
#define DSP_PBC			0x20	/* Port B control register */
#define DSP_PCC			0x21	/* Port C control register */
#define DSP_PBDDR		0x22	/* Port B data direction register */
#define DSP_PCDDR		0x23	/* Port C data direction register */
#define DSP_PBD			0x24	/* Port B data register */
#define DSP_PCD			0x25	/* Port C data register */
#define DSP_HOST_HCR	0x28	/* Host control register */
#define DSP_HOST_HSR	0x29	/* Host status register */
#define DSP_HOST_HRX	0x2b	/* Host receive register */
#define DSP_HOST_HTX	0x2b	/* Host transmit register */
#define DSP_SSI_CRA		0x2c	/* Ssi control register A */
#define DSP_SSI_CRB		0x2d	/* Ssi control register B */
#define DSP_SSI_SR		0x2e	/* Ssi status register */
#define DSP_SSI_TSR		0x2e	/* Ssi time slot register */
#define DSP_SSI_RX		0x2f	/* Ssi receive register */
#define DSP_SSI_TX		0x2f	/* Ssi transmit register */
#define DSP_BCR			0x3e	/* Port A bus control register */
#define DSP_IPR			0x3f	/* Interrupt priority register */

#define DSP_HOST_HCR_HRIE	0x00
#define DSP_HOST_HCR_HTIE	0x01
#define DSP_HOST_HCR_HCIE	0x02
#define DSP_HOST_HCR_HF2	0x03
#define DSP_HOST_HCR_HF3	0x04

#define DSP_HOST_HSR_HRDF	0x00
#define DSP_HOST_HSR_HTDE	0x01
#define DSP_HOST_HSR_HCP	0x02
#define DSP_HOST_HSR_HF0	0x03
#define DSP_HOST_HSR_HF1	0x04
#define DSP_HOST_HSR_DMA	0x07


struct DSP
{
	/* DSP state */
	uint8	state;

	/* Registers */
	uint16	pc;
	uint32	registers[64];

	/* stack[0=ssh], stack[1=ssl] */
	uint16	stack[2][15];

	/* External ram[0] is x:, ram[1] is y:, ram[2] is p: */
	uint32	ram[3][DSP_RAMSIZE];

	/* rom[0] is x:, rom[1] is y: */
	uint32	rom[2][512];

	/* Internal ram[0] is x:, ram[1] is y:, ram[2] is p: */
	uint32	ramint[3][512];

	/* peripheral space, [x|y]:0xffc0-0xffff */
	uint32	periph[2][64];

	/* host port, CPU side */
	uint8 hostport[8];

	/* Misc */
	uint32 loop_rep;		/* executing rep ? */
	uint32 last_loop_inst;	/* executing the last instruction in DO ? */
	uint32 first_host_write;	/* first byte written to host port */

	SDL_sem		*dsp56k_sem;
};

extern struct DSP dsp;

static inline struct DSP* getDSP(void)
{
	return &dsp;
}


void DSP_HandleReadAccess(void);
void DSP_HandleWriteAccess(void);

extern void DSP_Init(void);
extern void DSP_UnInit(void);

/* Setup functions */
extern void DSP_Reset(void);
extern void DSP_shutdown(void);

/* Host port transfer */
extern void DSP_host2dsp(void);
extern void DSP_dsp2host(void);


#endif /* _DSP_H */

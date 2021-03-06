/*
	DSP M56001 emulation
	Core of DSP emulation

	(C) 2003-2008 ARAnyM developer team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef DSP_CORE_H
#define DSP_CORE_H

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_RAMSIZE 32768

/* Host port, CPU side */
#define CPU_HOST_ICR	0x00
#define CPU_HOST_CVR	0x01
#define CPU_HOST_ISR	0x02
#define CPU_HOST_IVR	0x03
#define CPU_HOST_TRX0	0x04
#define CPU_HOST_TRXH	0x05
#define CPU_HOST_TRXM	0x06
#define CPU_HOST_TRXL	0x07
#define CPU_HOST_RX0	0x04
#define CPU_HOST_RXH	0x05
#define CPU_HOST_RXM	0x06
#define CPU_HOST_RXL	0x07
#define CPU_HOST_TXH	0x09
#define CPU_HOST_TXM	0x0a
#define CPU_HOST_TXL	0x0b

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
#define DSP_HOST_HCR		0x28	/* Host control register */
#define DSP_HOST_HSR		0x29	/* Host status register */
#define DSP_HOST_HRX		0x2b	/* Host receive register */
#define DSP_HOST_HTX		0x2b	/* Host transmit register */
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

#define DSP_SSI_CRA_DC0		0x8
#define DSP_SSI_CRA_DC1		0x9
#define DSP_SSI_CRA_DC2		0xa
#define DSP_SSI_CRA_DC3		0xb
#define DSP_SSI_CRA_DC4		0xc
#define DSP_SSI_CRA_WL0		0xd
#define DSP_SSI_CRA_WL1		0xe

#define DSP_SSI_CRB_OF0		0x0
#define DSP_SSI_CRB_OF1		0x1
#define DSP_SSI_CRB_SCD0	0x2
#define DSP_SSI_CRB_SCD1	0x3
#define DSP_SSI_CRB_SCD2	0x4
#define DSP_SSI_CRB_SCKD	0x5
#define DSP_SSI_CRB_SHFD	0x6
#define DSP_SSI_CRB_FSL0	0x7
#define DSP_SSI_CRB_FSL1	0x8
#define DSP_SSI_CRB_SYN		0x9
#define DSP_SSI_CRB_GCK		0xa
#define DSP_SSI_CRB_MOD		0xb
#define DSP_SSI_CRB_TE		0xc
#define DSP_SSI_CRB_RE		0xd
#define DSP_SSI_CRB_TIE		0xe
#define DSP_SSI_CRB_RIE		0xf

#define DSP_SSI_SR_IF0		0x0
#define DSP_SSI_SR_IF1		0x1
#define DSP_SSI_SR_TFS		0x2
#define DSP_SSI_SR_RFS		0x3
#define DSP_SSI_SR_TUE		0x4
#define DSP_SSI_SR_ROE		0x5
#define DSP_SSI_SR_TDE		0x6
#define DSP_SSI_SR_RDF		0x7

#define DSP_INTERRUPT_NONE      0x0
#define DSP_INTERRUPT_FAST      0x1
#define DSP_INTERRUPT_LONG      0x2

#define DSP_INTER_RESET			0x0
#define DSP_INTER_ILLEGAL		0x1
#define DSP_INTER_STACK_ERROR		0x2
#define DSP_INTER_TRACE			0x3
#define DSP_INTER_SWI			0x4
#define DSP_INTER_HOST_COMMAND		0x5
#define DSP_INTER_HOST_RCV_DATA		0x6
#define DSP_INTER_HOST_TRX_DATA		0x7
#define DSP_INTER_SSI_RCV_DATA_E	0x9
#define DSP_INTER_SSI_RCV_DATA		0xa
#define DSP_INTER_SSI_TRX_DATA_E	0xb
#define DSP_INTER_SSI_TRX_DATA		0xc


typedef struct dsp_core_ssi_s dsp_core_ssi_t;

struct dsp_core_ssi_s {
	Uint16  cra_word_length;
	Uint32  cra_word_mask;
	Uint16  cra_frame_rate_divider;

	Uint16  crb_src_clock;
	Uint16  crb_shifter;
	Uint16  crb_synchro;
	Uint16  crb_mode;
	Uint16  crb_te;
	Uint16  crb_re;
	Uint16  crb_tie;
	Uint16  crb_rie;

	Uint32  TX;
	Uint32  RX;
	Uint32  transmit_value;		/* DSP Transmit --> SSI */
	Uint32  received_value;		/* DSP Receive  --> SSI */
	Uint16  waitFrame;
	Uint16  slot_in_frame;
};


typedef struct dsp_core_s dsp_core_t;

struct dsp_core_s {

	/* DSP executing instructions ? */
	volatile int running;

	/* Registers */
	Uint16	pc;
	Uint32	registers[64];

	/* stack[0=ssh], stack[1=ssl] */
	Uint16	stack[2][16];

	/* External ram[0] is x:, ram[1] is y:, ram[2] is p: */
	Uint32	ram[3][DSP_RAMSIZE];

	/* rom[0] is x:, rom[1] is y: */
	Uint32	rom[2][512];

	/* Internal ram[0] is x:, ram[1] is y:, ram[2] is p: */
	Uint32	ramint[3][512];

	/* peripheral space, [x|y]:0xffc0-0xffff */
	volatile Uint32	periph[2][64];
	volatile Uint32	dsp_host_htx;
	volatile Uint32	dsp_host_rtx;


	/* host port, CPU side */
	volatile Uint8 hostport[12];

	/* SSI */
	dsp_core_ssi_t ssi;

	/* Misc */
	Uint32 loop_rep;		/* executing rep ? */
	Uint32 pc_on_rep;		/* True if PC is on REP instruction */

	/* For bootstrap routine */
	Uint16	bootstrap_pos;

	/* Interruptions */
	Uint16	interrupt_state;
	Uint32  interrupt_instr_fetch;
	Uint32  interrupt_save_pc;
	Uint16  interrupt_table[13];
	Uint16  interrupt_counter;
};


/* Emulator call these to init/stop/reset DSP emulation */
void dsp_core_init(dsp_core_t *dsp_core);
void dsp_core_shutdown(dsp_core_t *dsp_core);
void dsp_core_reset(dsp_core_t *dsp_core);

/* Post a new interrupt to the interrupt table */
void dsp_core_add_interrupt(dsp_core_t *dsp_core, Uint32 inter);

/* host port read/write by emulator, addr is 0-7, not 0xffa200-0xffa207 */
Uint8 dsp_core_read_host(dsp_core_t *dsp_core, int addr);
void dsp_core_write_host(dsp_core_t *dsp_core, int addr, Uint8 value);

/* dsp_cpu call these to read/write host port */
void dsp_core_hostport_dspread(dsp_core_t *dsp_core);
void dsp_core_hostport_dspwrite(dsp_core_t *dsp_core);

/* dsp_cpu call these to read/write/configure SSI port */
void dsp_core_ssi_configure(dsp_core_t *dsp_core, Uint32 adress, Uint32 value);
void dsp_core_ssi_receive_serial_clock(dsp_core_t *dsp_core);
void dsp_core_ssi_receive_SC2(dsp_core_t *dsp_core, Uint32 value);
void dsp_core_ssi_writeTX(dsp_core_t *dsp_core, Uint32 value);
void dsp_core_ssi_writeTSR(dsp_core_t *dsp_core);
Uint32 dsp_core_ssi_readRX(dsp_core_t *dsp_core);
void dsp_core_ssi_generate_internal_clock(dsp_core_t *dsp_core);



/* Process peripheral code */
void dsp_core_process_host_interface(dsp_core_t *dsp_core);	/* HI */
void dsp_core_process_ssi_interface(dsp_core_t *dsp_core);	/* SSI */

#ifdef __cplusplus
}
#endif

#endif /* DSP_CORE_H */

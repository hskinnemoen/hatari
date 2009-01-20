/*
  Hatari - midi.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  MIDI communication.
  Note that this code is far from being perfect. However, it is already
  enough to let some ST programs (e.g. the game Pirates!) use the host's midi
  system.

  TODO:
   - Most bits in the ACIA's status + control registers are currently ignored.
   - Check when we have to clear the ACIA_SR_INTERRUPT_REQUEST bit in the
     ACIA status register (it is currently done when reading or writing to
     the data register, but probably it should rather be done when reading the
     status register?).
*/
const char Midi_fileid[] = "Hatari midi.c : " __DATE__ " " __TIME__;

#include <SDL_types.h>

#include "main.h"
#include "configuration.h"
#include "ioMem.h"
#include "m68000.h"
#include "mfp.h"
#include "midi.h"
#include "file.h"


#define ACIA_SR_INTERRUPT_REQUEST  0x80
#define ACIA_SR_TX_EMPTY           0x02
#define ACIA_SR_RX_FULL            0x01

#define MIDI_DEBUG 0
#if MIDI_DEBUG
#define Dprintf(a) printf a
#else
#define Dprintf(a)
#endif


static FILE *pMidiFhIn  = NULL;        /* File handle used for Midi input */
static FILE *pMidiFhOut = NULL;        /* File handle used for Midi output */
static Uint8 MidiControlRegister;
static Uint8 MidiStatusRegister;
static Uint8 nRxDataByte;


/**
 * Initialization: Open MIDI device.
 */
void Midi_Init(void)
{
	if (!ConfigureParams.Midi.bEnableMidi)
		return;

	if (ConfigureParams.Midi.sMidiOutFileName[0])
	{
		/* Open MIDI output file */
		pMidiFhOut = File_Open(ConfigureParams.Midi.sMidiOutFileName, "wb");
		if (!pMidiFhOut)
		{
			Log_AlertDlg(LOG_ERROR, "MIDI output file open failed. MIDI support disabled.");
			ConfigureParams.Midi.bEnableMidi = FALSE;
			return;
		}
		setvbuf(pMidiFhOut, NULL, _IONBF, 0);    /* No output buffering! */
		Dprintf(("Opened file '%s' for MIDI output.\n",
			 ConfigureParams.Midi.sMidiOutFileName));
	}
	if (ConfigureParams.Midi.sMidiInFileName[0])
	{
		/* Try to open MIDI input file */
		pMidiFhIn = File_Open(ConfigureParams.Midi.sMidiInFileName, "rb");
		if (!pMidiFhIn)
		{
			Log_AlertDlg(LOG_ERROR, "MIDI input file open failed. MIDI support disabled.");
			ConfigureParams.Midi.bEnableMidi = FALSE;
			return;
		}
		setvbuf(pMidiFhIn, NULL, _IONBF, 0);    /* No input buffering! */
		Dprintf(("Opened file '%s' for MIDI input.\n",
			 ConfigureParams.Midi.sMidiInFileName));
	}
}


/**
 * Close MIDI device.
 */
void Midi_UnInit(void)
{
	pMidiFhIn = File_Close(pMidiFhIn);
	pMidiFhOut = File_Close(pMidiFhOut);

	Int_RemovePendingInterrupt(INTERRUPT_MIDI);
}


/**
 * Reset MIDI emulation.
 */
void Midi_Reset(void)
{
	MidiControlRegister = 0;
	MidiStatusRegister = ACIA_SR_TX_EMPTY;
	nRxDataByte = 1;

	if (ConfigureParams.Midi.bEnableMidi)
	{
		Int_AddRelativeInterrupt(2050, INT_CPU_CYCLE, INTERRUPT_MIDI);
	}
}


/**
 * Read MIDI status register ($FFFC04).
 */
void Midi_Control_ReadByte(void)
{
	Dprintf(("Midi_ReadControl : $%x.\n", MidiStatusRegister));

	/* ACIA registers need wait states - but the value seems to vary in certain cases */
	M68000_WaitState(8);

	IoMem[0xfffc04] = MidiStatusRegister;
}


/**
 * Write to MIDI control register ($FFFC04).
 */
void Midi_Control_WriteByte(void)
{
	/* ACIA registers need wait states - but the value seems to vary in certain cases */
	M68000_WaitState(8);

	MidiControlRegister = IoMem[0xfffc04];

	Dprintf(("Midi_WriteControl($%x)\n", MidiControlRegister));

	/* Do we need to generate a transfer interrupt? */
	if ((MidiControlRegister & 0xA0) == 0xA0)
	{
		Dprintf(("WriteControl: Transfer interrupt!\n"));

		/* Acknowledge in MFP circuit, pass bit,enable,pending */
		MFP_InputOnChannel(MFP_ACIA_BIT, MFP_IERB, &MFP_IPRB);

		MidiStatusRegister |= ACIA_SR_INTERRUPT_REQUEST;
	}
}


/**
 * Read MIDI data register ($FFFC06).
 */
void Midi_Data_ReadByte(void)
{
	Dprintf(("Midi_ReadData : $%x.\n", 1));

	/* ACIA registers need wait states (value seems to vary in certain cases) */
	M68000_WaitState(8);

	MidiStatusRegister &= ~(ACIA_SR_INTERRUPT_REQUEST|ACIA_SR_RX_FULL);

	/* GPIP I4 - General Purpose Pin Keyboard/MIDI interrupt,
	 * becomes high(1) again after data has been read. */
	MFP_GPIP |= 0x10;

	IoMem[0xfffc06] = nRxDataByte;
}


/**
 * Write to MIDI data register ($FFFC06).
 */
void Midi_Data_WriteByte(void)
{
	Uint8 nTxDataByte;

	/* ACIA registers need wait states (value seems to vary in certain cases) */
	M68000_WaitState(8);

	nTxDataByte = IoMem[0xfffc06];

	Dprintf(("Midi_WriteData($%x)\n", nTxDataByte));

	MidiStatusRegister &= ~ACIA_SR_INTERRUPT_REQUEST;

	if (!ConfigureParams.Midi.bEnableMidi)
		return;

	if (pMidiFhOut)
	{
		int ret;

		/* Write the character to the output file: */
		ret = fputc(nTxDataByte, pMidiFhOut);

		/* If there was an error then stop the midi emulation */
		if (ret == EOF)
		{
			Midi_UnInit();
			return;
		}
	}

	MidiStatusRegister &= ~ACIA_SR_TX_EMPTY;
}


/**
 * Read and write MIDI interface data regularly
 */
void Midi_InterruptHandler_Update(void)
{
	int nInChar;

	/* Remove this interrupt from list and re-order */
	Int_AcknowledgeInterrupt();

	/* Flush outgoing data */
	if (!(MidiStatusRegister & ACIA_SR_TX_EMPTY))
	{
		/* Do we need to generate a transfer interrupt? */
		if ((MidiControlRegister & 0xA0) == 0xA0)
		{
			Dprintf(("WriteData: Transfer interrupt!\n"));
			/* Acknowledge in MFP circuit, pass bit,enable,pending */
			MFP_InputOnChannel(MFP_ACIA_BIT, MFP_IERB, &MFP_IPRB);
			MidiStatusRegister |= ACIA_SR_INTERRUPT_REQUEST;
		}

		// if (pMidiFhOut)
		//	fflush(pMidiFhOut);

		MidiStatusRegister |= ACIA_SR_TX_EMPTY;
	}

	/* Read the bytes in, if we have any */
	if (pMidiFhIn && File_InputAvailable(pMidiFhIn))
	{
		nInChar = fgetc(pMidiFhIn);
		if (nInChar != EOF)
		{
			Dprintf(("Midi: Read character $%x\n", nInChar));
			/* Copy into our internal queue */
			nRxDataByte = nInChar;
			/* Do we need to generate a receive interrupt? */
			if ((MidiControlRegister & 0x80) == 0x80)
			{
				Dprintf(("WriteData: Receive interrupt!\n"));
				/* Acknowledge in MFP circuit */
				MFP_InputOnChannel(MFP_ACIA_BIT, MFP_IERB, &MFP_IPRB);
				MidiStatusRegister |= ACIA_SR_INTERRUPT_REQUEST;
			}
			MidiStatusRegister |= ACIA_SR_RX_FULL;
			/* GPIP I4 - General Purpose Pin Keyboard/MIDI interrupt:
			 * It will remain low(0) until data is read from $fffc06. */
			MFP_GPIP &= ~0x10;
		}
		else
		{
			Dprintf(("Midi: error during read!\n"));
			clearerr(pMidiFhIn);
		}
	}

	Int_AddRelativeInterrupt(2050, INT_CPU_CYCLE, INTERRUPT_MIDI);
}

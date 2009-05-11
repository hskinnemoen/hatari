/*
  Hatari - xbios.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  XBios Handler (Trap #14)

  We intercept and direct some XBios calls to handle the RS-232 etc. and help
  with floppy debugging.
*/
const char XBios_rcsid[] = "Hatari $Id: xbios.c,v 1.18 2008-11-03 20:24:25 thothy Exp $";

#include "main.h"
#include "configuration.h"
#include "floppy.h"
#include "log.h"
#include "m68000.h"
#include "rs232.h"
#include "screenSnapShot.h"
#include "stMemory.h"
#include "xbios.h"
#include "debugui.h"

#define XBIOS_DEBUG 0	/* for floppy read/write */


/* List of Atari ST RS-232 baud rates */
static const int BaudRates[] =
{
	19200, /* 0 */
	9600,  /* 1 */
	4800,  /* 2 */
	3600,  /* 3 */
	2400,  /* 4 */
	2000,  /* 5 */
	1800,  /* 6 */
	1200,  /* 7 */
	600,   /* 8 */
	300,   /* 9 */
	200,   /* 10 */
	150,   /* 11 */
	134,   /* 12 */
	110,   /* 13 */
	75,    /* 14 */
	50     /* 15 */
};


/*-----------------------------------------------------------------------*/
/**
 * XBIOS Floppy Read
 * Call 8
 */
static bool XBios_Floprd(Uint32 Params)
{
#if XBIOS_DEBUG
	char *pBuffer;
	Uint16 Dev,Sector,Side,Track,Count;

	/* Read details from stack */
	pBuffer = (char *)STRAM_ADDR(STMemory_ReadLong(Params+SIZE_WORD));
	Dev = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG);
	Sector = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG+SIZE_WORD);
	Track = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG+SIZE_WORD+SIZE_WORD);
	Side = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG+SIZE_WORD+SIZE_WORD+SIZE_WORD);
	Count = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG+SIZE_WORD+SIZE_WORD+SIZE_WORD+SIZE_WORD);

	Log_Printf(LOG_DEBUG, "FLOPRD %s,%d,%d,%d,%d at addr 0x%X\n", EmulationDrives[Dev].szFileName,
	           Side, Track, Sector, Count, M68000_GetPC());
#endif

	return FALSE;
}


/*-----------------------------------------------------------------------*/
/**
 * XBIOS Floppy Write
 * Call 9
 */
static bool XBios_Flopwr(Uint32 Params)
{
#if XBIOS_DEBUG
	char *pBuffer;
	Uint16 Dev,Sector,Side,Track,Count;

	/* Read details from stack */
	pBuffer = (char *)STRAM_ADDR(STMemory_ReadLong(Params+SIZE_WORD));
	Dev = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG);
	Sector = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG+SIZE_WORD);
	Track = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG+SIZE_WORD+SIZE_WORD);
	Side = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG+SIZE_WORD+SIZE_WORD+SIZE_WORD);
	Count = STMemory_ReadWord(Params+SIZE_WORD+SIZE_LONG+SIZE_LONG+SIZE_WORD+SIZE_WORD+SIZE_WORD+SIZE_WORD);

	Log_Printf(LOG_DEBUG, "FLOPWR %s,%d,%d,%d,%d at addr 0x%X\n", EmulationDrives[Dev].szFileName,
	           Side, Track, Sector, Count, M68000_GetPC());
#endif

	return FALSE;
}


/*-----------------------------------------------------------------------*/
/**
 * XBIOS RsConf
 * Call 15
 */
static bool XBios_Rsconf(Uint32 Params)
{
	short int Baud,Ctrl,Ucr,Rsr,Tsr,Scr;

	Baud = STMemory_ReadWord(Params+SIZE_WORD);
	Ctrl = STMemory_ReadWord(Params+SIZE_WORD+SIZE_WORD);
	Ucr = STMemory_ReadWord(Params+SIZE_WORD+SIZE_WORD+SIZE_WORD);
	Rsr = STMemory_ReadWord(Params+SIZE_WORD+SIZE_WORD+SIZE_WORD+SIZE_WORD);
	Tsr = STMemory_ReadWord(Params+SIZE_WORD+SIZE_WORD+SIZE_WORD+SIZE_WORD+SIZE_WORD);
	Scr = STMemory_ReadWord(Params+SIZE_WORD+SIZE_WORD+SIZE_WORD+SIZE_WORD+SIZE_WORD+SIZE_WORD);

	/* Set baud rate and other configuration, if RS232 emaulation is enabled */
	if (ConfigureParams.RS232.bEnableRS232)
	{
		if (Baud>=0 && Baud<=15)
		{
			/* Convert ST baud rate index to value */
			int BaudRate = BaudRates[Baud];
			/* And set new baud rate: */
			RS232_SetBaudRate(BaudRate);
		}

		if (Ucr != -1)
		{
			RS232_HandleUCR(Ucr);
		}

		if (Ctrl != -1)
		{
			RS232_SetFlowControl(Ctrl);
		}

		return TRUE;
	}

	return FALSE;
}


/*-----------------------------------------------------------------------*/
/**
 * XBIOS Scrdmp
 * Call 20
 */
static bool XBios_Scrdmp(Uint32 Params)
{
	ScreenSnapShot_SaveScreen();

	/* Correct return code? */
	Regs[REG_D0] = 0;

	return TRUE;
}


/*----------------------------------------------------------------------- */
/**
 * XBIOS Prtblk
 * Call 36
 */
static bool XBios_Prtblk(Uint32 Params)
{
	/* Correct return code? */
	Regs[REG_D0] = 0;

	return TRUE;
}

/*----------------------------------------------------------------------- */
/**
 * XBIOS Change Emulator Options
 * Call 255
 */
static bool XBios_HatariOption(Uint32 Params)
{
	int option = STMemory_ReadWord(Params+2);

	fprintf(stderr, "Intercepted XBIOS HatariOption(), option=%d (%08x), ", option, Params);

	switch(option)
	{
	case 1: /* Min speed? */
		ConfigureParams.System.bFastForward = false;
		//ConfigureParams.System.nMinMaxSpeed = MINMAXSPEED_MIN;
		/* Reset the sound emulation variables: */
		Sound_ResetBufferIndex();
		fprintf(stderr, "Normal speed\n");
		Regs[REG_D0] = 0;
		break;
	case 2: /* Max speed? */
		ConfigureParams.System.bFastForward = true;
		//ConfigureParams.System.nMinMaxSpeed = MINMAXSPEED_MAX;
		fprintf(stderr, "Maximum speed!\n");
		Regs[REG_D0] = 0;
		break;
	case 3:  /* Enable debugger? */
		bEnableDebug = TRUE;
		fprintf(stderr, "Debugger enabled\n");
		Regs[REG_D0] = 0;
		break;
	case 4:  /* Disable debugger? */
		bEnableDebug = FALSE;
		fprintf(stderr, "Debugger disabled\n");
		Regs[REG_D0] = 0;
		break;
	default:
		fprintf(stderr, "Unknown argument (%d)\n",Params);
		Regs[REG_D0] = -1;
	}

	return TRUE;
}

/*----------------------------------------------------------------------- */
/**
 * XBIOS Debug output to console
 * Call 254
 * Parameters: memptr at -2(sp) and numbytes at -6(sp)
 */
static bool XBios_Debug(Uint32 Params)
{
	int i;
	Uint32 memptr = STMemory_ReadLong(Params+2);
	Uint32 length = STMemory_ReadLong(Params+6);

	fprintf(stderr, "Memory dump at 0x%08x (%d bytes): ", memptr, length);
	if (length > 256)
	{
		length = 256;
		fprintf(stderr,"length truncated to 256 bytes! ");
	}
	for (i = 0; i < length; i++)
	{
		if ((i % 16) == 0)
			fprintf(stderr,"\n");
		fprintf(stderr,"%02x ", STMemory_ReadByte(memptr + i));
	}
	fprintf(stderr,"\n");

	return TRUE;
}

/**
 * XBIOS Dump all registers to console
 * Call 251
 * Parameters: none
 */
static bool XBios_Registers(Uint32 Params)
{
//	Uint32 memptr = STMemory_ReadLong(Params+2);
//	Uint32 length = STMemory_ReadLong(Params+6);

	fprintf(stderr, "Registers: \n");
	fprintf(stderr,"d/a0     d/a1     d/a2     d/a3      d/a4     d/a5     d/a6     d/a7 \n");
	fprintf(stderr,"%08x %08x %08x %08x %08x %08x %08x %08x \n",
		Regs[REG_D0], Regs[REG_D1], Regs[REG_D2], Regs[REG_D3],
		Regs[REG_D4], Regs[REG_D5], Regs[REG_D6], Regs[REG_D7] );
	fprintf(stderr,"%08x %08x %08x %08x %08x %08x %08x %08x \n",
		Regs[REG_A0], Regs[REG_A1], Regs[REG_A2], Regs[REG_A3],
		Regs[REG_A4], Regs[REG_A5], Regs[REG_A6], Regs[REG_A7] );

	return TRUE;
}

static Uint32 sCycleCounter[256];
extern int nScanlinesPerFrame;                   /* Number of scan lines per frame */
extern int nCyclesPerLine;                       /* Cycles per horizontal line scan */

/**
 * XBIOS Start or restart a cycle counter
 * Call 253
 * Parameters: counter no at -2(sp)
 */
static bool XBios_CounterStart(Uint32 Params)
{
	Params = STMemory_ReadWord(Params+2);
	Params &= 0xFF;  /* we only use 255 cycle counters */
	sCycleCounter[Params] = Cycles_GetCounter(CYCLES_COUNTER_VIDEO);

	return TRUE;
}

/**
 * XBIOS Stop a cycle counter
 * Call 252
 * Parameters: counter no at -2(sp)
 */
static bool XBios_CounterRead(Uint32 Params)
{
	Uint32 cycles,curr_cycles;

	Params = STMemory_ReadWord(Params+2);
	Params &= 0xFF;  /* we only use 255 cycle counters */

	cycles = Cycles_GetCounter(CYCLES_COUNTER_VIDEO);

	/* has the frame refreshed? */
	if(cycles < sCycleCounter[Params])
	{	/* extend the frame... */
		cycles += nScanlinesPerFrame*nCyclesPerLine;;
	}

	curr_cycles = cycles-sCycleCounter[Params];
	fprintf(stderr,"Cnt #%d at %d : %d cycles (%d frames + %d cycles)\n",
		Params, cycles, curr_cycles, curr_cycles / nCyclesPerLine,
		curr_cycles % nCyclesPerLine);

	return TRUE;
}

/**
 * XBIOS Set Hatari CPU frequency
 * Call 249
 * Parameters: CPU frequency (word)
 */
static bool XBios_CPUFreq(Uint32 Params)
{
	int cpuFreq = STMemory_ReadWord(Params+2);

	cpuFreq &= 0x1FF; /* max freq is 511 MHz */
	nCpuFreqShift = log2(cpuFreq) - 3;
	if(nCpuFreqShift < 0 || nCpuFreqShift > 6)
		nCpuFreqShift = 0;
	ConfigureParams.System.nCpuFreq = 8 << nCpuFreqShift;
	printf("cpu freq: %d , cpu shift: %d \n", cpuFreq ,  nCpuFreqShift);

	return TRUE;
}

/**
 * XBIOS Enter Hatari debug UI
 * Call 251
 * Parameters: none
 */
static bool XBios_DebugUI(Uint32 Params)
{
	DebugUI();
	return TRUE;
}


/*-----------------------------------------------------------------------*/
/**
 * Check if we need to re-direct XBios call to our own routines
 */
bool XBios(void)
{
	Uint32 Params;
	Uint16 XBiosCall;

	/* Find call */
	Params = Regs[REG_A7];
	XBiosCall = STMemory_ReadWord(Params);

	Log_Printf(LOG_DEBUG, "XBIOS %d\n", XBiosCall);
	switch (XBiosCall)
	{
	 case 8:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS Floprd()\n" );
		return XBios_Floprd(Params);
	 case 9:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS Flopwr()\n" );
		return XBios_Flopwr(Params);
	 case 15:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS Rsconf()\n" );
		return XBios_Rsconf(Params);
	 case 20:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS Scrdmp()\n" );
		return XBios_Scrdmp(Params);

	/* DHS specific debug patches */
	case 249:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS DHS CPUFreq()\n" );
		return XBios_CPUFreq(Params);
	case 250:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS DHS Registers()\n" );
		return XBios_Registers(Params);
	case 251:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS DHS DebugUI()\n" );
		return XBios_DebugUI(Params);
	case 252:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS DHS CounterRead()\n" );
		return XBios_CounterRead(Params);
	case 253:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS DHS CounterStart()\n" );
		return XBios_CounterStart(Params);
	case 254:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS DHS Debug()\n" );
		return XBios_Debug(Params);
	case 255:
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS DHS HatariOption()\n" );
		return XBios_HatariOption(Params);

	 default:  /* Call as normal! */
		HATARI_TRACE ( HATARI_TRACE_OS_XBIOS, "XBIOS %d\n", XBiosCall );
		return FALSE;
	}
}

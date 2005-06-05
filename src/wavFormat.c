/*
  Hatari - wavFormat.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  WAV File output

  As well as YM file output we also have output in .WAV format. These .WAV
  files can then be run through convertors to any other format, such as MP3.
  We simply save out the WAVE format headers and then write the sample data
  (at the current rate of playback) as we build it up each frame. When we stop
  recording we complete the size information in the headers and close up.


  RIFF Chunk (12 bytes in length total) Byte Number
    0 - 3  "RIFF" (ASCII Characters)
    4 - 7  Total Length Of Package To Follow (Binary, little endian)
    8 - 12  "WAVE" (ASCII Characters)

  FORMAT Chunk (24 bytes in length total) Byte Number
    0 - 3  "fmt_" (ASCII Characters)
    4 - 7  Length Of FORMAT Chunk (Binary, always 0x10)
    8 - 9  Always 0x01
    10 - 11  Channel Numbers (Always 0x01=Mono, 0x02=Stereo)
    12 - 15  Sample Rate (Binary, in Hz)
    16 - 19  Bytes Per Second
    20 - 21  Bytes Per Sample: 1=8 bit Mono, 2=8 bit Stereo or 16 bit Mono, 4=16 bit Stereo
    22 - 23  Bits Per Sample

  DATA Chunk Byte Number
    0 - 3  "data" (ASCII Characters)
    4 - 7  Length Of Data To Follow
    8 - end  Data (Samples)
*/
char WAVFormat_rcsid[] = "Hatari $Id: wavFormat.c,v 1.10 2005-06-05 14:19:39 thothy Exp $";

#include <SDL_endian.h>

#include "main.h"
#include "audio.h"
#include "configuration.h"
#include "file.h"
#include "log.h"
#include "sound.h"
#include "wavFormat.h"


static FILE *WavFileHndl;
static int nWavOutputBytes;             /* Number of samples bytes saved */
BOOL bRecordingWav = FALSE;             /* Is a WAV file open and recording? */


/*-----------------------------------------------------------------------*/
/*
*/
BOOL WAVFormat_OpenFile(char *pszWavFileName)
{
	const Uint32 Blank = 0;
	const Uint32 FmtLength = SDL_SwapLE32(0x10);
	const Uint16 SOne = SDL_SwapLE16(0x01);
	const Uint32 BitsPerSample = SDL_SwapLE32(8);
	Uint32 SampleLength;

	/* Set frequency (11Khz, 22Khz or 44Khz) */
	SampleLength = SDL_SwapLE32(SoundPlayBackFrequencies[ConfigureParams.Sound.nPlaybackQuality]);

	/* Create our file */
	WavFileHndl = fopen(pszWavFileName, "wb");

	if (WavFileHndl != NULL)
	{
		/* Create 'RIFF' chunk */
		fwrite("RIFF", 1, 4, WavFileHndl);                /* "RIFF" (ASCII Characters) */
		fwrite(&Blank, sizeof(Uint32), 1, WavFileHndl);   /* Total Length Of Package To Follow (Binary, little endian) */
		fwrite("WAVE", 1, 4, WavFileHndl);                /* "WAVE" (ASCII Characters) */

		/* Create 'FORMAT' chunk */
		fwrite("fmt ", 1, 4, WavFileHndl);                      /* "fmt_" (ASCII Characters) */
		fwrite(&FmtLength, sizeof(Uint32), 1, WavFileHndl);     /* Length Of FORMAT Chunk (Binary, always 0x10) */
		fwrite(&SOne, sizeof(Uint16), 1, WavFileHndl);          /* Always 0x01 */
		fwrite(&SOne, sizeof(Uint16), 1, WavFileHndl);          /* Channel Numbers (Always 0x01=Mono, 0x02=Stereo) */
		fwrite(&SampleLength, sizeof(Uint32), 1, WavFileHndl);  /* Sample Rate (Binary, in Hz) */
		fwrite(&SampleLength, sizeof(Uint32), 1, WavFileHndl);  /* Bytes Per Second */
		fwrite(&SOne, sizeof(Uint16), 1, WavFileHndl);          /* Bytes Per Sample: 1=8 bit Mono, 2=8 bit Stereo or 16 bit Mono, 4=16 bit Stereo */
		fwrite(&BitsPerSample, sizeof(Uint16), 1, WavFileHndl); /* Bits Per Sample */

		/* Create 'DATA' chunk */
		fwrite("data", 1, 4, WavFileHndl);                /* "data" (ASCII Characters) */
		fwrite(&Blank, sizeof(Uint32), 1, WavFileHndl);   /* Length Of Data To Follow */

		nWavOutputBytes = 0;
		bRecordingWav = TRUE;

		/* And inform user */
		Log_AlertDlg(LOG_INFO, "WAV sound data recording has been started.");
	}
	else
		bRecordingWav = FALSE;

	/* Ok, or failed? */
	return bRecordingWav;
}


/*-----------------------------------------------------------------------*/
/*
*/
void WAVFormat_CloseFile()
{
	if (bRecordingWav)
	{
		Uint32 nWavFileBytes;
		Uint32 nWavLEOutBytes;

		/* Update headers with sizes */
		nWavFileBytes = SDL_SwapLE32((12+24+8+nWavOutputBytes)-8);  /* File length, less 8 bytes for 'RIFF' and length */
		fseek(WavFileHndl, 4, SEEK_SET);                            /* 'Total Length Of Package' element */
		fwrite(&nWavFileBytes, sizeof(Uint32), 1, WavFileHndl);     /* Total Length Of Package in 'RIFF' chunk */

		fseek(WavFileHndl, 12+24+4, SEEK_SET);                      /* 'Length' element */
		nWavLEOutBytes = SDL_SwapLE32(nWavOutputBytes);
		fwrite(&nWavLEOutBytes, sizeof(Uint32), 1, WavFileHndl);    /* Length Of Data in 'DATA' chunk */

		/* Close file */
		fclose(WavFileHndl);
		WavFileHndl = NULL;
		bRecordingWav = FALSE;

		/* And inform user */
		Log_AlertDlg(LOG_INFO, "WAV Sound data recording has been stopped.");
	}
}


/*-----------------------------------------------------------------------*/
/*
*/
void WAVFormat_Update(char *pSamples, int Index, int Length)
{
	Sint8 sample;
	int i;

	if (bRecordingWav)
	{
		/* Output, better if did in two section if wrap */
		for(i = 0; i < Length; i++)
		{
			/* Convert sample to 'signed' byte */
			sample = pSamples[(Index+i)%MIXBUFFER_SIZE] ^ 128;
			/* And store */
			fwrite(&sample, sizeof(Sint8), 1, WavFileHndl);
		}

		/* Add samples to wav file */
		nWavOutputBytes += Length;
	}
}

/*
  Hatari - videl.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  Falcon Videl emulation. The Videl is the graphics shifter chip of the Falcon.
  It supports free programmable resolutions with 1, 2, 4, 8 or 16 bits per
  pixel.

  This file originally came from the Aranym project and has been heavily
  modified to work for Hatari (but the kudos for the great Videl emulation
  code goes to the people from the Aranym project of course).
*/
const char VIDEL_fileid[] = "Hatari videl.c : " __DATE__ " " __TIME__;

#include "main.h"
#include "configuration.h"
#include "ioMem.h"
#include "hostscreen.h"
#include "screen.h"
#include "stMemory.h"
#include "video.h"
#include "videl.h"
#include <SDL_endian.h>


#define handleRead(a) IoMem_ReadByte(a)
#define handleReadW(a) IoMem_ReadWord(a)
#define Atari2HostAddr(a) (&STRam[a])

#define VIDEL_DEBUG 0

#if VIDEL_DEBUG
#define Dprintf(a) printf a
#else
#define Dprintf(a)
#endif

#define HW	0xff8200
#define VIDEL_COLOR_REGS_BEGIN	0xff9800
#define VIDEL_COLOR_REGS_END	0xffa200


static int width, height, bpp, since_last_change;
static bool hostColorsSync;

/* Autozoom */
static int zoomwidth, prev_scrwidth;
static int zoomheight, prev_scrheight;
static int *zoomxtable;
static int *zoomytable;

static void VIDEL_renderScreenNoZoom(void);
static void VIDEL_renderScreenZoom(void);


// Called upon startup and when CPU encounters a RESET instruction.
void VIDEL_reset(void)
{
	since_last_change = 0;

	hostColorsSync = FALSE;

	/* Autozoom */
	zoomwidth=prev_scrwidth=0;
	zoomheight=prev_scrheight=0;
	zoomxtable=NULL;
	zoomytable=NULL;

	// default resolution to boot with
	width = 640;
	height = 480;
	HostScreen_setWindowSize(width, height, ConfigureParams.Screen.nForceBpp);
}

// monitor write access to Falcon and ST/E color palette registers
void VIDEL_ColorRegsWrite(void)
{
	hostColorsSync = FALSE;
}

void VIDEL_ShiftModeWriteWord(void)
{
	Dprintf(("VIDEL f_shift: %06x = 0x%x\n", IoAccessBaseAddress, handleReadW(HW+0x66)));
	bUseSTShifter = FALSE;
}

static long VIDEL_getVideoramAddress(void)
{
	return (handleRead(HW + 1) << 16) | (handleRead(HW + 3) << 8) | handleRead(HW + 0x0d);
}

static int VIDEL_getScreenBpp(void)
{
	int f_shift = handleReadW(HW + 0x66);
	int st_shift = handleRead(HW + 0x60);
	/* to get bpp, we must examine f_shift and st_shift.
	 * f_shift is valid if any of bits no. 10, 8 or 4
	 * is set. Priority in f_shift is: 10 ">" 8 ">" 4, i.e.
	 * if bit 10 set then bit 8 and bit 4 don't care...
	 * If all these bits are 0 and ST shifter is written
	 * after Falcon one, get display depth from st_shift
	 * (as for ST and STE)
	 */
	int bits_per_pixel;
	if (f_shift & 0x400)		/* Falcon: 2 colors */
		bits_per_pixel = 1;
	else if (f_shift & 0x100)	/* Falcon: hicolor */
		bits_per_pixel = 16;
	else if (f_shift & 0x010)	/* Falcon: 8 bitplanes */
		bits_per_pixel = 8;
	else if (!bUseSTShifter)	/* Falcon: 4 bitplanes */
		bits_per_pixel = 4;
	else if (st_shift == 0)
		bits_per_pixel = 4;
	else if (st_shift == 0x01)
		bits_per_pixel = 2;
	else /* if (st_shift == 0x02) */
		bits_per_pixel = 1;

	// Dprintf(("Videl works in %d bpp, f_shift=%04x, st_shift=%d", bits_per_pixel, f_shift, st_shift));

	return bits_per_pixel;
}

static int VIDEL_getScreenWidth(void)
{
	return (handleReadW(HW + 0x10) & 0x03ff) * 16 / VIDEL_getScreenBpp();
}

static int VIDEL_getScreenHeight(void)
{
	int vdb = handleReadW(HW + 0xa8) & 0x07ff;
	int vde = handleReadW(HW + 0xaa) & 0x07ff;
	int vmode = handleReadW(HW + 0xc2);

	/* visible y resolution:
	 * Graphics display starts at line VDB and ends at line
	 * VDE. If interlace mode off unit of VC-registers is
	 * half lines, else lines.
	 */
	int yres = vde - vdb;
	if (!(vmode & 0x02))		// interlace
		yres >>= 1;
	if (vmode & 0x01)		// double
		yres >>= 1;

	return yres;
}


/** map the correct colortable into the correct pixel format
 */
static void VIDEL_updateColors(void)
{
	//Dprintf(("ColorUpdate in progress\n"));

	int i, r, g, b, colors = 1 << bpp;

#define F_COLORS(i) handleRead(VIDEL_COLOR_REGS_BEGIN + (i))
#define STE_COLORS(i)	handleRead(0xff8240 + (i))

	if (!bUseSTShifter) {
		for (i = 0; i < colors; i++) {
			int offset = i << 2;
			r = F_COLORS(offset) & 0xfc;
			r |= r>>6;
			g = F_COLORS(offset + 1) & 0xfc;
			g |= g>>6;
			b = F_COLORS(offset + 3) & 0xfc;
			b |= b>>6;
			HostScreen_setPaletteColor(i, r,g,b);
		}
		HostScreen_updatePalette(colors);
	} else {
		for (i = 0; i < colors; i++) {
			int offset = i << 1;
			r = STE_COLORS(offset) & 0x0f;
			r = ((r & 7)<<1)|(r>>3);
			r |= r<<4;
			g = (STE_COLORS(offset + 1)>>4) & 0x0f;
			g = ((g & 7)<<1)|(g>>3);
			g |= g<<4;
			b = STE_COLORS(offset + 1) & 0x0f;
			b = ((b & 7)<<1)|(b>>3);
			b |= b<<4;
			HostScreen_setPaletteColor(i, r,g,b);
		}
		HostScreen_updatePalette(colors);
	}

	hostColorsSync = TRUE;
}


void VIDEL_ZoomModeChanged(void)
{
	/* User selected another zoom mode, so set a new screen resolution now */
	HostScreen_setWindowSize(width, height, bpp == 16 ? 16 : ConfigureParams.Screen.nForceBpp);
}


bool VIDEL_renderScreen(void)
{
	int vw	 = VIDEL_getScreenWidth();
	int vh	 = VIDEL_getScreenHeight();
	int vbpp = VIDEL_getScreenBpp();

	if (since_last_change > 2) {
		if (vw > 0 && vw != width) {
			Dprintf(("CH width %d\n", width));
			width = vw;
			since_last_change = 0;
		}
		if (vh > 0 && vh != height) {
			Dprintf(("CH height %d\n", width));
			height = vh;
			since_last_change = 0;
		}
		if (vbpp != bpp) {
			Dprintf(("CH bpp %d\n", vbpp));
			bpp = vbpp;
			since_last_change = 0;
		}
	}
	if (since_last_change == 3) {
		HostScreen_setWindowSize(width, height, bpp == 16 ? 16 : ConfigureParams.Screen.nForceBpp);
	}
	if (since_last_change < 4) {
		since_last_change++;
		return false;
	}

	if (!HostScreen_renderBegin())
		return false;

	if (ConfigureParams.Screen.bZoomLowRes) {
		VIDEL_renderScreenZoom();
	} else {
		VIDEL_renderScreenNoZoom();
	}

	HostScreen_renderEnd();

	HostScreen_update1( FALSE );

	return true;
}


/**
 * Performs conversion from the TOS's bitplane word order (big endian) data
 * into the native chunky color index.
 */
static void Videl_bitplaneToChunky(Uint16 *atariBitplaneData, Uint16 bpp,
                                   Uint8 colorValues[16])
{
	Uint32 a, b, c, d, x;

	/* Obviously the different cases can be broken out in various
	 * ways to lessen the amount of work needed for <8 bit modes.
	 * It's doubtful if the usage of those modes warrants it, though.
	 * The branches below should be ~100% correctly predicted and
	 * thus be more or less for free.
	 * Getting the palette values inline does not seem to help
	 * enough to worry about. The palette lookup is much slower than
	 * this code, though, so it would be nice to do something about it.
	 */
	if (bpp >= 4) {
		d = *(Uint32 *)&atariBitplaneData[0];
		c = *(Uint32 *)&atariBitplaneData[2];
		if (bpp == 4) {
			a = b = 0;
		} else {
			b = *(Uint32 *)&atariBitplaneData[4];
			a = *(Uint32 *)&atariBitplaneData[6];
		}
	} else {
		a = b = c = 0;
		if (bpp == 2) {
			d = *(Uint32 *)&atariBitplaneData[0];
		} else {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			d = atariBitplaneData[0]<<16;
#else
			d = atariBitplaneData[0];
#endif
		}
	}

	x = a;
	a =  (a & 0xf0f0f0f0)       | ((c & 0xf0f0f0f0) >> 4);
	c = ((x & 0x0f0f0f0f) << 4) |  (c & 0x0f0f0f0f);
	x = b;
	b =  (b & 0xf0f0f0f0)       | ((d & 0xf0f0f0f0) >> 4);
	d = ((x & 0x0f0f0f0f) << 4) |  (d & 0x0f0f0f0f);

	x = a;
	a =  (a & 0xcccccccc)       | ((b & 0xcccccccc) >> 2);
	b = ((x & 0x33333333) << 2) |  (b & 0x33333333);
	x = c;
	c =  (c & 0xcccccccc)       | ((d & 0xcccccccc) >> 2);
	d = ((x & 0x33333333) << 2) |  (d & 0x33333333);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	a = (a & 0x5555aaaa) | ((a & 0x00005555) << 17) | ((a & 0xaaaa0000) >> 17);
	b = (b & 0x5555aaaa) | ((b & 0x00005555) << 17) | ((b & 0xaaaa0000) >> 17);
	c = (c & 0x5555aaaa) | ((c & 0x00005555) << 17) | ((c & 0xaaaa0000) >> 17);
	d = (d & 0x5555aaaa) | ((d & 0x00005555) << 17) | ((d & 0xaaaa0000) >> 17);

	colorValues[ 8] = a;
	a >>= 8;
	colorValues[ 0] = a;
	a >>= 8;
	colorValues[ 9] = a;
	a >>= 8;
	colorValues[ 1] = a;

	colorValues[10] = b;
	b >>= 8;
	colorValues[ 2] = b;
	b >>= 8;
	colorValues[11] = b;
	b >>= 8;
	colorValues[ 3] = b;

	colorValues[12] = c;
	c >>= 8;
	colorValues[ 4] = c;
	c >>= 8;
	colorValues[13] = c;
	c >>= 8;
	colorValues[ 5] = c;

	colorValues[14] = d;
	d >>= 8;
	colorValues[ 6] = d;
	d >>= 8;
	colorValues[15] = d;
	d >>= 8;
	colorValues[ 7] = d;
#else
	a = (a & 0xaaaa5555) | ((a & 0x0000aaaa) << 15) | ((a & 0x55550000) >> 15);
	b = (b & 0xaaaa5555) | ((b & 0x0000aaaa) << 15) | ((b & 0x55550000) >> 15);
	c = (c & 0xaaaa5555) | ((c & 0x0000aaaa) << 15) | ((c & 0x55550000) >> 15);
	d = (d & 0xaaaa5555) | ((d & 0x0000aaaa) << 15) | ((d & 0x55550000) >> 15);

	colorValues[ 1] = a;
	a >>= 8;
	colorValues[ 9] = a;
	a >>= 8;
	colorValues[ 0] = a;
	a >>= 8;
	colorValues[ 8] = a;

	colorValues[ 3] = b;
	b >>= 8;
	colorValues[11] = b;
	b >>= 8;
	colorValues[ 2] = b;
	b >>= 8;
	colorValues[10] = b;

	colorValues[ 5] = c;
	c >>= 8;
	colorValues[13] = c;
	c >>= 8;
	colorValues[ 4] = c;
	c >>= 8;
	colorValues[12] = c;

	colorValues[ 7] = d;
	d >>= 8;
	colorValues[15] = d;
	d >>= 8;
	colorValues[ 6] = d;
	d >>= 8;
	colorValues[14] = d;
#endif
}


static void VIDEL_renderScreenNoZoom(void)
{
	int vw	 = VIDEL_getScreenWidth();
	int vh	 = VIDEL_getScreenHeight();

	int lineoffset = handleReadW(HW + 0x0e) & 0x01ff; // 9 bits
	int linewidth = handleReadW(HW + 0x10) & 0x03ff; // 10 bits
	/* 
	   I think this implementation is naive: 
	   indeed, I suspect that we should instead skip lineoffset
	   words each time we have read "more" than linewidth words
	   (possibly "more" because of the number of bit planes).
	   Moreover, the 1 bit plane mode is particular;
	   while doing some experiments on my Falcon, it seems to
	   behave like the 4 bit planes mode.
	   At last, we have also to take into account the 4 bits register
	   located at the word $ffff8264 (bit offset). This register makes
	   the semantics of the lineoffset register change a little.
	   int bitoffset = handleReadW(HW + 0x64) & 0x000f;
	   The meaning of this register in True Color mode is not clear
	   for me at the moment (and my experiments on the Falcon don't help
	   me).
	*/
	int nextline = linewidth + lineoffset;

	if (bpp < 16 && !hostColorsSync) {
		VIDEL_updateColors();
	}

	VIDEL_ConvertScreenNoZoom(vw, vh, bpp, nextline);
}


void VIDEL_ConvertScreenNoZoom(int vw, int vh, int vbpp, int nextline)
{
	int scrpitch = HostScreen_getPitch();

	long atariVideoRAM = VIDEL_getVideoramAddress();

	Uint16 *fvram = (Uint16 *) Atari2HostAddr(atariVideoRAM);
	Uint8 *hvram = HostScreen_getVideoramAddress();

	int hscrolloffset = (handleRead(HW + 0x65) & 0x0f);

	/* Clip to SDL_Surface dimensions */
	int scrwidth = HostScreen_getWidth();
	int scrheight = HostScreen_getHeight();
	int vw_clip = vw;
	int vh_clip = vh;
	if (vw>scrwidth) vw_clip = scrwidth;
	if (vh>scrheight) vh_clip = scrheight;	

	/* Horizontal scroll register set? */
	if (hscrolloffset) {
		/* Yes, so we need to adjust offset to next line: */
		nextline += vbpp;
	}

	/* Center screen */
	hvram += ((scrheight-vh_clip)>>1)*scrpitch;
	hvram += ((scrwidth-vw_clip)>>1)*HostScreen_getBpp();

	/* Render */
	if (vbpp < 16) {
		/* Bitplanes modes */

		// The SDL colors blitting...
		Uint8 color[16];

		// FIXME: The byte swap could be done here by enrolling the loop into 2 each by 8 pixels
		switch ( HostScreen_getBpp() ) {
			case 1:
				{
					Uint16 *fvram_line = fvram;
					Uint8 *hvram_line = hvram;
					int h;

					for (h = 0; h < vh_clip; h++) {
						Uint16 *fvram_column = fvram_line;
						Uint8 *hvram_column = hvram_line;
						int w;

						/* First 16 pixels: */
						Videl_bitplaneToChunky(fvram_column, vbpp, color);
						memcpy(hvram_column, color+hscrolloffset, 16-hscrolloffset);
						hvram_column += 16-hscrolloffset;
						fvram_column += vbpp;
						/* Now the main part of the line: */
						for (w = 1; w < (vw_clip+15)>>4; w++) {
							Videl_bitplaneToChunky( fvram_column, vbpp, color );
							memcpy(hvram_column, color, 16);
							hvram_column += 16;
							fvram_column += vbpp;
						}
						/* Last pixels of the line for fine scrolling: */
						if (hscrolloffset) {
							Videl_bitplaneToChunky(fvram_column, vbpp, color);
							memcpy(hvram_column, color, hscrolloffset);
						}

						hvram_line += scrpitch;
						fvram_line += nextline;
					}
				}
				break;
			case 2:
				{
					Uint16 *fvram_line = fvram;
					Uint16 *hvram_line = (Uint16 *)hvram;
					int h;

					for (h = 0; h < vh_clip; h++) {
						Uint16 *fvram_column = fvram_line;
						Uint16 *hvram_column = hvram_line;
						int w, j;

						/* First 16 pixels: */
						Videl_bitplaneToChunky(fvram_column, vbpp, color);
						for (j = 0; j < 16 - hscrolloffset; j++) {
							*hvram_column++ = HostScreen_getPaletteColor(color[j+hscrolloffset]);
						}
						fvram_column += vbpp;
						/* Now the main part of the line: */
						for (w = 1; w < (vw_clip+15)>>4; w++) {
							Videl_bitplaneToChunky( fvram_column, vbpp, color );
							for (j=0; j<16; j++) {
								*hvram_column++ = HostScreen_getPaletteColor( color[j] );
							}
							fvram_column += vbpp;
						}
						/* Last pixels of the line for fine scrolling: */
						if (hscrolloffset) {
							Videl_bitplaneToChunky(fvram_column, vbpp, color);
							for (j = 0; j < hscrolloffset; j++) {
								*hvram_column++ = HostScreen_getPaletteColor(color[j]);
							}
						}

						hvram_line += scrpitch>>1;
						fvram_line += nextline;
					}
				}
				break;
			case 3:
				{
					Uint16 *fvram_line = fvram;
					Uint8 *hvram_line = hvram;
					int h;

					for (h = 0; h < vh_clip; h++) {
						Uint16 *fvram_column = fvram_line;
						Uint8 *hvram_column = hvram_line;
						int w;

						for (w = 0; w < (vw_clip+15)>>4; w++) {
							int j;
							Videl_bitplaneToChunky( fvram_column, vbpp, color );

							for (j=0; j<16; j++) {
								Uint32 tmpColor = HostScreen_getPaletteColor( color[j] );
								putBpp24Pixel( hvram_column, tmpColor );
								hvram_column += 3;
							}

							fvram_column += vbpp;
						}

						hvram_line += scrpitch;
						fvram_line += nextline;
					}
				}
				break;
			case 4:
				{
					Uint16 *fvram_line = fvram;
					Uint32 *hvram_line = (Uint32 *)hvram;
					int h;

					for (h = 0; h < vh_clip; h++) {
						Uint16 *fvram_column = fvram_line;
						Uint32 *hvram_column = hvram_line;
						int w, j;

						/* First 16 pixels: */
						Videl_bitplaneToChunky(fvram_column, vbpp, color);
						for (j = 0; j < 16 - hscrolloffset; j++) {
							*hvram_column++ = HostScreen_getPaletteColor(color[j+hscrolloffset]);
						}
						fvram_column += vbpp;
						/* Now the main part of the line: */
						for (w = 1; w < (vw_clip+15)>>4; w++) {
							Videl_bitplaneToChunky( fvram_column, vbpp, color );
							for (j=0; j<16; j++) {
								*hvram_column++ = HostScreen_getPaletteColor( color[j] );
							}
							fvram_column += vbpp;
						}
						/* Last pixels of the line for fine scrolling: */
						if (hscrolloffset) {
							Videl_bitplaneToChunky(fvram_column, vbpp, color);
							for (j = 0; j < hscrolloffset; j++) {
								*hvram_column++ = HostScreen_getPaletteColor(color[j]);
							}
						}

						hvram_line += scrpitch>>2;
						fvram_line += nextline;
					}
				}
				break;
		}

	} else {

		// Falcon TC (High Color)
		switch ( HostScreen_getBpp() )  {
			case 1:
				{
					/* FIXME: when Videl switches to 16bpp, set the palette to 3:3:2 */
					Uint16 *fvram_line = fvram;
					Uint8 *hvram_line = hvram;
					int h;

					for (h = 0; h < vh_clip; h++) {
						Uint16 *fvram_column = fvram_line;
						Uint8 *hvram_column = hvram_line;
						int w, tmp;

						for (w = 0; w < vw_clip; w++) {
							
							tmp = SDL_SwapBE16(*fvram_column);

							*hvram_column = ((tmp>>13) & 7) << 5;
							*hvram_column |= ((tmp>>8) & 7) << 2;
							*hvram_column |= ((tmp>>2) & 3);

							hvram_column++;
							fvram_column++;
						}

						hvram_line += scrpitch;
						fvram_line += nextline;
					}
				}
				break;
			case 2:
				{
					Uint16 *fvram_line = fvram;
					Uint16 *hvram_line = (Uint16 *)hvram;
					int h;

					for (h = 0; h < vh_clip; h++) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
						//FIXME: here might be a runtime little/big video endian switch like:
						//      if ( /* videocard memory in Motorola endian format */ false)
						memcpy(hvram_line, fvram_line, vw_clip<<1);
#else
						int w;
						Uint16 *fvram_column = fvram_line;
						Uint16 *hvram_column = hvram_line;

						for (w = 0; w < vw_clip; w++) {
							// byteswap with SDL asm macros
							*hvram_column++ = SDL_SwapBE16(*fvram_column++);
						}
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN

						hvram_line += scrpitch>>1;
						fvram_line += nextline;
					}
				}
				break;
			case 3:
				{
					Uint16 *fvram_line = fvram;
					Uint8 *hvram_line = hvram;
					int h;

					for (h = 0; h < vh_clip; h++) {
						Uint16 *fvram_column = fvram_line;
						Uint8 *hvram_column = hvram_line;
						int w;

						for (w = 0; w < vw_clip; w++) {
							int data = *fvram_column++;

							Uint32 tmpColor =
								HostScreen_getColor(
									(Uint8) (data & 0xf8),
									(Uint8) ( ((data & 0x07) << 5) |
											  ((data >> 11) & 0x3c)),
									(Uint8) ((data >> 5) & 0xf8));
							
							putBpp24Pixel( hvram_column, tmpColor );

							hvram_column += 3;
						}

						hvram_line += scrpitch;
						fvram_line += nextline;
					}
				}
				break;
			case 4:
				{
					Uint16 *fvram_line = fvram;
					Uint32 *hvram_line = (Uint32 *)hvram;
					int h;

					for (h = 0; h < vh_clip; h++) {
						Uint16 *fvram_column = fvram_line;
						Uint32 *hvram_column = hvram_line;
						int w;

						for (w = 0; w < vw_clip; w++) {
							int data = *fvram_column++;

							*hvram_column++ =
								HostScreen_getColor(
									(Uint8) (data & 0xf8),
									(Uint8) ( ((data & 0x07) << 5) |
											  ((data >> 11) & 0x3c)),
									(Uint8) ((data >> 5) & 0xf8));
						}

						hvram_line += scrpitch>>2;
						fvram_line += nextline;
					}
				}
				break;
		}
	}
}


static void VIDEL_renderScreenZoom(void)
{
	/* Atari screen infos */
	int vw	 = VIDEL_getScreenWidth();
	int vh	 = VIDEL_getScreenHeight();

	int lineoffset = handleReadW(HW + 0x0e) & 0x01ff; // 9 bits
	int linewidth = handleReadW(HW + 0x10) & 0x03ff; // 10 bits
	/* same remark as before: too naive */
	int nextline = linewidth + lineoffset;

	if ((vw<32) || (vh<32))  return;

	if (bpp<16 && !hostColorsSync) {
		VIDEL_updateColors();
	}

	VIDEL_ConvertScreenZoom(vw, vh, bpp, nextline);
}


void VIDEL_ConvertScreenZoom(int vw, int vh, int vbpp, int nextline)
{
	int i, j, w, h, cursrcline;

	Uint16 *fvram = (Uint16 *) Atari2HostAddr(VIDEL_getVideoramAddress());

	/* Host screen infos */
	int scrpitch = HostScreen_getPitch();
	int scrwidth = HostScreen_getWidth();
	int scrheight = HostScreen_getHeight();
	int scrbpp = HostScreen_getBpp();
	Uint8 *hvram = (Uint8 *) HostScreen_getVideoramAddress();

	int hscrolloffset = (handleRead(HW + 0x65) & 0x0f);

	/* Horizontal scroll register set? */
	if (hscrolloffset) {
		/* Yes, so we need to adjust offset to next line: */
		nextline += vbpp;
	}

	/* Integer zoom coef ? */
	if (/*(bx_options.autozoom.integercoefs) &&*/ (scrwidth>=vw) && (scrheight>=vh)) {
		int coefx = scrwidth/vw;
		int coefy = scrheight/vh;

		scrwidth = vw * coefx;
		scrheight = vh * coefy;

		/* Center screen */
		hvram += ((HostScreen_getHeight()-scrheight)>>1)*scrpitch;
		hvram += ((HostScreen_getWidth()-scrwidth)>>1)*scrbpp;
	}

	/* New zoom ? */
	if ((zoomwidth != vw) || (scrwidth != prev_scrwidth)) {
		if (zoomxtable) {
			free(zoomxtable);
		}
		zoomxtable = malloc(sizeof(int)*scrwidth);
		for (i=0; i<scrwidth; i++) {
			zoomxtable[i] = (vw*i)/scrwidth;
		}
		zoomwidth = vw;
		prev_scrwidth = scrwidth;
	}
	if ((zoomheight != vh) || (scrheight != prev_scrheight)) {
		if (zoomytable) {
			free(zoomytable);
		}
		zoomytable = malloc(sizeof(int)*scrheight);
		for (i=0; i<scrheight; i++) {
			zoomytable[i] = (vh*i)/scrheight;
		}
		zoomheight = vh;
		prev_scrheight = scrheight;
	}

	cursrcline = -1;

	if (vbpp<16) {
		Uint8 color[16];

		/* Bitplanes modes */
		switch(scrbpp) {
			case 1:
				{
					/* One complete planar 2 chunky line */
					Uint8 *p2cline = malloc(sizeof(Uint8)*vw);

					Uint16 *fvram_line;
					Uint8 *hvram_line = hvram;

					for (h = 0; h < scrheight; h++) {
						fvram_line = fvram + (zoomytable[h] * nextline);

						/* Recopy the same line ? */
						if (zoomytable[h] == cursrcline) {
							memcpy(hvram_line, hvram_line-scrpitch, scrwidth*scrbpp);
						} else {
							Uint16 *fvram_column = fvram_line;
							Uint8 *hvram_column = p2cline;

							/* First 16 pixels of a new line */
							Videl_bitplaneToChunky(fvram_column, vbpp, color);
							memcpy(hvram_column, color+hscrolloffset, 16-hscrolloffset);
							hvram_column += 16-hscrolloffset;
							fvram_column += vbpp;
							/* Convert main part of the new line */
							for (w=1; w < (vw+15)>>4; w++) {
								Videl_bitplaneToChunky( fvram_column, vbpp, color );
								memcpy(hvram_column, color, 16);
								hvram_column += 16;
								fvram_column += vbpp;
							}
							/* Last pixels of the line for fine scrolling: */
							if (hscrolloffset) {
								Videl_bitplaneToChunky(fvram_column, vbpp, color);
								memcpy(hvram_column, color, hscrolloffset);
							}

							/* Zoom a new line */
							for (w=0; w<scrwidth; w++) {
								hvram_line[w] = p2cline[zoomxtable[w]];
							}
						}

						hvram_line += scrpitch;
						cursrcline = zoomytable[h];
					}

					free(p2cline);
				}
				break;
			case 2:
				{
					/* One complete planar 2 chunky line */
					Uint16 *p2cline = malloc(sizeof(Uint16)*vw);

					Uint16 *fvram_line = fvram;
					Uint16 *hvram_line = (Uint16 *)hvram;

					for (h = 0; h < scrheight; h++) {
						fvram_line = fvram + (zoomytable[h] * nextline);

						/* Recopy the same line ? */
						if (zoomytable[h] == cursrcline) {
							memcpy(hvram_line, hvram_line-(scrpitch>>1), scrwidth*scrbpp);
						} else {
							Uint16 *fvram_column = fvram_line;
							Uint16 *hvram_column = p2cline;

							/* First 16 pixels of a new line: */
							Videl_bitplaneToChunky(fvram_column, vbpp, color);
							for (j = 0; j < 16 - hscrolloffset; j++) {
								*hvram_column++ = HostScreen_getPaletteColor(color[j+hscrolloffset]);
							}
							fvram_column += vbpp;
							/* Convert the main part of the new line: */
							for (w = 1; w < (vw+15)>>4; w++) {
								Videl_bitplaneToChunky( fvram_column, vbpp, color );
								for (j=0; j<16; j++) {
									*hvram_column++ = HostScreen_getPaletteColor( color[j] );
								}
								fvram_column += vbpp;
							}
							/* Last pixels of the new line for fine scrolling: */
							if (hscrolloffset) {
								Videl_bitplaneToChunky(fvram_column, vbpp, color);
								for (j = 0; j < hscrolloffset; j++) {
									*hvram_column++ = HostScreen_getPaletteColor(color[j]);
								}
							}

							/* Zoom a new line */
							for (w=0; w<scrwidth; w++) {
								hvram_line[w] = p2cline[zoomxtable[w]];
							}
						}

						hvram_line += scrpitch>>1;
						cursrcline = zoomytable[h];
					}

					free(p2cline);
				}
				break;
			case 3:
				{
					/* One complete planar 2 chunky line */
					Uint8 *p2cline = malloc(sizeof(Uint8)*vw*3);

					Uint16 *fvram_line;
					Uint8 *hvram_line = hvram;

					for (h = 0; h < scrheight; h++) {
						fvram_line = fvram + (zoomytable[h] * nextline);

						/* Recopy the same line ? */
						if (zoomytable[h] == cursrcline) {
							memcpy(hvram_line, hvram_line-scrpitch, scrwidth*scrbpp);
						} else {
							Uint16 *fvram_column = fvram_line;
							Uint8 *hvram_column = p2cline;

							/* Convert a new line */
							for (w=0; w < (vw+15)>>4; w++) {
								Videl_bitplaneToChunky( fvram_column, vbpp, color );

								for (j=0; j<16; j++) {
									Uint32 tmpColor = HostScreen_getPaletteColor( color[j] );
									putBpp24Pixel( hvram_column, tmpColor );
									hvram_column += 3;
								}

								fvram_column += vbpp;
							}
							
							/* Zoom a new line */
							for (w=0; w<scrwidth; w++) {
								hvram_line[w*3] = p2cline[zoomxtable[w]*3];
								hvram_line[w*3+1] = p2cline[zoomxtable[w]*3+1];
								hvram_line[w*3+2] = p2cline[zoomxtable[w]*3+2];
							}
						}

						hvram_line += scrpitch;
						cursrcline = zoomytable[h];
					}

					free(p2cline);
				}
				break;
			case 4:
				{
					/* One complete planar 2 chunky line */
					Uint32 *p2cline = malloc(sizeof(Uint32)*vw);

					Uint16 *fvram_line;
					Uint32 *hvram_line = (Uint32 *)hvram;

					for (h = 0; h < scrheight; h++) {
						fvram_line = fvram + (zoomytable[h] * nextline);

						/* Recopy the same line ? */
						if (zoomytable[h] == cursrcline) {
							memcpy(hvram_line, hvram_line-(scrpitch>>2), scrwidth*scrbpp);
						} else {
							Uint16 *fvram_column = fvram_line;
							Uint32 *hvram_column = p2cline;

							/* First 16 pixels of a new line: */
							Videl_bitplaneToChunky(fvram_column, vbpp, color);
							for (j = 0; j < 16 - hscrolloffset; j++) {
								*hvram_column++ = HostScreen_getPaletteColor(color[j+hscrolloffset]);
							}
							fvram_column += vbpp;
							/* Convert the main part of the new line: */
							for (w = 1; w < (vw+15)>>4; w++) {
								Videl_bitplaneToChunky( fvram_column, vbpp, color );
								for (j=0; j<16; j++) {
									*hvram_column++ = HostScreen_getPaletteColor( color[j] );
								}
								fvram_column += vbpp;
							}
							/* Last pixels of the new line for fine scrolling: */
							if (hscrolloffset) {
								Videl_bitplaneToChunky(fvram_column, vbpp, color);
								for (j = 0; j < hscrolloffset; j++) {
									*hvram_column++ = HostScreen_getPaletteColor(color[j]);
								}
							}

							/* Zoom a new line */
							for (w=0; w<scrwidth; w++) {
								hvram_line[w] = p2cline[zoomxtable[w]];
							}
						}

						hvram_line += scrpitch>>2;
						cursrcline = zoomytable[h];
					}

					free(p2cline);
				}
				break;
		}
	} else {
		/* Falcon TrueColour mode */

		switch(scrbpp) {
			case 1:
				{
					/* FIXME: when Videl switches to 16bpp, set the palette to 3:3:2 */
					Uint16 *fvram_line;
					Uint8 *hvram_line = hvram;

					for (h = 0; h < scrheight; h++) {
						Uint16 *fvram_column;
						Uint8 *hvram_column;

						fvram_line = fvram + (zoomytable[h] * nextline);
						fvram_column = fvram_line;
						hvram_column = hvram_line;

						/* Recopy the same line ? */
						if (zoomytable[h] == cursrcline) {
							memcpy(hvram_line, hvram_line-scrpitch, scrwidth*scrbpp);
						} else {
							for (w = 0; w < scrwidth; w++) {
								Uint16 srcword;
								Uint8 dstbyte;
							
								srcword = SDL_SwapBE16(fvram_column[zoomxtable[w]]);

								dstbyte = ((srcword>>13) & 7) << 5;
								dstbyte |= ((srcword>>8) & 7) << 2;
								dstbyte |= ((srcword>>2) & 3);

								*hvram_column++ = dstbyte;
							}
						}

						hvram_line += scrpitch;
						cursrcline = zoomytable[h];
					}
				}
				break;
			case 2:
				{
					Uint16 *fvram_line;
					Uint16 *hvram_line = (Uint16 *)hvram;

					for (h = 0; h < scrheight; h++) {
						Uint16 *fvram_column;
						Uint16 *hvram_column;

						fvram_line = fvram + (zoomytable[h] * nextline);
						fvram_column = fvram_line;
						hvram_column = hvram_line;

						/* Recopy the same line ? */
						if (zoomytable[h] == cursrcline) {
							memcpy(hvram_line, hvram_line-(scrpitch>>1), scrwidth*scrbpp);
						} else {
							for (w = 0; w < scrwidth; w++) {
								Uint16 srcword;
							
								srcword = SDL_SwapBE16(fvram_column[zoomxtable[w]]);
								*hvram_column++ = srcword;
							}
						}

						hvram_line += scrpitch>>1;
						cursrcline = zoomytable[h];
					}
				}
				break;
			case 3:
				{
					Uint16 *fvram_line;
					Uint8 *hvram_line = hvram;

					for (h = 0; h < scrheight; h++) {
						Uint16 *fvram_column;
						Uint8 *hvram_column;

						fvram_line = fvram + (zoomytable[h] * nextline);
						fvram_column = fvram_line;
						hvram_column = hvram_line;

						/* Recopy the same line ? */
						if (zoomytable[h] == cursrcline) {
							memcpy(hvram_line, hvram_line-scrpitch, scrwidth*scrbpp);
						} else {
							for (w = 0; w < scrwidth; w++) {
								Uint16 srcword;
								Uint32 dstlong;
							
								srcword = fvram_column[zoomxtable[w]];

								dstlong = HostScreen_getColor(
										(Uint8) (srcword & 0xf8),
										(Uint8) ( ((srcword & 0x07) << 5) |
											  ((srcword >> 11) & 0x3c)),
										(Uint8) ((srcword >> 5) & 0xf8));

								putBpp24Pixel( hvram_column, dstlong );
								hvram_column += 3;
							}
						}

						hvram_line += scrpitch;
						cursrcline = zoomytable[h];
					}
				}
				break;
			case 4:
				{
					Uint16 *fvram_line;
					Uint32 *hvram_line = (Uint32 *)hvram;

					for (h = 0; h < scrheight; h++) {
						Uint16 *fvram_column;
						Uint32 *hvram_column;

						fvram_line = fvram + (zoomytable[h] * nextline);
						fvram_column = fvram_line;
						hvram_column = hvram_line;

						/* Recopy the same line ? */
						if (zoomytable[h] == cursrcline) {
							memcpy(hvram_line, hvram_line-(scrpitch>>2), scrwidth*scrbpp);
						} else {
							for (w = 0; w < scrwidth; w++) {
								Uint16 srcword;
							
								srcword = fvram_column[zoomxtable[w]];

								*hvram_column++ =
									HostScreen_getColor(
										(Uint8) (srcword & 0xf8),
										(Uint8) ( ((srcword & 0x07) << 5) |
											  ((srcword >> 11) & 0x3c)),
										(Uint8) ((srcword >> 5) & 0xf8));
							}
						}

						hvram_line += scrpitch>>2;
						cursrcline = zoomytable[h];
					}
				}
				break;
		}
	}
}

/*
  Hatari - hostscreen.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  Host video routines. This file originally came from the Aranym project but
  has been thoroughly reworked for Hatari. However, integration with the rest
  of the Hatari source code is still bad and needs a lot of improvement...
*/
const char HostScreen_rcsid[] = "Hatari $Id: hostscreen.c,v 1.6 2006-10-10 20:13:05 thothy Exp $";

#include "main.h"
#include "configuration.h"
#include "sysdeps.h"
#include "stMemory.h"
#include "ioMem.h"
#include "hostscreen.h"
#include "screen.h"

#define VIDEL_DEBUG 1

#if VIDEL_DEBUG
#define Dprintf(a) printf a
#else
#define Dprintf(a)
#endif


#define RGB_BLACK     0x00000000
#define RGB_BLUE      0x000000ff
#define RGB_GREEN     0x00ff0000
#define RGB_CYAN      0x00ff00ff
#define RGB_RED       0xff000000
#define RGB_MAGENTA   0xff0000ff
#define RGB_LTGRAY    0xbbbb00bb
#define RGB_GRAY      0x88880088
#define RGB_LTBLUE    0x000000aa
#define RGB_LTGREEN   0x00aa0000
#define RGB_LTCYAN    0x00aa00aa
#define RGB_LTRED     0xaa000000
#define RGB_LTMAGENTA 0xaa0000aa
#define RGB_YELLOW    0xffff0000
#define RGB_LTYELLOW  0xaaaa0000
#define RGB_WHITE     0xffff00ff


static SDL_Surface *mainSurface;        // The main window surface
static SDL_Surface *surf;               // pointer to actual surface


static SDL_mutex   *screenLock;
static uint32 sdl_videoparams;
static uint32 hs_width, hs_height, hs_bpp;
static BOOL   doUpdate; // the HW surface is available -> the SDL need not to update the surface after ->pixel access

static struct { // TOS palette (bpp < 16) to SDL color mapping
	SDL_Color	standard[256];
	uint32		native[256];
} palette;


static const unsigned long default_palette[] = {
    RGB_WHITE, RGB_RED, RGB_GREEN, RGB_YELLOW,
    RGB_BLUE, RGB_MAGENTA, RGB_CYAN, RGB_LTGRAY,
    RGB_GRAY, RGB_LTRED, RGB_LTGREEN, RGB_LTYELLOW,
    RGB_LTBLUE, RGB_LTMAGENTA, RGB_LTCYAN, RGB_BLACK
};


void HostScreen_Init(void) {
	int i;
	for(i=0; i<256; i++) {
		unsigned long color = default_palette[i%16];
		palette.standard[i].r = color >> 24;
		palette.standard[i].g = (color >> 16) & 0xff;
		palette.standard[i].b = color & 0xff;
	}

	screenLock = SDL_CreateMutex();

	mainSurface=NULL;
}

void HostScreen_UnInit(void) {
	SDL_DestroyMutex(screenLock);
}


void HostScreen_toggleFullScreen(void)
{
	sdl_videoparams ^= SDL_FULLSCREEN;
	if(SDL_WM_ToggleFullScreen(mainSurface) == 0) {
		Dprintf(("toggleFullScreen: SDL_WM_ToggleFullScreen() not supported -> using SDL_SetVideoMode()"));

		// SDL_WM_ToggleFullScreen() did not work.
		// We have to change video mode "by hand".
		SDL_Surface *temp = SDL_ConvertSurface(mainSurface, mainSurface->format,
		                                       mainSurface->flags);
		if (temp == NULL)
			bug("toggleFullScreen: Unable to save screen content.");

#if 1
		HostScreen_setWindowSize(hs_width, hs_height, hs_bpp);
#else
		mainSurface = SDL_SetVideoMode(width, height, bpp, sdl_videoparams);
		if (mainSurface == NULL)
			bug("toggleFullScreen: Unable to set new video mode.");
		if (mainSurface->format->BitsPerPixel <= 8)
			SDL_SetColors(mainSurface, temp->format->palette->colors, 0,
			              temp->format->palette->ncolors);
#endif

		if (SDL_BlitSurface(temp, NULL, mainSurface, NULL) != 0)
			bug("toggleFullScreen: Unable to restore screen content.");
		SDL_FreeSurface(temp);

		/* refresh the screen */
		HostScreen_update1(TRUE);
	}
}


int HostScreen_selectVideoMode(SDL_Rect **modes, uint32 *width, uint32 *height)
{
	int i, bestw, besth;

	/* Search the smallest nearest mode */
	bestw = modes[0]->w;
	besth = modes[0]->h;
	for (i=0;modes[i]; ++i) {
		if ((modes[i]->w >= *width) && (modes[i]->h >= *height)) {
			if ((modes[i]->w < bestw) || (modes[i]->h < besth)) {
				bestw = modes[i]->w;
				besth = modes[i]->h;
			}			
		}
	}

	*width = bestw;
	*height = besth;
	Dprintf(("hostscreen: video mode found: %dx%d\n",*width,*height));

	return 1;
}

void HostScreen_searchVideoMode( uint32 *width, uint32 *height, uint32 *bpp )
{
	SDL_Rect **modes;
	SDL_PixelFormat pixelformat;
	int modeflags;

	/* Search in available modes the best suited */
	Dprintf(("hostscreen: video mode asked: %dx%dx%d\n",*width,*height,*bpp));

	if ((*width == 0) || (*height == 0)) {
		*width = 640;
		*height = 480;
	}

	/* Read available video modes */
	modeflags = 0 /*SDL_HWSURFACE | SDL_HWPALETTE*/;
	if (bInFullScreen)
		modeflags |= SDL_FULLSCREEN;

	/*--- Search a video mode with asked bpp ---*/
	if (*bpp != 0) {
		pixelformat.BitsPerPixel = *bpp;
		modes = SDL_ListModes(&pixelformat, modeflags);
		if ((modes != (SDL_Rect **) 0) && (modes != (SDL_Rect **) -1)) {
			Dprintf(("hostscreen: searching a good video mode (any bpp)\n"));
			if (HostScreen_selectVideoMode(modes,width,height)) {
				Dprintf(("hostscreen: video mode selected: %dx%dx%d\n",*width,*height,*bpp));
				return;
			}
		}
	}

	/*--- Search a video mode with any bpp ---*/
	modes = SDL_ListModes(NULL, modeflags);
	if ((modes != (SDL_Rect **) 0) && (modes != (SDL_Rect **) -1)) {
		Dprintf(("hostscreen: searching a good video mode\n"));
		if (HostScreen_selectVideoMode(modes,width,height)) {
			Dprintf(("hostscreen: video mode selected: %dx%dx%d\n",*width,*height,*bpp));
			return;
		}
	}

	if (modes == (SDL_Rect **) 0) {
		Dprintf(("hostscreen: No modes available\n"));
	}

	if (modes == (SDL_Rect **) -1) {
		/* Any mode available */
		Dprintf(("hostscreen: Any modes available\n"));
	}

	Dprintf(("hostscreen: video mode selected: %dx%dx%d\n",*width,*height,*bpp));
}

void HostScreen_setWindowSize( uint32 width, uint32 height, uint32 bpp )
{
	if (ConfigureParams.Screen.ChosenDisplayMode == DISPLAYMODE_LOWCOL_HIGHRES
	    || ConfigureParams.Screen.ChosenDisplayMode == DISPLAYMODE_HICOL_HIGHRES) {
		/* Ugly: 400x300 threshold is currently hard-coded. */
		/* Should rather be selectable by the user! */	
	    	if (width <= 400)
			width *= 2;
	    	if (height <= 400)
			height *= 2;
	}

	// Select a correct video mode
	HostScreen_searchVideoMode(&width, &height, &bpp);	

	hs_width	 = width;
	hs_height = height;
	hs_bpp = bpp;

	// SelectVideoMode();
	if (bInFullScreen)
		sdl_videoparams = SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN;
	else
		sdl_videoparams = SDL_SWSURFACE|SDL_HWPALETTE;

	mainSurface = SDL_SetVideoMode(width, height, bpp, sdl_videoparams);

	surf = mainSurface;

	// update the surface's palette
	HostScreen_updatePalette( 256 );

	Dprintf(("Surface Pitch = %d, width = %d, height = %d\n", surf->pitch, surf->w, surf->h));
	Dprintf(("Must Lock? %s\n", SDL_MUSTLOCK(surf) ? "YES" : "NO"));

	// is the SDL_update needed?
	doUpdate = ( surf->flags & SDL_HWSURFACE ) == 0;

	HostScreen_renderBegin();

//	VideoRAMBaseHost = (uint8 *) surf->pixels;
//	InitVMEMBaseDiff(VideoRAMBaseHost, VideoRAMBase);
//	Dprintf(("VideoRAM starts at %p (%08x)\n", VideoRAMBaseHost, VideoRAMBase));
	Dprintf(("surf->pixels = %p, getVideoSurface() = %p\n",
			surf->pixels, SDL_GetVideoSurface()->pixels));

	HostScreen_renderEnd();

	Dprintf(("Pixel format:bitspp=%d, tmasks r=%04x g=%04x b=%04x"
			", tshifts r=%d g=%d b=%d"
			", tlosses r=%d g=%d b=%d\n",
			surf->format->BitsPerPixel,
			surf->format->Rmask, surf->format->Gmask, surf->format->Bmask,
			surf->format->Rshift, surf->format->Gshift, surf->format->Bshift,
			surf->format->Rloss, surf->format->Gloss, surf->format->Bloss));
}


void HostScreen_update5( int32 x, int32 y, int32 w, int32 h, BOOL forced )
{
	if ( !forced && !doUpdate ) // the HW surface is available
		return;

	//	SDL_UpdateRect(SDL_GetVideoSurface(), 0, 0, width, height);
	// SDL_UpdateRect(surf, x, y, w, h);
	SDL_UpdateRect(mainSurface, x, y, w, h);
}

void HostScreen_update1( BOOL forced )
{
	HostScreen_update5( 0, 0, hs_width, hs_height, forced );
}

void HostScreen_update0()
{
	HostScreen_update5( 0, 0, hs_width, hs_height, FALSE );
}


uint32 HostScreen_getBitsPerPixel(void)
{
	return surf->format->BitsPerPixel;
}


#if 0
void HostScreen_gfxFastPixelColorNolock(int16 x, int16 y, uint32 color)
{
	int bpp;
	uint8 *p;

	/* Get destination format */
	bpp = surf->format->BytesPerPixel;
	p = (uint8 *)surf->pixels + y * surf->pitch + x * bpp;
	switch(bpp) {
		case 1:
			*p = color;
			break;
		case 2:
			*(uint16 *)p = color;
			break;
		case 3:
			putBpp24Pixel( p, color );
			break;
		case 4:
			*(uint32 *)p = color;
			break;
	} /* switch */
}
#endif

#if 0
uint32 HostScreen_gfxGetPixel( int16 x, int16 y )
{
	int bpp;
	uint8 *p;

	/* Get destination format */
	bpp = surf->format->BytesPerPixel;
	p = (uint8 *)surf->pixels + y * surf->pitch + x * bpp;
	switch(bpp) {
		case 1:
			return (uint32)(*(uint8 *)p);
		case 2:
			return (uint32)(*(uint16 *)p);
		case 3:
			// FIXME maybe some & problems? and endian
			return getBpp24Pixel( p );
		case 4:
			return *(uint32 *)p;
	} /* switch */
	return 0;	// should never happen
}
#endif

uint32 HostScreen_getBpp()
{
	return surf->format->BytesPerPixel;
}

uint32 HostScreen_getPitch() {
	return surf->pitch;
}

uint32 HostScreen_getWidth() {
	return hs_width;
}

uint32 HostScreen_getHeight() {
	return hs_height;
}

uint8 *HostScreen_getVideoramAddress() {
	return surf->pixels;	/* FIXME maybe this should be mainSurface? */
}

void HostScreen_setPaletteColor( uint8 idx, uint32 red, uint32 green, uint32 blue ) {
	// set the SDL standard RGB palette settings
	palette.standard[idx].r = red;
	palette.standard[idx].g = green;
	palette.standard[idx].b = blue;
	// convert the color to native
	palette.native[idx] = SDL_MapRGB( surf->format, red, green, blue );
}

uint32 HostScreen_getPaletteColor( uint8 idx ) {
	return palette.native[idx];
}

void HostScreen_updatePalette( uint16 colorCount ) {
	SDL_SetColors( surf, palette.standard, 0, colorCount );
}

uint32 HostScreen_getColor( uint32 red, uint32 green, uint32 blue ) {
	return SDL_MapRGB( surf->format, red, green, blue );
}

#if 0
void HostScreen_lock(void) {
	while (SDL_mutexP(screenLock)==-1) {
		SDL_Delay(20);
		fprintf(stderr, "Couldn't lock mutex\n");
	}
}

void HostScreen_unlock(void) {
	while (SDL_mutexV(screenLock)==-1) {
		SDL_Delay(20);
		fprintf(stderr, "Couldn't unlock mutex\n");
	}
}
#endif

BOOL HostScreen_renderBegin(void) {
	if (SDL_MUSTLOCK(surf))
		if (SDL_LockSurface(surf) < 0) {
			printf("Couldn't lock surface to refresh!\n");
			return FALSE;
		}

	return TRUE;
}

void HostScreen_renderEnd() {
	if (SDL_MUSTLOCK(surf))
		SDL_UnlockSurface(surf);
}



/**
 * Performs conversion from the TOS's bitplane word order (big endian) data
 * into the native chunky color index.
 */
void HostScreen_bitplaneToChunky( uint16 *atariBitplaneData, uint16 bpp, uint8 colorValues[16] )
{
	uint32 a, b, c, d, x;

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
		d = *(uint32 *)&atariBitplaneData[0];
		c = *(uint32 *)&atariBitplaneData[2];
		if (bpp == 4) {
			a = b = 0;
		} else {
			b = *(uint32 *)&atariBitplaneData[4];
			a = *(uint32 *)&atariBitplaneData[6];
		}
	} else {
		a = b = c = 0;
		if (bpp == 2) {
			d = *(uint32 *)&atariBitplaneData[0];
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
/*
  Hatari - dlgMain.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  The main dialog.
*/
const char DlgMain_fileid[] = "Hatari dlgMain.c : " __DATE__ " " __TIME__;

#include "main.h"
#include "configuration.h"
#include "dialog.h"
#include "sdlgui.h"
#include "screen.h"


#define MAINDLG_ABOUT    2
#define MAINDLG_SYSTEM   3
#define MAINDLG_ROM      4
#define MAINDLG_MEMORY   5
#define MAINDLG_FLOPPYS  6
#define MAINDLG_HARDDISK 7
#define MAINDLG_SCREEN   8
#define MAINDLG_SOUND    9
#define MAINDLG_JOY      10
#define MAINDLG_KEYBD    11
#define MAINDLG_DEVICES  12
#define MAINDLG_LOADCFG  13
#define MAINDLG_SAVECFG  14
#define MAINDLG_NORESET  15
#define MAINDLG_RESET    16
#define MAINDLG_OK       17
#define MAINDLG_QUIT     18
#define MAINDLG_CANCEL   19


/* The main dialog: */
static SGOBJ maindlg[] =
{
	{ SGBOX, 0, 0, 0,0, 50,19, NULL },
	{ SGTEXT, 0, 0, 17,1, 16,1, "Hatari main menu" },
	{ SGBUTTON, 0, 0, 2,4, 14,1, "About" },
	{ SGBUTTON, 0, 0, 2,6, 14,1, "System" },
	{ SGBUTTON, 0, 0, 2,8, 14,1, "ROM" },
	{ SGBUTTON, 0, 0, 2,10, 14,1, "Memory" },
	{ SGBUTTON, 0, 0, 18,4, 14,1, "Floppy disks" },
	{ SGBUTTON, 0, 0, 18,6, 14,1, "Hard disks" },
	{ SGBUTTON, 0, 0, 18,8, 14,1, "Screen" },
	{ SGBUTTON, 0, 0, 18,10, 14,1, "Sound" },
	{ SGBUTTON, 0, 0, 34,4, 14,1, "Joysticks" },
	{ SGBUTTON, 0, 0, 34,6, 14,1, "Keyboard" },
	{ SGBUTTON, 0, 0, 34,8, 14,1, "Devices" },
	{ SGBUTTON, 0, 0, 7,13, 16,1, "Load config." },
	{ SGBUTTON, 0, 0, 27,13, 16,1, "Save config." },
	{ SGRADIOBUT, 0, 0, 3,15, 10,1, "No Reset" },
	{ SGRADIOBUT, 0, 0, 3,17, 10,1, "Reset machine" },
	{ SGBUTTON, SG_DEFAULT, 0, 21,15, 8,3, "OK" },
	{ SGBUTTON, 0, 0, 36,15, 10,1, "Quit" },
	{ SGBUTTON, SG_CANCEL, 0, 36,17, 10,1, "Cancel" },
	{ -1, 0, 0, 0,0, 0,0, NULL }
};


/*-----------------------------------------------------------------------*/
/*
  This functions sets up the actual font and then displays the main dialog.
*/
int Dialog_MainDlg(bool *bReset, bool *bLoadedSnapshot)
{
	int retbut;
	bool bOldMouseVisibility;
	int nOldMouseX, nOldMouseY;

	*bReset = false;
	*bLoadedSnapshot = false;

	if (SDLGui_SetScreen(sdlscrn))
		return false;

	SDL_GetMouseState(&nOldMouseX, &nOldMouseY);
	bOldMouseVisibility = SDL_ShowCursor(SDL_QUERY);
	SDL_ShowCursor(SDL_ENABLE);

	SDLGui_CenterDlg(maindlg);

	maindlg[MAINDLG_NORESET].state |= SG_SELECTED;
	maindlg[MAINDLG_RESET].state &= ~SG_SELECTED;

	do
	{
		retbut = SDLGui_DoDialog(maindlg, NULL);
		switch (retbut)
		{
		 case MAINDLG_ABOUT:
			Dialog_AboutDlg();
			break;
		 case MAINDLG_FLOPPYS:
		 case MAINDLG_HARDDISK:
			Dialog_DiskDlg();
			break;
		 case MAINDLG_ROM:
			DlgRom_Main();
			break;
		 case MAINDLG_SCREEN:
			Dialog_ScreenDlg();
			break;
		 case MAINDLG_SOUND:
			Dialog_SoundDlg();
			break;
		 case MAINDLG_SYSTEM:
			Dialog_SystemDlg();
			break;
		 case MAINDLG_MEMORY:
			if (Dialog_MemDlg())
			{
				/* Memory snapshot has been loaded - leave GUI immediately */
				*bLoadedSnapshot = true;
				return true;
			}
			break;
		 case MAINDLG_JOY:
			Dialog_JoyDlg();
			break;
		 case MAINDLG_KEYBD:
			Dialog_KeyboardDlg();
			break;
		 case MAINDLG_DEVICES:
			Dialog_DeviceDlg();
			break;
		 case MAINDLG_LOADCFG:
			Configuration_Load(NULL);
			break;
		 case MAINDLG_SAVECFG:
			Configuration_Save();
			break;
		 case MAINDLG_QUIT:
			bQuitProgram = true;
			break;
		}
	}
	while (retbut != MAINDLG_OK && retbut != MAINDLG_CANCEL && retbut != SDLGUI_QUIT
	        && retbut != SDLGUI_ERROR && !bQuitProgram);


	if (maindlg[MAINDLG_RESET].state & SG_SELECTED)
		*bReset = true;

	SDL_ShowCursor(bOldMouseVisibility);
	Main_WarpMouse(nOldMouseX, nOldMouseY);

	return (retbut == MAINDLG_OK);
}

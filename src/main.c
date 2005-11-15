/*
  Hatari - main.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  Main initialization and event handling routines.
*/
char Main_rcsid[] = "Hatari $Id: main.c,v 1.81 2005-11-15 12:24:41 thothy Exp $";

#include <time.h>
#include <unistd.h>
#include <sched.h>      /* for sched_yield() */

#include <SDL.h>

#include "main.h"
#include "configuration.h"
#include "dialog.h"
#include "audio.h"
#include "joy.h"
#include "file.h"
#include "floppy.h"
#include "gemdos.h"
#include "hdc.h"
#include "ikbd.h"
#include "ioMem.h"
#include "keymap.h"
#include "log.h"
#include "m68000.h"
#include "memorySnapShot.h"
#include "misc.h"
#include "midi.h"
#include "printer.h"
#include "reset.h"
#include "rs232.h"
#include "screen.h"
#include "sdlgui.h"
#include "shortcut.h"
#include "sound.h"
#include "stMemory.h"
#include "tos.h"
#include "vdi.h"
#include "video.h"
#include "ymFormat.h"
#include "debugui.h"

#include "uae-cpu/hatari-glue.h"


BOOL bQuitProgram=FALSE;                  /* Flag to quit program cleanly */
BOOL bEmulationActive=TRUE;               /* Run emulation when started */
BOOL bEnableDebug=FALSE;                  /* Enable debug UI? */
static BOOL bAccurateDelays;              /* Host system has an accurate SDL_Delay()? */
static char szBootDiskImage[FILENAME_MAX];
char szWorkingDir[FILENAME_MAX];          /* Working directory */
BOOL bIgnoreNextMouseMotion = FALSE;      /* Next mouse motion will be ignored (needed after SDL_WarpMouse) */


/*-----------------------------------------------------------------------*/
/*
  Save/Restore snapshot of local variables('MemorySnapShot_Store' handles type)
*/
void Main_MemorySnapShot_Capture(BOOL bSave)
{
  int nBytes;

  /* Save/Restore details */
  /* Only save/restore area of memory machine ie set to, eg 1Mb */
  if (bSave) {
    nBytes = STRamEnd;
    MemorySnapShot_Store(&nBytes,sizeof(nBytes));
    MemorySnapShot_Store(STRam,nBytes);
  }
  else {
    MemorySnapShot_Store(&nBytes,sizeof(nBytes));
    MemorySnapShot_Store(STRam,nBytes);
  }
  /* And Cart/TOS/Hardware area */
  MemorySnapShot_Store(&STRam[0xE00000],0x200000);
  MemorySnapShot_Store(szBootDiskImage, sizeof(szBootDiskImage));
  MemorySnapShot_Store(szWorkingDir,sizeof(szWorkingDir));
}


/*-----------------------------------------------------------------------*/
/*
  Pause emulation, stop sound
*/
void Main_PauseEmulation(void)
{
  if( bEmulationActive )
  {
    Audio_EnableAudio(FALSE);
    bEmulationActive = FALSE;
  }
}

/*-----------------------------------------------------------------------*/
/*
  Start emulation
*/
void Main_UnPauseEmulation(void)
{
  if( !bEmulationActive )
  {
    Sound_ResetBufferIndex();
    Audio_EnableAudio(ConfigureParams.Sound.bEnableSound);
    Screen_SetFullUpdate();       /* Cause full screen update (to clear all) */

    bEmulationActive = TRUE;
  }
}


/*-----------------------------------------------------------------------*/
/*
  This function waits on each emulated VBL to synchronize the real time
  with the emulated ST.
  Unfortunately SDL_Delay and other sleep functions like usleep or nanosleep
  are very inaccurate on some systems like Linux 2.4 or Mac OS X (they can only
  wait for a multiple of 10ms due to the scheduler on these systems), so we have
  to "busy wait" there to get an accurate timing.
*/
void Main_WaitOnVbl(void)
{
  int nCurrentMilliTicks, nDestMilliTicks;
  static int nOldMilliTicks = 0;
  signed int nDelay;

  nCurrentMilliTicks = SDL_GetTicks();

  /* Only wait in normal speed mode */
  if (ConfigureParams.System.nMinMaxSpeed != MINMAXSPEED_MAX)
  {
    nDestMilliTicks = nOldMilliTicks + 1000/nScreenRefreshRate;
    nDelay = nDestMilliTicks - nCurrentMilliTicks;

    if (bAccurateDelays)
    {
      /* Accurate sleeping is possible -> use SDL_Delay to free the CPU */
      if (nDelay > 1)
      {
        SDL_Delay(nDelay - 1);
      }
    }
    else
    {
      /* No accurate SDL_Delay -> only wait if more than 10ms to go... */
      if (nDelay >= 10)
      {
        SDL_Delay(9);
      }
    }

    /* Now busy-wait for the right tick: */
    do
    {
      nCurrentMilliTicks = SDL_GetTicks();
      nDelay = nDestMilliTicks - nCurrentMilliTicks;

      if (nDelay > 3)
      {
        /* Avoid getting a bad priority -> call sched_yield... */
        sched_yield();
      }
    }
    while (nDelay > 0);
  }

  nOldMilliTicks = nCurrentMilliTicks;
}


/*-----------------------------------------------------------------------*/
/*
  Since SDL_Delay and friends are very inaccurate on some systems, we have
  to check if we can rely on this delay function.
*/
static void Main_CheckForAccurateDelays(void)
{
  int nStartTicks, nEndTicks;

  /* Force a task switch now, so we have a longer timeslice afterwards */
  SDL_Delay(10);

  nStartTicks = SDL_GetTicks();
  SDL_Delay(1);
  nEndTicks = SDL_GetTicks();

  /* If the delay took longer than 10ms, we are on an inaccurate system! */
  bAccurateDelays = ((nEndTicks - nStartTicks) < 9);

  if (bAccurateDelays)
    Log_Printf(LOG_DEBUG, "Host system has accurate delays. (%d)\n", nEndTicks - nStartTicks);
  else
    Log_Printf(LOG_DEBUG, "Host system does not have accurate delays. (%d)\n", nEndTicks - nStartTicks);
}


/* ----------------------------------------------------------------------- */
/*
  Set mouse pointer to new coordinates and set flag to ignore the mouse event
  that is generated by SDL_WarpMouse().
*/
void Main_WarpMouse(int x, int y)
{
  SDL_WarpMouse(x, y);                  /* Set mouse pointer to new position */
  bIgnoreNextMouseMotion = TRUE;        /* Ignore mouse motion event from SDL_WarpMouse */
}


/* ----------------------------------------------------------------------- */
/*
  Message handler
  Here we process the SDL events (keyboard, mouse, ...) and map it to
  Atari IKBD events.
*/
void Main_EventHandler(void)
{
  SDL_Event event;

  if( SDL_PollEvent(&event) )
   switch( event.type )
   {

    case SDL_QUIT:
       bQuitProgram = TRUE;
       set_special(SPCFLAG_BRK);        /* Assure that CPU core shuts down */
       break;

    case SDL_MOUSEMOTION:               /* Read/Update internal mouse position */
       if (bIgnoreNextMouseMotion)
       {
         bIgnoreNextMouseMotion = FALSE;
       }
       else
       {
         KeyboardProcessor.Mouse.dx += event.motion.xrel;
         KeyboardProcessor.Mouse.dy += event.motion.yrel;
       }
       break;

    case SDL_MOUSEBUTTONDOWN:
       if( event.button.button==SDL_BUTTON_LEFT )
       {
         if(Keyboard.LButtonDblClk==0)
           Keyboard.bLButtonDown |= BUTTON_MOUSE;  /* Set button down flag */
       }
       else if( event.button.button==SDL_BUTTON_RIGHT )
         Keyboard.bRButtonDown |= BUTTON_MOUSE;
       else if( event.button.button==SDL_BUTTON_MIDDLE )
         Keyboard.LButtonDblClk = 1;    /* Start double-click sequence in emulation time */
       break;

    case SDL_MOUSEBUTTONUP:
       if( event.button.button==SDL_BUTTON_LEFT )
         Keyboard.bLButtonDown &= ~BUTTON_MOUSE;
       else if( event.button.button==SDL_BUTTON_RIGHT )
         Keyboard.bRButtonDown &= ~BUTTON_MOUSE;;
       break;

    case SDL_KEYDOWN:
       Keymap_KeyDown(&event.key.keysym);
       break;

    case SDL_KEYUP:
       Keymap_KeyUp(&event.key.keysym);
       break;
   }
}


/*-----------------------------------------------------------------------*/
/*
  Show supported options.
*/
static void Main_ShowOptions(void)
{
  printf("Usage:\n hatari [options] [disk image name]\n"
         "Where options are:\n"
         "  --help or -h          Print this help text and exit.\n"
         "  --version or -v       Print version number and exit.\n"
         "  --mono or -m          Start in monochrome mode instead of color.\n"
         "  --fullscreen or -f    Start emulator in fullscreen mode.\n"
         "  --window or -w        Start emulator in window mode.\n"
         "  --joystick or -j      Emulate a ST joystick with the cursor keys.\n"
         "  --nosound             Disable sound (faster!).\n"
         "  --printer             Enable printer support (experimental).\n"
         "  --midi <filename>     Enable midi support and write midi data to <filename>.\n"
         "  --rs232 <filename>    Use <filename> as the serial port device.\n"
         "  --frameskip           Skip every second frame (speeds up emulation!).\n"
         "  --debug or -D         Allow debug interface.\n"
         "  --harddrive <dir>     Emulate an ST harddrive\n"
         "     or -d <dir>         (<dir> = root directory).\n"
         "  --hdimage <imagename> Emulate an ST harddrive with an image.\n"
         "  --tos <file>          Use TOS image <file>.\n"
         "  --cartridge <file>    Use ROM cartridge image <file>.\n"
         "  --cpulevel <x>        Set the CPU type (x => 680x0) (TOS 2.06 only!).\n"
         "  --compatible          Use a more compatible (but slower) 68000 CPU mode.\n"
         "  --blitter             Enable blitter emulation (unstable!)\n"
         "  --vdi                 Use extended VDI resolution\n"
         "  --memsize <x>         ST RAM size. x = size in MiB from 0 to 14; 0 for 512KiB\n"
         "  --configfile <file>   Use <file> instead of ~/.hatari.cfg as configuration\n"
         "     or -c <file>        file.\n"
         "  --slowfdc             Slow down FDC emulation (very experimental!).\n"
         "  --machine <x>         Select machine type (x = st or ste)\n"
        );
}


/*-----------------------------------------------------------------------*/
/*
  Check for any passed parameters
*/
static void Main_ReadParameters(int argc, char *argv[])
{
  int i;

  /* Scan for any which we can use */
  for(i=1; i<argc; i++)
  {
    if (strlen(argv[i])>0)
    {
      if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h"))
      {
        Main_ShowOptions();
        exit(0);
      }
      else if (!strcmp(argv[i],"--version") || !strcmp(argv[i],"-v"))
      {
        printf("This is %s.\n", PROG_NAME);
        printf("This program is free software licensed under the GNU GPL.\n");
        exit(0);
      }
      else if (!strcmp(argv[i],"--mono") || !strcmp(argv[i],"-m"))
      {
        bUseHighRes=TRUE;
        ConfigureParams.Screen.bUseHighRes=TRUE;
        STRes=PrevSTRes=ST_HIGH_RES;
      }
      else if (!strcmp(argv[i],"--fullscreen") || !strcmp(argv[i],"-f"))
      {
        ConfigureParams.Screen.bFullScreen = TRUE;
      }
      else if (!strcmp(argv[i],"--window") || !strcmp(argv[i],"-w"))
      {
        ConfigureParams.Screen.bFullScreen = FALSE;
      }
      else if (!strcmp(argv[i],"--joystick") || !strcmp(argv[i],"-j"))
      {
        ConfigureParams.Joysticks.Joy[1].nJoystickMode = JOYSTICK_KEYBOARD;
      }
      else if ( !strcmp(argv[i],"--nosound") )
      {
        ConfigureParams.Sound.bEnableSound = FALSE;
      }
      else if ( !strcmp(argv[i],"--frameskip") )
      {
        ConfigureParams.Screen.bFrameSkip = TRUE;
      }
      else if ( !strcmp(argv[i],"--printer") )
      {
        /* FIXME: add more commandline configuration for printing */
        ConfigureParams.Printer.bEnablePrinting = TRUE;
      }
      else if (!strcmp(argv[i], "--midi"))
      {
        if(i+1 >= argc)
          fprintf(stderr, "Missing argument for --midi\n");
        else
        {
          if (strlen(argv[i+1]) <= sizeof(ConfigureParams.Midi.szMidiOutFileName))
          {
            ConfigureParams.Midi.bEnableMidi = TRUE;
            strcpy(ConfigureParams.Midi.szMidiOutFileName, argv[i+1]);
          }
          else fprintf(stderr, "Midi file name too long!\n");
          i += 1;
        }
      }
      else if (!strcmp(argv[i], "--rs232"))
      {
        if(i+1 >= argc)
          fprintf(stderr, "Missing argument for --rs232\n");
        else
        {
          if (strlen(argv[i+1]) <= sizeof(ConfigureParams.RS232.szOutFileName))
          {
            ConfigureParams.RS232.bEnableRS232 = TRUE;
            strcpy(ConfigureParams.RS232.szOutFileName, argv[i+1]);
            strcpy(ConfigureParams.RS232.szInFileName, argv[i+1]);
          }
          else fprintf(stderr, "RS232 file name too long!\n");
          i += 1;
        }
      }
      else if (!strcmp(argv[i],"--debug") || !strcmp(argv[i],"-D"))
      {
        bEnableDebug=TRUE;
      }
      else if (!strcmp(argv[i],"--hdimage"))
      {
        if(i+1 >= argc)
          fprintf(stderr, "Missing argument for --hdimage\n");
        else
        {
          if (strlen(argv[i+1]) <= sizeof(ConfigureParams.HardDisk.szHardDiskImage))
          {
            ConfigureParams.HardDisk.bUseHardDiskImage = TRUE;
            strcpy(ConfigureParams.HardDisk.szHardDiskImage, argv[i+1]);
          }
          else fprintf(stderr, "HD image file name too long!\n");
          i += 1;
        }
      }
      else if (!strcmp(argv[i],"--harddrive") || !strcmp(argv[i],"-d"))
      {
        if(i+1 >= argc)
          fprintf(stderr, "Missing argument for --harddrive\n");
        else
        {
          if(strlen(argv[i+1]) <= MAX_PATH )
          {
            ConfigureParams.HardDisk.bUseHardDiskDirectories = TRUE;
            ConfigureParams.HardDisk.bBootFromHardDisk = TRUE;
            strcpy(ConfigureParams.HardDisk.szHardDiskDirectories[0], argv[i+1]);
          }
          else fprintf(stderr, "HD directory name too long!\n");
          i += 1;
        }
      }
      else if (!strcmp(argv[i],"--tos"))
      {
        if(i+1>=argc)
          fprintf(stderr,"Missing argument for --tos.\n");
        else
          strncpy(ConfigureParams.Rom.szTosImageFileName, argv[++i], sizeof(ConfigureParams.Rom.szTosImageFileName));
      }
      else if (!strcmp(argv[i],"--cartridge"))
      {
        if(i+1>=argc)
          fprintf(stderr,"Missing argument for --cartridge.\n");
        else
          strncpy(ConfigureParams.Rom.szCartridgeImageFileName, argv[++i], sizeof(ConfigureParams.Rom.szCartridgeImageFileName));
      }
      else if (!strcmp(argv[i],"--cpulevel"))
      {
        if(i+1>=argc)
          fprintf(stderr,"Missing argument for --cpulevel.\n");
         else
          cpu_level = atoi(argv[++i]);
        if(cpu_level<0 || cpu_level>4)
          cpu_level = 0;
        ConfigureParams.System.nCpuLevel = cpu_level;
      }
      else if (!strcmp(argv[i],"--compatible") || !strcmp(argv[i],"-d"))
      {
        cpu_compatible = TRUE;
        ConfigureParams.System.bCompatibleCpu = TRUE;
      }
      else if (!strcmp(argv[i],"--blitter"))
      {
        ConfigureParams.System.bBlitter = TRUE;
      }
      else if (!strcmp(argv[i], "--vdi"))
      {
        bUseVDIRes = ConfigureParams.Screen.bUseExtVdiResolutions = TRUE;
      }
      else if (!strcmp(argv[i], "--memsize"))
      {
        if(i+1 >= argc)
          fprintf(stderr,"Missing argument for --memsize.\n");
        else
          ConfigureParams.Memory.nMemorySize = atoi(argv[++i]);
        if (ConfigureParams.Memory.nMemorySize < 0 || ConfigureParams.Memory.nMemorySize > 14)
        {
          fprintf(stderr, "Memory size must be between 0 and 14 MB.\n");
          ConfigureParams.Memory.nMemorySize = 1;
        }
      }
      else if (!strcmp(argv[i],"--configfile") || !strcmp(argv[i],"-c"))
      {
        if (i+1 >= argc)
          fprintf(stderr, "Missing argument for --configfile\n");
        else
        {
          if (strlen(argv[i+1]) <= sizeof(sConfigFileName))
          {
            strcpy(sConfigFileName, argv[i+1]);
            Configuration_Load(NULL);
          }
          else
            fprintf(stderr, "Config file name too long!\n");
          i += 1;
        }
      }
      else if (!strcmp(argv[i], "--slowfdc"))
      {
        ConfigureParams.System.bSlowFDC = TRUE;
      }
      else if (!strcmp(argv[i],"--machine"))
      {
        if (i+1 >= argc)
          fprintf(stderr, "Missing argument for --machine\n");
        else
        {
          if (strcasecmp(argv[i+1], "st") == 0)
          {
            ConfigureParams.System.nMachineType = MACHINE_ST;
          }
          else if (strcasecmp(argv[i+1], "ste") == 0)
          {
            ConfigureParams.System.nMachineType = MACHINE_STE;
          }
          else if (strcasecmp(argv[i+1], "tt") == 0)
          {
            ConfigureParams.System.nMachineType = MACHINE_TT;
            ConfigureParams.System.nCpuLevel = cpu_level = 3;
            ConfigureParams.System.nCpuFreq = 32;
          }
          else
          {
            fprintf(stderr, "Unknown machine type: %s\n", argv[i+1]);
          }
          i += 1;
        }
      }
      else
      {
        /* Possible passed disk image filename, ie starts with character other than '-' */
        if (argv[i][0] != '-' && strlen(argv[i]) < sizeof(szBootDiskImage)
            && File_Exists(argv[i]))
        {
          strcpy(szBootDiskImage, argv[i]);
          File_MakeAbsoluteName(szBootDiskImage);
        }
        else
          fprintf(stderr,"Illegal parameter: %s\n", argv[i]);
      }
    }
  }

  Configuration_WorkOnDetails(FALSE);
}


/*-----------------------------------------------------------------------*/
/*
  Initialise emulation
*/
static void Main_Init(void)
{
  /* Open debug log file */
  Log_Init();
  Log_Printf(LOG_INFO, PROG_NAME ", compiled on:  " __DATE__ ", " __TIME__ "\n");

  /* Init SDL's video subsystem. Note: Audio and joystick subsystems
     will be initialized later (failures there are not fatal). */
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr, "Could not initialize the SDL library:\n %s\n", SDL_GetError() );
    exit(-1);
  }

  Misc_SeedRandom(1043618);
  SDLGui_Init();
  Printer_Init();
  RS232_Init();
  Midi_Init();
  Screen_Init();
  Floppy_Init();
  Init680x0();                  /* Init CPU emulation */
  Audio_Init();
  Keymap_Init();

  /* Init HD emulation */
  if (ConfigureParams.HardDisk.bUseHardDiskImage)
  {
    char *szHardDiskImage = ConfigureParams.HardDisk.szHardDiskImage;
    if (HDC_Init(szHardDiskImage))
      printf("Hard drive image %s mounted.\n", szHardDiskImage);
    else
      printf("Couldn't open HD file: %s, or no partitions\n", szHardDiskImage);
  }
  GemDOS_Init();
  if(ConfigureParams.HardDisk.bUseHardDiskDirectories)
  {
    GemDOS_InitDrives();
  }

  if(Reset_Cold())              /* Reset all systems, load TOS image */
  {
    /* If loading of the TOS failed, we bring up the GUI to let the
     * user choose another TOS ROM file. */
    Dialog_DoProperty();
  }
  if(!bTosImageLoaded || bQuitProgram)
  {
    fprintf(stderr, "Failed to load TOS image!\n");
    SDL_Quit();
    exit(-2);
  }

  IoMem_Init();
  Joy_Init();
  Sound_Init();

  /* Check passed disk image parameter, boot directly into emulator */
  if (strlen(szBootDiskImage) > 0)
  {
    Floppy_InsertDiskIntoDrive(0, szBootDiskImage);
  }
}


/*-----------------------------------------------------------------------*/
/*
  Un-Initialise emulation
*/
static void Main_UnInit(void)
{
  Screen_ReturnFromFullScreen();
  Floppy_UnInit();
  HDC_UnInit();
  Midi_UnInit();
  RS232_UnInit();
  Printer_UnInit();
  IoMem_UnInit();
  GemDOS_UnInitDrives();
  if(Sound_AreWeRecording())
    Sound_EndRecording();
  Audio_UnInit();
  YMFormat_FreeRecording();
  SDLGui_UnInit();
  Screen_UnInit();
  Exit680x0();

  /* SDL uninit: */
  SDL_Quit();

  /* Close debug log file */
  Log_UnInit();
}


/*-----------------------------------------------------------------------*/
/*
  Main
*/
int main(int argc, char *argv[])
{
  /* Generate random seed */
  srand(time(NULL));

  /* Get working directory */
  getcwd(szWorkingDir, FILENAME_MAX);

  szBootDiskImage[0] = 0;

  /* Set default configuration values: */
  Configuration_SetDefault();

  /* Now load the values from the configuration file */
  Configuration_Load(CONFDIR"/hatari.cfg");     /* Try the global configuration file first */
  Configuration_Load(NULL);                     /* Now try the users configuration file */

  /* Check for any passed parameters */
  Main_ReadParameters(argc, argv);

  /* Init emulator system */
  Main_Init();

  /* Check if SDL_Delay is accurate */
  Main_CheckForAccurateDelays();

  /* Switch immediately to fullscreen if user wants to */
  if (ConfigureParams.Screen.bFullScreen)
    Screen_EnterFullScreen();

  /* Run emulation */
  Main_UnPauseEmulation();
  Start680x0();                 /* Start emulation */

  /* Un-init emulation system */
  Main_UnInit();

  return 0;
}

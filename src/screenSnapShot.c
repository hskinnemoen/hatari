/*
  Hatari

  Screen Snapshot
*/

#include <SDL.h>
#include <dirent.h>
#include <string.h>

#include "main.h"
#include "misc.h"
#include "screen.h"
#include "screenSnapShot.h"
#include "statusBar.h"
#include "video.h"
#include "vdi.h"

//#define ALLOW_SCREEN_GRABS  /* FIXME */

extern SDL_Surface *sdlscrn;

int nScreenShots=0;                  /* Number of screen shots saved */
BOOL bRecordingAnimation=FALSE;      /* Recording animation? */
BOOL bGrabWhenChange;
int GrabFrameCounter,GrabFrameLatch;

/*-----------------------------------------------------------------------*/
/*
  Scan working directory to get the screenshot number
*/
void ScreenSnapShot_GetNum(void){
  char dummy[5];
  int i, num;
  DIR *workingdir = opendir(szWorkingDir);
  struct dirent *file;

  nScreenShots = 0;
  if(workingdir == NULL)return;

  file = readdir(workingdir);
  while(file != NULL){
    /* copy first 4 letters */
    for(i=0;i<4;i++)
      if(file->d_name[i])
        dummy[i] = file->d_name[i]; 
    dummy[i] = '\0'; /* null terminate */
    if(strcmp("grab", dummy) == 0){
      /* copy next 4 numbers */
      for(i=0;i<4;i++)
        if(file->d_name[4+i] >= '0' && file->d_name[4+i] <= '9')
          dummy[i] = file->d_name[4+i]; 
        else break;

      dummy[i] = '\0'; /* null terminate */
      sscanf(dummy,"%i", &num);
      if(num > nScreenShots)nScreenShots = num;
    }
    /* next file.. */
  file = readdir(workingdir);
  } 
}


/*-----------------------------------------------------------------------*/
/* REDUNDANT.
  Check if we have pressed PrintScreen
*/
void ScreenSnapShot_CheckPrintKey(void)
{
#ifdef ALLOW_SCREEN_GRABS
  /* Did press Print Screen key? */
  if (GetAsyncKeyState(VK_SNAPSHOT)&0x0001) {  /* Print Key pressed(not held) */
    /* Save our screen */
    ScreenSnapShot_SaveScreen();
  }
#endif  /*ALLOW_SCREEN_GRABS*/
}

/*-----------------------------------------------------------------------*/
/*
  Save screen shot out .BMP file with filename 'grab0000.bmp','grab0001.bmp'....
*/
void ScreenSnapShot_SaveScreen(void)
{
  char szFileName[MAX_FILENAME_LENGTH];
  
  ScreenSnapShot_GetNum();
  /* Only do when NOT in full screen and NOT VDI resolution */
  if (!bInFullScreen && !bUseVDIRes) {
    /* Create our filename */
    nScreenShots++;
    sprintf(szFileName,"%s/grab%4.4d.bmp",szWorkingDir,nScreenShots);
    if(SDL_SaveBMP(sdlscrn, szFileName))
      fprintf(stderr, "Screen dump failed!\n");
    else 
      fprintf(stderr, "Screen dump saved to: %s\n", szFileName);    
  }
}

/*-----------------------------------------------------------------------*/
/*
  Are we recording an animation?
*/
BOOL ScreenSnapShot_AreWeRecording(void)
{
  return(bRecordingAnimation);
}

/*-----------------------------------------------------------------------*/
/*
  Start recording animation
*/
void ScreenSnapShot_BeginRecording(BOOL bCaptureChange, int nFramesPerSecond)
{
  /* Set in globals */
  bGrabWhenChange = bCaptureChange;
  /* Set animation timer rate */
  GrabFrameCounter = 0;
  GrabFrameLatch = (int)(50.0f/(float)nFramesPerSecond);
  /* Start animation */
  bRecordingAnimation = TRUE;
  /* Set status bar */
  StatusBar_SetIcon(STATUS_ICON_SCREEN,ICONSTATE_ON);
  /* And inform user */
  Main_Message("Screen-Shot recording started.",PROG_NAME /*,MB_OK|MB_ICONINFORMATION*/);
}

/*-----------------------------------------------------------------------*/
/*
  Stop recording animation
*/
void ScreenSnapShot_EndRecording()
{
  /* Were we recording? */
  if (bRecordingAnimation) {
    /* Stop animation */
    bRecordingAnimation = FALSE;
    /* Turn off icon */
    StatusBar_SetIcon(STATUS_ICON_SCREEN,ICONSTATE_OFF);
    /* And inform user */
    Main_Message("Screen-Shot recording stopped.",PROG_NAME /*,MB_OK|MB_ICONINFORMATION*/);
  }
}

/*-----------------------------------------------------------------------*/
/*
  Recording animation frame
*/
void ScreenSnapShot_RecordFrame(BOOL bFrameChanged)
{
  /* As we recording? And running in a Window */
  if (bRecordingAnimation && !bInFullScreen) {
    /* Yes, but on a change basis or a timer? */
    if (bGrabWhenChange) {
      /* On change, so did change this frame? */
      if (bFrameChanged)
        ScreenSnapShot_SaveScreen();
    }
    else {
      /* On timer, check for latch and save */
      GrabFrameCounter++;
      if (GrabFrameCounter>=GrabFrameLatch) {
        ScreenSnapShot_SaveScreen();
        GrabFrameCounter = 0;
      }
    }
  }
}

/*
  Hatari - sdlgui.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  A tiny graphical user interface for Hatari.
*/
static char rcsid[] = "Hatari $Id: sdlgui.c,v 1.12 2003-04-12 16:31:01 thothy Exp $";

#include <SDL.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "main.h"
#include "memAlloc.h"
#include "screen.h"
#include "sdlgui.h"
#include "file.h"
#include "zip.h"

#define SGRADIOBUTTON_NORMAL    12
#define SGRADIOBUTTON_SELECTED  13
#define SGCHECKBOX_NORMAL    14
#define SGCHECKBOX_SELECTED  15
#define SGARROWUP    1
#define SGARROWDOWN  2
#define SGFOLDER     5



static SDL_Surface *stdfontgfx = NULL;  /* The 'standard' font graphics */
static SDL_Surface *fontgfx = NULL;     /* The actual font graphics */
static int fontwidth, fontheight;       /* Height and width of the actual font */



/*-----------------------------------------------------------------------*/
/*
  Initialize the GUI.
*/
int SDLGui_Init()
{
  char fontname[256];
  sprintf(fontname, "%s/font8.bmp", DATADIR);

  /* Load the font graphics: */
  stdfontgfx = SDL_LoadBMP(fontname);
  if( stdfontgfx==NULL )
  {
    fprintf(stderr, "Error: Can't load image %s: %s\n", fontname, SDL_GetError() );
    return -1;
  }

  return 0;
}


/*-----------------------------------------------------------------------*/
/*
  Uninitialize the GUI.
*/
int SDLGui_UnInit()
{
  if(stdfontgfx)
  {
    SDL_FreeSurface(stdfontgfx);
    stdfontgfx = NULL;
  }
  if(fontgfx)
  {
    SDL_FreeSurface(fontgfx);
    fontgfx = NULL;
  }

  return 0;
}


/*-----------------------------------------------------------------------*/
/*
  Prepare the font to suit the actual resolution.
*/
int SDLGui_PrepareFont()
{
/* FIXME: Freeing the old font gfx does sometimes crash with a SEGFAULT
  if(fontgfx)
  {
    SDL_FreeSurface(fontgfx);
    fontgfx = NULL;
  }
*/

  if( stdfontgfx==NULL )
  {
    fprintf(stderr, "Error: The font has not been loaded!\n");
    return -1;
  }

  /* Convert the font graphics to the actual screen format */
  fontgfx = SDL_DisplayFormat(stdfontgfx);
  if( fontgfx==NULL )
  {
    fprintf(stderr, "Could not convert font:\n %s\n", SDL_GetError() );
    return -1;
  }
  /* Set transparent pixel */
  SDL_SetColorKey(fontgfx, (SDL_SRCCOLORKEY|SDL_RLEACCEL), SDL_MapRGB(fontgfx->format,255,255,255));
  /* Get the font width and height: */
  fontwidth = fontgfx->w/16;
  fontheight = fontgfx->h/16;

  return 0;
}


/*-----------------------------------------------------------------------*/
/*
  Center a dialog so that it appears in the middle of the screen.
  Note: We only store the coordinates in the root box of the dialog,
  all other objects in the dialog are positioned relatively to this one.
*/
void SDLGui_CenterDlg(SGOBJ *dlg)
{
  dlg[0].x = (sdlscrn->w/fontwidth-dlg[0].w)/2;
  dlg[0].y = (sdlscrn->h/fontheight-dlg[0].h)/2;
}


/*-----------------------------------------------------------------------*/
/*
  Draw a text string.
*/
void SDLGui_Text(int x, int y, const char *txt)
{
  int i;
  char c;
  SDL_Rect sr, dr;

  for(i=0; txt[i]!=0; i++)
  {
    c = txt[i];
    sr.x=fontwidth*(c%16);  sr.y=fontheight*(c/16);
    sr.w=fontwidth;         sr.h=fontheight;
    dr.x=x+i*fontwidth;     dr.y=y;
    dr.w=fontwidth;         dr.h=fontheight;
    SDL_BlitSurface(fontgfx, &sr, sdlscrn, &dr);
  }
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog text object.
*/
void SDLGui_DrawText(SGOBJ *tdlg, int objnum)
{
  int x, y;
  x = (tdlg[0].x+tdlg[objnum].x)*fontwidth;
  y = (tdlg[0].y+tdlg[objnum].y)*fontheight;
  SDLGui_Text(x, y, tdlg[objnum].txt);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog box object.
*/
void SDLGui_DrawBox(SGOBJ *bdlg, int objnum)
{
  SDL_Rect rect;
  int x, y, w, h;
  Uint32 grey = SDL_MapRGB(sdlscrn->format,192,192,192);
  Uint32 upleftc, downrightc;

  x = bdlg[objnum].x*fontwidth;
  y = bdlg[objnum].y*fontheight;
  if(objnum>0)                    /* Since the root object is a box, too, */
  {                               /* we have to look for it now here and only */
    x += bdlg[0].x*fontwidth;     /* add its absolute coordinates if we need to */
    y += bdlg[0].y*fontheight;
  }
  w = bdlg[objnum].w*fontwidth;
  h = bdlg[objnum].h*fontheight;

  if( bdlg[objnum].state&SG_SELECTED )
  {
    upleftc = SDL_MapRGB(sdlscrn->format,128,128,128);
    downrightc = SDL_MapRGB(sdlscrn->format,255,255,255);
  }
  else
  {
    upleftc = SDL_MapRGB(sdlscrn->format,255,255,255);
    downrightc = SDL_MapRGB(sdlscrn->format,128,128,128);
  }

  rect.x = x;  rect.y = y;
  rect.w = w;  rect.h = h;
  SDL_FillRect(sdlscrn, &rect, grey);

  rect.x = x;  rect.y = y-1;
  rect.w = w;  rect.h = 1;
  SDL_FillRect(sdlscrn, &rect, upleftc);

  rect.x = x-1;  rect.y = y;
  rect.w = 1;  rect.h = h;
  SDL_FillRect(sdlscrn, &rect, upleftc);

  rect.x = x;  rect.y = y+h;
  rect.w = w;  rect.h = 1;
  SDL_FillRect(sdlscrn, &rect, downrightc);

  rect.x = x+w;  rect.y = y;
  rect.w = 1;  rect.h = h;
  SDL_FillRect(sdlscrn, &rect, downrightc);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a normal button.
*/
void SDLGui_DrawButton(SGOBJ *bdlg, int objnum)
{
  int x,y;

  SDLGui_DrawBox(bdlg, objnum);

  x = (bdlg[0].x+bdlg[objnum].x+(bdlg[objnum].w-strlen(bdlg[objnum].txt))/2)*fontwidth;
  y = (bdlg[0].y+bdlg[objnum].y+(bdlg[objnum].h-1)/2)*fontheight;

  if( bdlg[objnum].state&SG_SELECTED )
  {
    x+=1;
    y+=1;
  }
  SDLGui_Text(x, y, bdlg[objnum].txt);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog radio button object.
*/
void SDLGui_DrawRadioButton(SGOBJ *rdlg, int objnum)
{
  char str[80];
  int x, y;

  x = (rdlg[0].x+rdlg[objnum].x)*fontwidth;
  y = (rdlg[0].y+rdlg[objnum].y)*fontheight;

  if( rdlg[objnum].state&SG_SELECTED )
    str[0]=SGRADIOBUTTON_SELECTED;
   else
    str[0]=SGRADIOBUTTON_NORMAL;
  str[1]=' ';
  strcpy(&str[2], rdlg[objnum].txt);

  SDLGui_Text(x, y, str);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog check box object.
*/
void SDLGui_DrawCheckBox(SGOBJ *cdlg, int objnum)
{
  char str[80];
  int x, y;

  x = (cdlg[0].x+cdlg[objnum].x)*fontwidth;
  y = (cdlg[0].y+cdlg[objnum].y)*fontheight;

  if( cdlg[objnum].state&SG_SELECTED )
    str[0]=SGCHECKBOX_SELECTED;
   else
    str[0]=SGCHECKBOX_NORMAL;
  str[1]=' ';
  strcpy(&str[2], cdlg[objnum].txt);

  SDLGui_Text(x, y, str);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a dialog popup button object.
*/
void SDLGui_DrawPopupButton(SGOBJ *pdlg, int objnum)
{
  int x, y, w, h;
  const char *downstr = "\x02";

  SDLGui_DrawBox(pdlg, objnum);

  x = (pdlg[0].x+pdlg[objnum].x)*fontwidth;
  y = (pdlg[0].y+pdlg[objnum].y)*fontheight;
  w = pdlg[objnum].w*fontwidth;
  h = pdlg[objnum].h*fontheight;

  SDLGui_Text(x, y, pdlg[objnum].txt);
  SDLGui_Text(x+w-fontwidth, y, downstr);
}


/*-----------------------------------------------------------------------*/
/*
  Draw a whole dialog.
*/
void SDLGui_DrawDialog(SGOBJ *dlg)
{
  int i;
  for(i=0; dlg[i].type!=-1; i++ )
  {
    switch( dlg[i].type )
    {
      case SGBOX:
        SDLGui_DrawBox(dlg, i);
        break;
      case SGTEXT:
        SDLGui_DrawText(dlg, i);
        break;
      case SGBUTTON:
        SDLGui_DrawButton(dlg, i);
        break;
      case SGRADIOBUT:
        SDLGui_DrawRadioButton(dlg, i);
        break;
      case SGCHECKBOX:
        SDLGui_DrawCheckBox(dlg, i);
        break;
      case SGPOPUP:
        SDLGui_DrawPopupButton(dlg, i);
        break;
    }
  }
  SDL_UpdateRect(sdlscrn, 0,0,0,0);
}


/*-----------------------------------------------------------------------*/
/*
  Search an object at a certain position.
*/
int SDLGui_FindObj(SGOBJ *dlg, int fx, int fy)
{
  int len, i;
  int ob = -1;
  int xpos, ypos;

  len = 0;
  while( dlg[len].type!=-1)   len++;

  xpos = fx/fontwidth;
  ypos = fy/fontheight;
  /* Now search for the object: */
  for(i=len; i>0; i--)
  {
    if(xpos>=dlg[0].x+dlg[i].x && ypos>=dlg[0].y+dlg[i].y
       && xpos<dlg[0].x+dlg[i].x+dlg[i].w && ypos<dlg[0].y+dlg[i].y+dlg[i].h)
    {
      ob = i;
      break;
    }
  }

  return ob;
}


/*-----------------------------------------------------------------------*/
/*
  Show and process a dialog. Returns the button number that has been
  pressed or -1 if something went wrong.
*/
int SDLGui_DoDialog(SGOBJ *dlg)
{
  int obj=0;
  int oldbutton=0;
  int retbutton=0;
  int i, j, b;
  SDL_Event evnt;
  SDL_Rect rct;
  Uint32 grey;

  grey = SDL_MapRGB(sdlscrn->format,192,192,192);

  SDLGui_DrawDialog(dlg);

  /* Is the left mouse button still pressed? Yes -> Handle TOUCHEXIT objects here */
  SDL_PumpEvents();
  b = SDL_GetMouseState(&i, &j);
  obj = SDLGui_FindObj(dlg, i, j);
  if(obj>0 && (dlg[obj].flags&SG_TOUCHEXIT) )
  {
    oldbutton = obj;
    if( b&SDL_BUTTON(1) )
    {
      dlg[obj].state |= SG_SELECTED;
      return obj;
    }
  }

  /* The main loop */
  do
  {
    if( SDL_WaitEvent(&evnt)==1 )  /* Wait for events */
      switch(evnt.type)
      {
        case SDL_KEYDOWN:
          break;
        case SDL_QUIT:
          bQuitProgram = TRUE;
          break;
        case SDL_MOUSEBUTTONDOWN:
          obj = SDLGui_FindObj(dlg, evnt.button.x, evnt.button.y);
          if(obj>0)
          {
            if(dlg[obj].type==SGBUTTON)
            {
              dlg[obj].state |= SG_SELECTED;
              SDLGui_DrawButton(dlg, obj);
              SDL_UpdateRect(sdlscrn, (dlg[0].x+dlg[obj].x)*fontwidth-2, (dlg[0].y+dlg[obj].y)*fontheight-2,
                             dlg[obj].w*fontwidth+4, dlg[obj].h*fontheight+4);
              oldbutton=obj;
            }
            if( dlg[obj].flags&SG_TOUCHEXIT )
            {
              dlg[obj].state |= SG_SELECTED;
              retbutton = obj;
            }
          }
          break;
        case SDL_MOUSEBUTTONUP:
          obj = SDLGui_FindObj(dlg, evnt.button.x, evnt.button.y);
          if(obj>0)
          {
            switch(dlg[obj].type)
            {
              case SGBUTTON:
                if(oldbutton==obj)
                  retbutton=obj;
                break;
              case SGRADIOBUT:
                for(i=obj-1; i>0 && dlg[i].type==SGRADIOBUT; i--)
                {
                  dlg[i].state &= ~SG_SELECTED;  /* Deselect all radio buttons in this group */
                  rct.x = (dlg[0].x+dlg[i].x)*fontwidth;
                  rct.y = (dlg[0].y+dlg[i].y)*fontheight;
                  rct.w = fontwidth;  rct.h = fontheight;
                  SDL_FillRect(sdlscrn, &rct, grey); /* Clear old */
                  SDLGui_DrawRadioButton(dlg, i);
                  SDL_UpdateRects(sdlscrn, 1, &rct);
                }
                for(i=obj+1; dlg[i].type==SGRADIOBUT; i++)
                {
                  dlg[i].state &= ~SG_SELECTED;  /* Deselect all radio buttons in this group */
                  rct.x = (dlg[0].x+dlg[i].x)*fontwidth;
                  rct.y = (dlg[0].y+dlg[i].y)*fontheight;
                  rct.w = fontwidth;  rct.h = fontheight;
                  SDL_FillRect(sdlscrn, &rct, grey); /* Clear old */
                  SDLGui_DrawRadioButton(dlg, i);
                  SDL_UpdateRects(sdlscrn, 1, &rct);
                }
                dlg[obj].state |= SG_SELECTED;  /* Select this radio button */
                rct.x = (dlg[0].x+dlg[obj].x)*fontwidth;
                rct.y = (dlg[0].y+dlg[obj].y)*fontheight;
                rct.w = fontwidth;  rct.h = fontheight;
                SDL_FillRect(sdlscrn, &rct, grey); /* Clear old */
                SDLGui_DrawRadioButton(dlg, obj);
                SDL_UpdateRects(sdlscrn, 1, &rct);
                break;
              case SGCHECKBOX:
                dlg[obj].state ^= SG_SELECTED;
                rct.x = (dlg[0].x+dlg[obj].x)*fontwidth;
                rct.y = (dlg[0].y+dlg[obj].y)*fontheight;
                rct.w = fontwidth;  rct.h = fontheight;
                SDL_FillRect(sdlscrn, &rct, grey); /* Clear old */
                SDLGui_DrawCheckBox(dlg, obj);
                SDL_UpdateRects(sdlscrn, 1, &rct);
                break;
              case SGPOPUP:
                dlg[obj].state |= SG_SELECTED;
                SDLGui_DrawPopupButton(dlg, obj);
                SDL_UpdateRect(sdlscrn, (dlg[0].x+dlg[obj].x)*fontwidth-2, (dlg[0].y+dlg[obj].y)*fontheight-2,
                           dlg[obj].w*fontwidth+4, dlg[obj].h*fontheight+4);
                retbutton=obj;
                break;
            }
          }
          if(oldbutton>0)
          {
            dlg[oldbutton].state &= ~SG_SELECTED;
            SDLGui_DrawButton(dlg, oldbutton);
            SDL_UpdateRect(sdlscrn, (dlg[0].x+dlg[oldbutton].x)*fontwidth-2, (dlg[0].y+dlg[oldbutton].y)*fontheight-2,
                           dlg[oldbutton].w*fontwidth+4, dlg[oldbutton].h*fontheight+4);
            oldbutton = 0;
          }
          if( dlg[obj].flags&SG_EXIT )
          {
            retbutton = obj;
          }
          break;
      }
  }
  while(retbutton==0 && !bQuitProgram);

  if(bQuitProgram)
    retbutton=-1;

  return retbutton;
}


/*-----------------------------------------------------------------------*/
/*
  Show and process a file select dialog.
  Returns TRUE if the use selected "okay", FALSE if "cancel".
  input: zip_path = pointer to buffer to contain file path within a selected
  zip file, or NULL if browsing zip files is disallowed.
*/
#define SGFSDLG_UPDIR     6
#define SGFSDLG_ROOTDIR   7
#define SGFSDLG_ENTRY1    10
#define SGFSDLG_ENTRY16   25
#define SGFSDLG_UP        26
#define SGFSDLG_DOWN      27
#define SGFSDLG_OKAY      28
#define SGFSDLG_CANCEL    29
int SDLGui_FileSelect(char *path_and_name, char *zip_path)
{
  int i,n;
  int entries = 0;                             /* How many files are in the actual directory? */
  int ypos = 0;
  char dlgfilenames[16][36];
  struct dirent **files = NULL;
  char path[MAX_FILENAME_LENGTH], fname[128];  /* The actual file and path names */
  char dlgpath[39], dlgfname[33];              /* File and path name in the dialog */
  BOOL reloaddir = TRUE;                       /* Do we have to reload the directory file list? */
  BOOL refreshentries = TRUE;                  /* Do we have to update the file names in the dialog? */
  int retbut;
  int oldcursorstate;
  int selection = -1;                          /* The actual selection, -1 if none selected */
  char zipfilename[MAX_FILENAME_LENGTH];       /* Filename in zip file */
  char zipdir[MAX_FILENAME_LENGTH];
  BOOL browsingzip = FALSE;                    /* Are we browsing an archive? */
  zip_dir *zipfiles = NULL;

  SGOBJ fsdlg[] =
  {
    { SGBOX, 0, 0, 0,0, 40,25, NULL },
    { SGTEXT, 0, 0, 13,1, 13,1, "Choose a file" },
    { SGTEXT, 0, 0, 1,2, 7,1, "Folder:" },
    { SGTEXT, 0, 0, 1,3, 38,1, dlgpath },
    { SGTEXT, 0, 0, 1,4, 6,1, "File:" },
    { SGTEXT, 0, 0, 7,4, 31,1, dlgfname },
    { SGBUTTON, 0, 0, 31,1, 4,1, ".." },
    { SGBUTTON, 0, 0, 36,1, 3,1, "/" },
    { SGBOX, 0, 0, 1,6, 38,16, NULL },
    { SGBOX, 0, 0, 38,7, 1,14, NULL },
    { SGTEXT, SG_EXIT, 0, 2,6, 35,1, dlgfilenames[0] },
    { SGTEXT, SG_EXIT, 0, 2,7, 35,1, dlgfilenames[1] },
    { SGTEXT, SG_EXIT, 0, 2,8, 35,1, dlgfilenames[2] },
    { SGTEXT, SG_EXIT, 0, 2,9, 35,1, dlgfilenames[3] },
    { SGTEXT, SG_EXIT, 0, 2,10, 35,1, dlgfilenames[4] },
    { SGTEXT, SG_EXIT, 0, 2,11, 35,1, dlgfilenames[5] },
    { SGTEXT, SG_EXIT, 0, 2,12, 35,1, dlgfilenames[6] },
    { SGTEXT, SG_EXIT, 0, 2,13, 35,1, dlgfilenames[7] },
    { SGTEXT, SG_EXIT, 0, 2,14, 35,1, dlgfilenames[8] },
    { SGTEXT, SG_EXIT, 0, 2,15, 35,1, dlgfilenames[9] },
    { SGTEXT, SG_EXIT, 0, 2,16, 35,1, dlgfilenames[10] },
    { SGTEXT, SG_EXIT, 0, 2,17, 35,1, dlgfilenames[11] },
    { SGTEXT, SG_EXIT, 0, 2,18, 35,1, dlgfilenames[12] },
    { SGTEXT, SG_EXIT, 0, 2,19, 35,1, dlgfilenames[13] },
    { SGTEXT, SG_EXIT, 0, 2,20, 35,1, dlgfilenames[14] },
    { SGTEXT, SG_EXIT, 0, 2,21, 35,1, dlgfilenames[15] },
    { SGBUTTON, SG_TOUCHEXIT, 0, 38,6, 1,1, "\x01" },          /* Arrow up */
    { SGBUTTON, SG_TOUCHEXIT, 0, 38,21, 1,1, "\x02" },         /* Arrow down */
    { SGBUTTON, 0, 0, 10,23, 8,1, "Okay" },
    { SGBUTTON, 0, 0, 24,23, 8,1, "Cancel" },
    { -1, 0, 0, 0,0, 0,0, NULL }
  };

  SDLGui_CenterDlg(fsdlg);

  /* Prepare the path and filename variables */
  File_splitpath(path_and_name, path, fname, NULL);
  File_ShrinkName(dlgpath, path, 38);
  File_ShrinkName(dlgfname, fname, 32);

  /* Save old mouse cursor state and enable cursor anyway */
  oldcursorstate = SDL_ShowCursor(SDL_QUERY);
  if( oldcursorstate==SDL_DISABLE )
    SDL_ShowCursor(SDL_ENABLE);

  do
    {
      if( reloaddir )
	{
	  if( strlen(path)>=MAX_FILENAME_LENGTH )
	    {
	      fprintf(stderr, "SDLGui_FileSelect: Path name too long!\n");
	      return FALSE;
	    }

	  /* Free old allocated memory: */
	  if( files!=NULL )
	    {
	      for(i=0; i<entries; i++)
		{
		  free(files[i]);
		}
	      free(files);
	      files = NULL;
	    }

	  if( browsingzip )
	    {
	      files = ZIP_GetFilesDir(zipfiles, zipdir, &entries);
	    }
	  else
	    {
	      /* Load directory entries: */
	      entries = scandir(path, &files, 0, alphasort);
	    }

	  if(entries<0)
	    {
	      fprintf(stderr, "SDLGui_FileSelect: Path not found.\n");
	      return FALSE;
	    }

	  reloaddir = FALSE;
	  refreshentries = TRUE;
	}/* reloaddir */


      if( refreshentries )
	{
	  /* Copy entries to dialog: */
	  for(i=0; i<16; i++)
	    {
	      if( i+ypos<entries )
		{
 		  char tempstr[MAX_FILENAME_LENGTH];
		  struct stat filestat;
		  /* Prepare entries: */
		  strcpy(tempstr, "  ");
		  strcat(tempstr, files[i+ypos]->d_name);
		  File_ShrinkName(dlgfilenames[i], tempstr, 35);
		  /* Mark folders: */
		  strcpy(tempstr, path);
		  strcat(tempstr, files[i+ypos]->d_name);

		  if( browsingzip )
		    {
		      if( tempstr[strlen(tempstr)-1] == '/'  )
			dlgfilenames[i][0] = SGFOLDER;    /* Mark folders */
		    }
		  else
		    {
		      if( stat(tempstr, &filestat)==0 && S_ISDIR(filestat.st_mode) )
			dlgfilenames[i][0] = SGFOLDER;    /* Mark folders */
		      if( File_FileNameIsZIP(tempstr) && browsingzip == FALSE)
			dlgfilenames[i][0] = SGFOLDER;    /* Mark .ZIP archives as folders */
		    }
		}
	      else
		dlgfilenames[i][0] = 0;  /* Clear entry */
	    }
	  refreshentries = FALSE;
	}/* refreshentries */

      /* Show dialog: */
      retbut = SDLGui_DoDialog(fsdlg);

      /* Has the user clicked on a file or folder? */
      if( retbut>=SGFSDLG_ENTRY1 && retbut<=SGFSDLG_ENTRY16 && retbut-SGFSDLG_ENTRY1+ypos<entries)
	{
	  char tempstr[MAX_FILENAME_LENGTH];
	  struct stat filestat;

	  if( browsingzip == TRUE )
	    {
	      strcpy(tempstr, zipdir);
	      strcat(tempstr, files[retbut-SGFSDLG_ENTRY1+ypos]->d_name);
	      if(tempstr[strlen(tempstr)-1] == '/')
		{
		  /* handle the ../ directory */
		  if(strcmp(files[retbut-SGFSDLG_ENTRY1+ypos]->d_name, "../") == 0)
		    {
		      /* close the zip file */
		      if( strcmp(tempstr, "../") == 0 )
			{
			  reloaddir = refreshentries = TRUE;
			  /* free zip file entries */
			  while(zipfiles->nfiles > 0)
			    {
			      Memory_Free(zipfiles->names[zipfiles->nfiles-1]);
			      zipfiles->nfiles--;
			    }
			  Memory_Free(zipfiles);
			  /* Copy the path name to the dialog */
			  File_ShrinkName(dlgpath, path, 38);
			  browsingzip = FALSE;
			}
		      else
			{
			  i=strlen(tempstr)-1; n=0;
			  while(i > 0 && n < 3) if( tempstr[i--] == '/' )n++;
			  if(tempstr[i+1] == '/') tempstr[i+2] = '\0';
			  else tempstr[0] = '\0';

			  strcpy(zipdir, tempstr);
			  File_ShrinkName(dlgpath, zipdir, 38);
			}
		    }
		  else /* not the "../" directory */
		    {
		      strcpy(zipdir, tempstr);
		      File_ShrinkName(dlgpath, zipdir, 38);
		    }
		  reloaddir = TRUE;
		  /* Copy the path name to the dialog */
		  selection = -1;                /* Remove old selection */
		  zipfilename[0] = '\0';
		  dlgfname[0] = 0;
		  ypos = 0;

		} else {
		  /* Select a file in the zip */
		  selection = retbut-SGFSDLG_ENTRY1+ypos;
		  strcpy(zipfilename, files[selection]->d_name);
		  File_ShrinkName(dlgfname, zipfilename, 32);
		}

	    } /* if browsingzip */
	  else
	    {
	      strcpy(tempstr, path);
	      strcat(tempstr, files[retbut-SGFSDLG_ENTRY1+ypos]->d_name);
	      if( stat(tempstr, &filestat)==0 && S_ISDIR(filestat.st_mode) )
		{
		  /* Set the new directory */
		  strcpy(path, tempstr);
		  if( strlen(path)>=3 )
		    {
		      if(path[strlen(path)-2]=='/' && path[strlen(path)-1]=='.')
			path[strlen(path)-2] = 0;  /* Strip a single dot at the end of the path name */
		      if(path[strlen(path)-3]=='/' && path[strlen(path)-2]=='.' && path[strlen(path)-1]=='.')
			{
			  /* Handle the ".." folder */
			  char *ptr;
			  if( strlen(path)==3 )
			    path[1] = 0;
			  else
			    {
			      path[strlen(path)-3] = 0;
			      ptr = strrchr(path, '/');
			      if(ptr)  *(ptr+1) = 0;
			    }
			}
		    }
		  File_AddSlashToEndFileName(path);
		  reloaddir = TRUE;
		  /* Copy the path name to the dialog */
		  File_ShrinkName(dlgpath, path, 38);
		  selection = -1;                /* Remove old selection */
		  dlgfname[0] = 0;
		  ypos = 0;
		}
	  else if ( File_FileNameIsZIP(tempstr) && zip_path != NULL )
	    {
	      /* open a zip file */
	      zipfiles = ZIP_GetFiles(tempstr);
	      if( zipfiles != NULL && browsingzip == FALSE )
		{
		  selection = retbut-SGFSDLG_ENTRY1+ypos;
		  strcpy(fname, files[selection]->d_name);
		  File_ShrinkName(dlgfname, fname, 32);
		  browsingzip=TRUE;
		  strcpy(zipdir, "");
		  File_ShrinkName(dlgpath, zipdir, 38);
		  reloaddir = refreshentries = TRUE;
		}

	    } else {
	      /* Select a file */
	      selection = retbut-SGFSDLG_ENTRY1+ypos;
	      strcpy(fname, files[selection]->d_name);
	      File_ShrinkName(dlgfname, fname, 32);
	    }

	    } /* not browsingzip */

	}
      else    /* Has the user clicked on another button? */
	{
	  switch(retbut)
	    {
	    case SGFSDLG_UPDIR:                 /* Change path to parent directory */

	      if( browsingzip )
	      {
	      /* close the zip file */
	      if( strcmp(zipdir, "") == 0 )
		{
		  reloaddir = refreshentries = TRUE;
		  /* free zip file entries */
		  while(zipfiles->nfiles > 0)
		    {
		      Memory_Free(zipfiles->names[zipfiles->nfiles-1]);
		      zipfiles->nfiles--;
		    }
		  Memory_Free(zipfiles);
		  /* Copy the path name to the dialog */
		  File_ShrinkName(dlgpath, path, 38);
		  browsingzip = FALSE;
		  reloaddir = TRUE;
		  selection = -1;                 /* Remove old selection */
		  fname[0] = 0;
		  dlgfname[0] = 0;
		  ypos = 0;
		}
	      else
		{
		  i=strlen(zipdir)-1; n=0;
		  while(i > 0 && n < 2) if( zipdir[i--] == '/' )n++;
		  if(zipdir[i+1] == '/') zipdir[i+2] = '\0';
		  else zipdir[0] = '\0';

		  File_ShrinkName(dlgpath, zipdir, 38);
		  reloaddir = TRUE;
		  selection = -1;                 /* Remove old selection */
		  zipfilename[0] = '\0';
		  dlgfname[0] = 0;
		  ypos = 0;
		}
	      }  /* not a zip file: */
	      else if( strlen(path)>2 )
		{
		  char *ptr;
		  File_CleanFileName(path);
		  ptr = strrchr(path, '/');
		  if(ptr)  *(ptr+1) = 0;
		  File_AddSlashToEndFileName(path);
		  reloaddir = TRUE;
		  File_ShrinkName(dlgpath, path, 38);  /* Copy the path name to the dialog */
		  selection = -1;                 /* Remove old selection */
		  fname[0] = 0;
		  dlgfname[0] = 0;
		  ypos = 0;
		}
	      break;
	    case SGFSDLG_ROOTDIR:               /* Change to root directory */
	      if( browsingzip )
		{
		  /* free zip file entries */
		  while(zipfiles->nfiles > 0)
		    {
		      Memory_Free(zipfiles->names[zipfiles->nfiles-1]);
		      zipfiles->nfiles--;
		    }
		  Memory_Free(zipfiles);
		  browsingzip = FALSE;
		}

	      strcpy(path, "/");
	      reloaddir = TRUE;
	      strcpy(dlgpath, path);
	      selection = -1;                   /* Remove old selection */
	      fname[0] = 0;
	      dlgfname[0] = 0;
	      ypos = 0;
	      break;
	    case SGFSDLG_UP:                    /* Scroll up */
	      if( ypos>0 )
		{
		  --ypos;
		  refreshentries = TRUE;
		}
	      SDL_Delay(20);
	      break;
	    case SGFSDLG_DOWN:                  /* Scroll down */
	      if( ypos+17<=entries )
		{
		  ++ypos;
		  refreshentries = TRUE;
		}
	      SDL_Delay(20);
	      break;
	    } /* switch */
	} /* other button code */


    } /* do */


  while(retbut!=SGFSDLG_OKAY && retbut!=SGFSDLG_CANCEL && !bQuitProgram);

  if( oldcursorstate==SDL_DISABLE )
    SDL_ShowCursor(SDL_DISABLE);

  File_makepath(path_and_name, path, fname, NULL);

  /* Free old allocated memory: */
  if( files!=NULL )
  {
    for(i=0; i<entries; i++)
    {
      free(files[i]);
    }
    free(files);
    files = NULL;
  }

  if( browsingzip )
    {
      /* free zip file entries */
      while(zipfiles->nfiles > 0)
	{
	  Memory_Free(zipfiles->names[zipfiles->nfiles-1]);
	  zipfiles->nfiles--;
	}
      Memory_Free(zipfiles);
    }

  if( zip_path != NULL )
    {
      if( browsingzip )
	{
	  strcpy(zip_path, zipdir);
	  strcat(zip_path, zipfilename);
	} else zip_path[0] = '\0';
    }
  return( retbut==SGFSDLG_OKAY );
}



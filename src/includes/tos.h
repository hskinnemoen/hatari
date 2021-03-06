/*
  Hatari - tos.h

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.
*/

#ifndef HATARI_TOS_H
#define HATARI_TOS_H

extern bool bIsEmuTOS;
extern Uint16 TosVersion;
extern Uint32 TosAddress, TosSize;
extern bool bTosImageLoaded;
extern bool bRamTosImage;
extern unsigned int ConnectedDriveMask;
extern int nNumDrives;

extern void TOS_MemorySnapShot_Capture(bool bSave);
extern int TOS_LoadImage(void);

#endif

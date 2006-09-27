/*
  Hatari - ide.h

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.
*/

#ifndef HATARI_IDE_H
#define HATARI_IDE_H

#include "sysdeps.h"

extern uae_u32 Ide_Mem_bget(uaecptr addr);
extern uae_u32 Ide_Mem_wget(uaecptr addr);
extern uae_u32 Ide_Mem_lget(uaecptr addr);
extern void Ide_Mem_bput(uaecptr addr, uae_u32 val);
extern void Ide_Mem_wput(uaecptr addr, uae_u32 val);
extern void Ide_Mem_lput(uaecptr addr, uae_u32 val);

#endif /* HATARI_IDE_H */

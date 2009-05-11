/*
  Hatari - debugui.h

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.
*/

#ifndef HATARI_DEBUGUI_H
#define HATARI_DEBUGUI_H

/* DebugUI_ParseCommand() return values */
enum {
	DEBUG_QUIT,
	DEBUG_CMD,
	DEBUG_STEP
};

extern void DebugUI(void);
extern int DebugUI_ParseCommand(char *input);

extern bool bDebugStep;

#endif /* HATARI_DEBUGUI_H */

/*
  Hatari - st.c

  This file is distributed under the GNU Public License, version 2 or at
  your option any later version. Read the file gpl.txt for details.

  ST disk image support.

  The file format of the .ST image files is simplicity itself. They are just
  straight images of the disk in question, with sectors stored in the expected
  logical order.
  So, on a sector basis the images run from sector 0 (bootsector) to however
  many sectors are on the disk. On a track basis the layout is the same as for
  MSA files but obviously the data is raw, no track header or compression or
  anything like that.

  TRACK 0, SIDE 0
  TRACK 0, SIDE 1
  TRACK 1, SIDE 0
  TRACK 1, SIDE 1
  TRACK 2, SIDE 0
  TRACK 2, SIDE 1
*/
const char ST_rcsid[] = "Hatari $Id: st.c,v 1.9 2007-10-31 21:31:50 eerot Exp $";

#include "main.h"
#include "file.h"
#include "st.h"

#define SAVE_TO_ST_IMAGES


#if defined(__riscos)
/* The following two lines are required on RISC OS for preventing it from
 * interfering with the .ST image files: */
#include <unixlib/local.h>
int __feature_imagefs_is_file = 1;
#endif


/*-----------------------------------------------------------------------*/
/**
 * Does filename end with a .ST extension? If so, return TRUE
 */
BOOL ST_FileNameIsST(char *pszFileName, BOOL bAllowGZ)
{
	return(File_DoesFileExtensionMatch(pszFileName,".st")
	       || (bAllowGZ && File_DoesFileExtensionMatch(pszFileName,".st.gz")));
}


/*-----------------------------------------------------------------------*/
/**
 * Load .ST file into memory, set number of bytes loaded and return a pointer
 * to the buffer.
 */
Uint8 *ST_ReadDisk(char *pszFileName, long *pImageSize)
{
	Uint8 *pStFile;

	*pImageSize = 0;

	/* Just load directly a buffer, and set ImageSize accordingly */
	pStFile = File_Read(pszFileName, pImageSize, NULL);
	if (!pStFile)
		*pImageSize = 0;

	return pStFile;
}


/*-----------------------------------------------------------------------*/
/**
 * Save .ST file from memory buffer. Returns TRUE is all OK
 */
BOOL ST_WriteDisk(char *pszFileName, Uint8 *pBuffer, int ImageSize)
{
#ifdef SAVE_TO_ST_IMAGES

	/* Just save buffer directly to file */
	return( File_Save(pszFileName, pBuffer, ImageSize, FALSE) );

#else   /*SAVE_TO_ST_IMAGES*/

	/* Oops, cannot save */
	return FALSE;

#endif  /*SAVE_TO_ST_IMAGES*/
}

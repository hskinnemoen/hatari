/*
  Hatari
*/

/*-----------------------------------------------------------------------*/
/* VDI Screens 640x480 */
SCREENDRAW VDIScreenDraw_640x480[] = {
  {  /* Low */
    ConvertVDIRes_16Colour,
    /*MODE_640x480x256,*/
    640,480,8,1,
    {
      { 0,640/2, 0,480,  0,0 },
      { 0,640/2, 0,480,  0,0 },
      { 0,640/2, 0,480,  0,0 },
      { 0,640/2, 0,480,  0,0 },
    }
  },
  {  /* Medium */
    ConvertVDIRes_4Colour,
    /*MODE_640x480x256,*/
    640,480,8,1,
    {
      { 0,640/4, 0,480,  0,0 },
      { 0,640/4, 0,480,  0,0 },
      { 0,640/4, 0,480,  0,0 },
      { 0,640/4, 0,480,  0,0 },
    }
  },
  {  /* High */
    ConvertVDIRes_2Colour,
    /*MODE_640x480x256,*/
    640,480,8,1,
    {
      { 0,640/8, 0,480,  0,0 },
      { 0,640/8, 0,480,  0,0 },
      { 0,640/8, 0,480,  0,0 },
      { 0,640/8, 0,480,  0,0 },
    }
  },
};

/* VDI Screens 800x600 */
SCREENDRAW VDIScreenDraw_800x600[] = {
  {  /* Low */
    ConvertVDIRes_16Colour,
    /*MODE_800x600x256,*/
    800,600,8,1,
    {
      { 0,800/2, 0,600,  0,0 },
      { 0,800/2, 0,600,  0,0 },
      { 0,800/2, 0,600,  0,0 },
      { 0,800/2, 0,600,  0,0 },
    }
  },
  {  /* Medium */
    ConvertVDIRes_4Colour,
    /*MODE_800x600x256,*/
    800,600,8,1,
    {
      { 0,800/4, 0,600,  0,0 },
      { 0,800/4, 0,600,  0,0 },
      { 0,800/4, 0,600,  0,0 },
      { 0,800/4, 0,600,  0,0 },
    }
  },
  {  /* High */
    ConvertVDIRes_2Colour,
    /*MODE_800x600x256,*/
    800,600,8,1,
    {
      { 0,800/8, 0,600,  0,0 },
      { 0,800/8, 0,600,  0,0 },
      { 0,800/8, 0,600,  0,0 },
      { 0,800/8, 0,600,  0,0 },
    }
  },
};

/* VDI Screens 1024x768 */
SCREENDRAW VDIScreenDraw_1024x768[] = {
  {  /* Low */
    ConvertVDIRes_16Colour,
    /*MODE_1024x768x256,*/
    1024,768,8,1,
    {
      { 0,1024/2, 0,768,  0,0 },
      { 0,1024/2, 0,768,  0,0 },
      { 0,1024/2, 0,768,  0,0 },
      { 0,1024/2, 0,768,  0,0 },
    }
  },
  {  /* Medium */
    ConvertVDIRes_4Colour,
    /*MODE_1024x768x256,*/
    1024,768,8,1,
    {
      { 0,1024/4, 0,768,  0,0 },
      { 0,1024/4, 0,768,  0,0 },
      { 0,1024/4, 0,768,  0,0 },
      { 0,1024/4, 0,768,  0,0 },
    }
  },
  {  /* High */
    ConvertVDIRes_2Colour,
    /*MODE_1024x768x256,*/
    1024,768,8,1,
    {
      { 0,1024/8, 0,768,  0,0 },
      { 0,1024/8, 0,768,  0,0 },
      { 0,1024/8, 0,768,  0,0 },
      { 0,1024/8, 0,768,  0,0 },
    }
  },
};

/*-----------------------------------------------------------------------*/
///// NO OVERSCAN
SCREENDRAW ScreenDraw_Low_320x200x256_NoOverscan = {
  ConvertLowRes_320x8Bit,
  /*MODE_320x200x256,*/
  320,200,8,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Low_320x240x256_NoOverscan = {
  ConvertLowRes_320x8Bit,
  /*MODE_320x240x256,*/
  320,240,8,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 }
  }
};

SCREENDRAW ScreenDraw_Low_320x200x16Bit_NoOverscan = {
  ConvertLowRes_320x16Bit,
  /*MODE_320x200x16BIT,*/
  320,200,16,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Low_320x240x16Bit_NoOverscan = {
  ConvertLowRes_320x16Bit,
  /*MODE_320x240x16BIT,*/
  320,240,16,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 }
  }
};

SCREENDRAW ScreenDraw_Low_640x400x256_NoOverscan = {
  ConvertLowRes_640x8Bit,
  /*MODE_640x400x256,*/
  640,400,8,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Low_640x480x256_NoOverscan = {
  ConvertLowRes_640x8Bit,
  /*MODE_640x480x256,*/
  640,480,8,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 }
  }
};

SCREENDRAW ScreenDraw_Low_640x400x16Bit_NoOverscan = {
  ConvertLowRes_640x16Bit,
  /*MODE_640x400x16BIT,*/
  640,400,16,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Low_640x480x16Bit_NoOverscan = {
  ConvertLowRes_640x16Bit,
  /*MODE_640x480x16BIT,*/
  640,480,16,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 }
  }
};

SCREENDRAW ScreenDraw_Medium_640x400x256_NoOverscan = {
  ConvertMediumRes_640x8Bit,
  /*MODE_640x400x256,*/
  640,400,8,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Medium_640x480x256_NoOverscan = {
  ConvertMediumRes_640x8Bit,
  /*MODE_640x480x256,*/
  640,480,8,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 }
  }
};

SCREENDRAW ScreenDraw_Medium_640x400x16Bit_NoOverscan = {
  ConvertMediumRes_640x16Bit,
  /*MODE_640x400x16BIT,*/
  640,400,16,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Medium_640x480x16Bit_NoOverscan = {
  ConvertMediumRes_640x16Bit,
  /*MODE_640x480x16BIT,*/
  640,480,16,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 }
  }
};

SCREENDRAW ScreenDraw_High_640x400x256_NoOverscan = {
  ConvertHighRes_640x8Bit,
  /*MODE_640x400x256,*/
  640,400,8,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  0,0 },  // These are not valid!(cannot have overscan in High Res)
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  0,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  0,0 },
  }
};

SCREENDRAW ScreenDraw_High_640x480x256_NoOverscan = {
  ConvertHighRes_640x8Bit,
  /*MODE_640x480x256,*/
  640,480,8,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  40,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  40,0 },  // These are not valid!(cannot have overscan in High Res)
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  40,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  40,0 },
  }
};

//-----------------------------------------------------------------------
///// OVERSCAN
#define ScreenDraw_Low_320x200x256 ScreenDraw_Low_320x200x256_NoOverscan
#define ScreenDraw_Medium_640x400x256 ScreenDraw_Medium_640x400x256_NoOverscan
#define ScreenDraw_High_640x400x256 ScreenDraw_High_640x400x256_NoOverscan
#define ScreenDraw_Medium_640x400x16Bit ScreenDraw_Medium_640x400x16Bit_NoOverscan
#define ScreenDraw_Low_640x400x256 ScreenDraw_Low_640x400x256_NoOverscan
#define ScreenDraw_Low_640x400x16Bit ScreenDraw_Low_640x400x16Bit_NoOverscan
#define ScreenDraw_Low_320x200x16Bit ScreenDraw_Low_320x200x16Bit_NoOverscan

SCREENDRAW ScreenDraw_Low_320x240x256 = {
  ConvertLowRes_320x8Bit,
  /*MODE_320x240x256,*/
  320,240,8,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 0,OVERSCAN_TOP+200,    6,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200+OVERSCAN_BOTTOM,  1,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 13,OVERSCAN_TOP+200+17,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Low_320x240x16Bit = {
  ConvertLowRes_320x16Bit,
  /*MODE_320x240x16BIT,*/
  320,240,16,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 0,OVERSCAN_TOP+200,    6,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200+OVERSCAN_BOTTOM,  1,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 13,OVERSCAN_TOP+200+17,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Low_640x480x256 = {
  ConvertLowRes_640x8Bit,
  /*MODE_640x480x256,*/
  640,480,8,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 0,OVERSCAN_TOP+200,    6,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200+OVERSCAN_BOTTOM,  1,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 13,OVERSCAN_TOP+200+17,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Low_800x600x256 = {
  ConvertLowRes_640x8Bit,
  /*MODE_800x600x256,*/
  800,600,8,2,
  {
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16 }
  }
};

SCREENDRAW ScreenDraw_Low_640x480x16Bit = {
  ConvertLowRes_640x16Bit,
  /*MODE_640x480x16BIT,*/
  640,480,16,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 0,OVERSCAN_TOP+200,    6,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200+OVERSCAN_BOTTOM,  1,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 13,OVERSCAN_TOP+200+17,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Low_800x600x16Bit = {
  ConvertLowRes_640x16Bit,
  /*MODE_800x600x16BIT,*/
  800,600,16,2,
  {
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16*2 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16*2 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16*2 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16*2 }
  }
};

SCREENDRAW ScreenDraw_Medium_640x480x256 = {
  ConvertMediumRes_640x8Bit,
  /*MODE_640x480x256,*/
  640,480,8,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 0,OVERSCAN_TOP+200,    6,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200+OVERSCAN_BOTTOM,  1,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 13,OVERSCAN_TOP+200+17,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Medium_800x600x256 = {
  ConvertMediumRes_640x8Bit,
  /*MODE_800x600x256,*/
  800,600,8,2,
  {
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16 }
  }
};

SCREENDRAW ScreenDraw_Medium_640x480x16Bit = {
  ConvertMediumRes_640x16Bit,
  /*MODE_640x480x16BIT,*/
  640,480,16,2,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200,  20,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 0,OVERSCAN_TOP+200,    6,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+200+OVERSCAN_BOTTOM,  1,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, 13,OVERSCAN_TOP+200+17,  0,0 }
  }
};

SCREENDRAW ScreenDraw_Medium_800x600x16Bit = {
  ConvertMediumRes_640x16Bit,
  /*MODE_800x600x16BIT,*/
  800,600,16,2,
  {
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16*2 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16*2 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16*2 },
    { 0,SCREENBYTES_LINE, 0,NUM_VISIBLE_LINES,  16,16*2 }
  }
};

SCREENDRAW ScreenDraw_High_640x480x256 = {
  ConvertHighRes_640x8Bit,
  /*MODE_640x480x256,*/
  640,480,8,1,
  {
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  40,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  40,0 },  // These are not valid!(cannot have overscan in High Res)
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  40,0 },
    { SCREENBYTES_LEFT,SCREENBYTES_MIDDLE, OVERSCAN_TOP,OVERSCAN_TOP+400,  40,0 },
  }
};


//-----------------------------------------------------------------------
// Modes to select according to chosen option from dialog(with and without overscan)
// In order DISPLAYMODE_16COL_LOWRES,DISPLAYMODE_16COL_HIGHRES,DISPLAYMODE_16COL_FULL,DISPLAYMODE_HICOL_LOWRES,DISPLAYMODE_HICOL_HIGHRES and DISPLAYMODE_HICOL_FULL
SCREENDRAW_DISPLAYOPTIONS ScreenDisplayOptions_NoOverscan[] = {
  // Low-Colour, Low Res
  {
    &ScreenDraw_Low_320x200x256_NoOverscan,&ScreenDraw_Low_320x240x256_NoOverscan,
    &ScreenDraw_Medium_640x400x256_NoOverscan,&ScreenDraw_Medium_640x480x256_NoOverscan,
    &ScreenDraw_High_640x400x256_NoOverscan,&ScreenDraw_High_640x480x256_NoOverscan,
    &ScreenDraw_Medium_640x400x256_NoOverscan,&ScreenDraw_Medium_640x480x256_NoOverscan,
  },
  // Low-Colour, High Res
  {
    &ScreenDraw_Low_640x400x256_NoOverscan,&ScreenDraw_Low_640x480x256_NoOverscan,
    &ScreenDraw_Medium_640x400x256_NoOverscan,&ScreenDraw_Medium_640x480x256_NoOverscan,
    &ScreenDraw_High_640x400x256_NoOverscan,&ScreenDraw_High_640x480x256_NoOverscan,
    &ScreenDraw_Medium_640x400x256_NoOverscan,&ScreenDraw_Medium_640x480x256_NoOverscan,
  },
  // Low-Colour, Full View
  {
    &ScreenDraw_Low_800x600x256,&ScreenDraw_Low_800x600x256,
    &ScreenDraw_Medium_800x600x256,&ScreenDraw_Medium_800x600x256,
    &ScreenDraw_High_640x400x256_NoOverscan,&ScreenDraw_High_640x480x256_NoOverscan,
    &ScreenDraw_Medium_800x600x256,&ScreenDraw_Medium_800x600x256,
  },
  // Hi-Colour, Low Res
  {
    &ScreenDraw_Low_320x200x16Bit_NoOverscan,&ScreenDraw_Low_320x240x16Bit_NoOverscan,
    &ScreenDraw_Medium_640x400x16Bit_NoOverscan,&ScreenDraw_Medium_640x480x16Bit_NoOverscan,
    &ScreenDraw_High_640x400x256_NoOverscan,&ScreenDraw_High_640x480x256_NoOverscan,
    &ScreenDraw_Medium_640x400x16Bit_NoOverscan,&ScreenDraw_Medium_640x480x16Bit_NoOverscan,
  },
  // Hi-Colour, High Res
  {
    &ScreenDraw_Low_640x400x16Bit_NoOverscan,&ScreenDraw_Low_640x480x16Bit_NoOverscan,
    &ScreenDraw_Medium_640x400x16Bit_NoOverscan,&ScreenDraw_Medium_640x480x16Bit_NoOverscan,
    &ScreenDraw_High_640x400x256_NoOverscan,&ScreenDraw_High_640x480x256_NoOverscan,
    &ScreenDraw_Medium_640x400x16Bit_NoOverscan,&ScreenDraw_Medium_640x480x16Bit_NoOverscan,
  },
  // Hi-Colour, Full View
  {
    &ScreenDraw_Low_800x600x16Bit,&ScreenDraw_Low_800x600x16Bit,
    &ScreenDraw_Medium_800x600x16Bit,&ScreenDraw_Medium_800x600x16Bit,
    &ScreenDraw_High_640x400x256_NoOverscan,&ScreenDraw_High_640x480x256_NoOverscan,
    &ScreenDraw_Medium_800x600x16Bit,&ScreenDraw_Medium_800x600x16Bit,
  }
};

SCREENDRAW_DISPLAYOPTIONS ScreenDisplayOptions[] = {
  // Low-Colour, Low Res
  {
    &ScreenDraw_Low_320x200x256,&ScreenDraw_Low_320x200x256_NoOverscan,
    &ScreenDraw_Medium_640x400x256,&ScreenDraw_Medium_640x400x256_NoOverscan,
    &ScreenDraw_High_640x400x256,&ScreenDraw_High_640x400x256_NoOverscan,
    &ScreenDraw_Medium_640x400x256,&ScreenDraw_Medium_640x400x256_NoOverscan,
  },
  // Low-Colour, High Res
  {
    &ScreenDraw_Low_640x400x256,&ScreenDraw_Low_640x400x256_NoOverscan,
    &ScreenDraw_Medium_640x400x256,&ScreenDraw_Medium_640x400x256_NoOverscan,
    &ScreenDraw_High_640x400x256,&ScreenDraw_High_640x400x256_NoOverscan,
    &ScreenDraw_Medium_640x400x256,&ScreenDraw_Medium_640x400x256_NoOverscan,
  },
  // Low-Colour, Full View
  {
    &ScreenDraw_Low_800x600x256,&ScreenDraw_Low_800x600x256,
    &ScreenDraw_Medium_800x600x256,&ScreenDraw_Medium_800x600x256,
    &ScreenDraw_High_640x400x256,&ScreenDraw_High_640x400x256_NoOverscan,
    &ScreenDraw_Medium_800x600x256,&ScreenDraw_Medium_800x600x256,
  },
  // Hi-Colour, Low Res
  {
    &ScreenDraw_Low_320x200x16Bit,&ScreenDraw_Low_320x200x16Bit_NoOverscan,
    &ScreenDraw_Medium_640x400x16Bit,&ScreenDraw_Medium_640x400x16Bit_NoOverscan,
    &ScreenDraw_High_640x400x256,&ScreenDraw_High_640x400x256_NoOverscan,
    &ScreenDraw_Medium_640x400x16Bit,&ScreenDraw_Medium_640x400x16Bit_NoOverscan,
  },
  // Hi-Colour, High Res
  {
    &ScreenDraw_Low_640x400x16Bit,&ScreenDraw_Low_640x400x16Bit_NoOverscan,
    &ScreenDraw_Medium_640x400x16Bit,&ScreenDraw_Medium_640x400x16Bit_NoOverscan,
    &ScreenDraw_High_640x400x256,&ScreenDraw_High_640x400x256_NoOverscan,
    &ScreenDraw_Medium_640x400x16Bit,&ScreenDraw_Medium_640x400x16Bit_NoOverscan,
  },
  // Hi-Colour, Full View
  {
    &ScreenDraw_Low_800x600x16Bit,&ScreenDraw_Low_800x600x16Bit,
    &ScreenDraw_Medium_800x600x16Bit,&ScreenDraw_Medium_800x600x16Bit,
    &ScreenDraw_High_640x400x256,&ScreenDraw_High_640x400x256_NoOverscan,
    &ScreenDraw_Medium_800x600x16Bit,&ScreenDraw_Medium_800x600x16Bit,
  }
};

/*
  Hatari
*/

// Interrupt handlers in system
enum {
  INTERRUPT_NULL,

#ifdef USE_DEBUGGER
  INTERRUPT_DEBUGGER,
  INTERRUPT_SINGLESTEP,
#endif  //USE_DEBUGGER

  INTERRUPT_VIDEO_VBL,
  INTERRUPT_VIDEO_VBL_PENDING,
  INTERRUPT_VIDEO_HBL,
  INTERRUPT_VIDEO_ENDLINE,
  INTERRUPT_MFP_TIMERA,
  INTERRUPT_MFP_TIMERB,
  INTERRUPT_MFP_TIMERC,
  INTERRUPT_MFP_TIMERD,
  INTERRUPT_IKBD_RESETTIMER,
  INTERRUPT_IKBD_ACIA,
  
  MAX_INTERRUPTS
};

// Event timer structure - keeps next timer to occur in structure so don't need to check all entries
typedef struct {
  BOOL bUsed;                      // Is interrupt active?
  int Cycles;
  void *pFunction;
} INTERRUPTHANDLER;

extern void *pIntHandlerFunctions[];
extern int nCyclesOver;
extern int nFrameCyclesOver;

extern void Int_Reset(void);
extern void Int_MemorySnapShot_Capture(BOOL bSave);
extern int Int_HandlerFunctionToID(void *pHandlerFunction);
extern void *Int_IDToHandlerFunction(int ID);
extern int Int_FindFrameCycles(void);
extern void Int_SetNewInterrupt(void);
extern void Int_AcknowledgeInterrupt(void);
extern void Int_AddAbsoluteInterrupt(int CycleTime, int Handler);
extern void Int_AddRelativeInterrupt(int CycleTime, int Handler);
extern void Int_RemovePendingInterrupt(int Handler);
extern BOOL Int_InterruptActive(int Handler);
extern int Int_FindCyclesPassed(int Handler);
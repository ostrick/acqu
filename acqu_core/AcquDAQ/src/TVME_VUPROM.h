//--Author	JRM Annand   19th May 2012
//--Rev 	...
//--Rev 	JRM Annand    4th Jul 2012 Working version
//--Rev 	JRM Annand    7th Jul 2012 Add some functions
//--Rev  	JRM Annand   12th Jul 2012 Add online control
//--Rev 	JRM Annand   29th Aug 2012 Various L1 width, strobe delay
//--Rev 	JRM Annand    2nd Sep 2012 TAPS enable
//--Rev 	JRM Annand    3rd Sep 2012 Default fType Slow-Ctrl
//--Rev 	B Oussena    19th Sep 2012 Bug fix L1,2 prescale address
//--Rev 	JRM Annand   26th Sep 2012 Add registers for multiplicity
//--Rev 	JRM Annand   28th Sep 2012 Pattern readout and extra ctrl fns
//--Rev  	JRM Annand    1st Dec 2012 Scaler read only, Ref TDC width ctrl
//--Rev 	JRM Annand    2nd Dec 2012 Mod RAM download (buff size & <=) 
//--Rev 	JRM Annand   28th Feb 2013 Modified L1,L2 prescale setup 
//--Update 	JRM Annand   10th Sep 2013 Reg. map changes for expanded DAQ 
//
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TVME_VUPROM
// GSI VUPROM FPGA base VMEbus module
// Has a multitude of functions, depending on the programing of the FPGA
// Trigger condition processor
// DAQ control master, up to 7 slave VME systems
// 192 channel scaler
// Hit pattern register
//

#ifndef __TVME_VUPROM_h__
#define __TVME_VUPROM_h__

#include "TVMEmodule.h"
#include <unistd.h>

//
enum{ EVU_MaxCPU=14 };     // maximum number of CPUs VUPROM can handle
// enumerator specifies internal register index
// For register address offsets see VMEreg_t VUPROMreg
enum{
  EVU_Firmware,
  EVU_MThresh1,
  EVU_MThresh2,
  EVU_L1mask,
  EVU_InpPrescale1,
  EVU_InpPrescale2,
  EVU_RdMValue,
  EVU_RdMPattern1,
  EVU_RdMPattern2,
  EVU_R1enable,
  EVU_R1addr,
  EVU_R1data,
  EVU_R1read,
  EVU_R2enable,
  EVU_R2addr,
  EVU_R2data,
  EVU_R2read,
  EVU_L1prescale,
  EVU_L1prescale1,
  EVU_L1prescale2,
  EVU_L1prescale3,
  EVU_L2prescale,
  EVU_L2prescale1,
  EVU_L2prescale2,
  EVU_L2prescale3,
  EVU_InPattRead,
  EVU_TriggerFiredPatt,
  EVU_L1Width,
  EVU_L1Delay,
  EVU_L2Delay,
  EVU_RefTDCWidth,
  EVU_CPUctrl,
  EVU_TrigCtrl,
  EVU_EnableTAPS,
  EVU_CPUreset,
  EVU_MasterReset,
  EVU_ACK,
  EVU_IRQ,
  EVU_TCS,
  EVU_HelicityInhibit,
  EVU_HelicityPattern,
  EVU_IntDelay0,
  EVU_FCDelay = EVU_IntDelay0 + EVU_MaxCPU,
  EVU_EvID,
  EVU_TrigEvID,
  EVU_StatEvID,
  EVU_ReadDebug,
  EVU_SetDebug0,
  EVU_SetDebug1,
  EVU_SetDebug2,
  EVU_SetDebug3,
  EVU_ScalerClr,
  EVU_ScalerLoad,
  EVU_Scaler
};

class TVME_VUPROM : public TVMEmodule {
 protected:
  UInt_t fTCSresetDelay;
  Int_t fPatternChan;
  Int_t fEnableCPU;           // 8 bits enable CPUs 0-7. 7 = local CPU
  Int_t fFCDelay;             // Delay from Fast Clear to re-arm trigger
  Int_t fIntDelay[EVU_MaxCPU];// Delays in interrupt signals to CPUs
  Int_t fRAMid;               // ID of pre-created trigger files
  Int_t fInputMask;           // Mask for input signals (1 = enable)
  Int_t fInputPrescale[16];   // Prescale values for input signals
  Int_t fL1Prescale[16];      // Prescale values for L1 signals
  Int_t fL2Prescale[16];      // Prescale values for L2 signals
  Int_t fL1Width;             // Width of L1 output 1
  Int_t fL1Delay;             // Internal L1 delay
  Int_t fL2Delay;             // Internal L2 delay
  Int_t fRefTDCWidth;         // Width Ref TDC output
  Int_t fDebugOut[16];        // Internal signals switched to debug outputs 0-3
  Int_t fTAPSEnable;          // 1 = TAPS in the busy circuit, 0 = NOT
  Int_t fHelicityInhibit;     // 1 = MAMI Helicity Inhibit, 0 = NOT
  Int_t fMThresh[16];         // Multiplicity threshold values
  Int_t fTrigParm[64];        // Holds trigger parameters for easy provision
  Char_t fCommandReply[128];  // Messages back to DAQ control
  Bool_t fIsIRQEnabled;       // Readout register activated?
  Bool_t fIsPattRead;         // pattern read activated?
  Bool_t fIsScalerRead;       // scaler read activated?
  Bool_t fIsGUIpermit;        // are you allowed to change via GUI
  Bool_t fIsTCSStarted;       // running state of the trigger control system
 public:
  TVME_VUPROM( Char_t*, Char_t*, FILE*, Char_t* );
  virtual ~TVME_VUPROM();
  virtual void SetConfig( Char_t*, Int_t );   // configure VME module
  virtual void PostInit( );
  virtual void WaitIRQ();
  virtual void ResetIRQ();
  virtual void EnableIRQ();
  virtual void DisableIRQ(){ fIsIRQEnabled = kFALSE; }
  // All TDAQmodules have the following procedures, by default dummies
  // Here they provide control of trigger-related hardware
  virtual void RunTrigCtrl();     // Issue start to TCS and enable triggers
  virtual void EndTrigCtrl();     // Disable triggers and Issue stop to TCS
  virtual void ResetTrigCtrl();   // Stop/start the TCS, disable/enable triggers
  virtual void EnableTrigCtrl();  // Enable triggers
  virtual void DisableTrigCtrl(); // Disable triggers
  virtual void ReadIRQ(void**);
  virtual void ReadIRQScaler(void**); // Readout scalers
  virtual Bool_t CheckHardID();       // Cross check loaded firmware version
  virtual void RAMDownload(Int_t,Int_t=-1);  // Download trigger pattern file
  virtual void SetDebugOut(Int_t, Int_t);    // switch debug output
  virtual void SetPrescale(Int_t, Int_t=-1, Int_t=0); // inp trig prescale val.
  //  virtual void SetLPrescale(Int_t, Int_t=-1, Int_t=0);// L1/L2 prescale val.
  virtual void SetIntDelay(Int_t, Int_t);    // interrupt delay
  virtual void CmdExe(Char_t*);              // execute command from DAQ ctrl
  virtual Int_t* GetParms(Int_t);            // provide trigger parameters
  virtual void SendEventID(UInt_t);          // send event ID to remote
  virtual UInt_t GetEventID();               // receive event ID from remote
  virtual void SetMThresh(Int_t, Int_t=-1);  // Set multiplicity thresholds
  virtual void StartTCS();                   // Start the TCS
  virtual void StopTCS();                    // Stop the TCS
  Char_t* GetCommandReply(){ return fCommandReply; }
  Bool_t IsIRQEnabled(){ return fIsIRQEnabled; }
  Bool_t IsPattRead(){ return fIsPattRead; }
  Bool_t IsScalerRead(){ return fIsScalerRead; }
  // Getters for protected variables
  virtual Int_t GetEnableCPU(){ return fEnableCPU; }
  Int_t GetFCDelay(){ return fFCDelay; }
  virtual Int_t GetIntDelay(Int_t i){ return fIntDelay[i]; }
  Int_t GetRAMid(){ return fRAMid; }
  Int_t GetInputMask(){ return fInputMask; }
  Int_t GetInputPrescale(Int_t i){ return fInputPrescale[i]; }
  Int_t GetL1Prescale(Int_t i){ return fL1Prescale[i]; }
  Int_t GetL2Prescale(Int_t i){ return fL2Prescale[i]; }
  Int_t GetL1Width(){ return fL1Width; }
  Int_t GetL1Delay(){ return fL1Delay; }
  Int_t GetL2Delay(){ return fL2Delay; }
  Int_t GetMThresh(Int_t i){ return fMThresh[i]; }
  //
  ClassDef(TVME_VUPROM,1)   

    };


//-----------------------------------------------------------------------------
inline void TVME_VUPROM::WaitIRQ( )
{
  // Poll the state of the IRQ-4 flag, bit 1 of the CSR register
  // Set IRQ ACK-2 NIM output if IRQ-4 received (acquisition busy)
  Int_t datum;
  for(;;){
    if( fIsIRQEnabled ){       // "interrupt" enabled?
      datum = Read(EVU_IRQ);   // poll IRQ register
      if( datum )break;        // exit when IRQ sensed
    }
  }
}

//-----------------------------------------------------------------------------
inline void TVME_VUPROM::ResetIRQ( )
{
  // Reset IRQ ACK-2 (busy) flag;
  //
  Write(EVU_ACK, 1);
}

//-----------------------------------------------------------------------------
inline void TVME_VUPROM::EnableIRQ( )
{
  // Enable triggers via INT-4 input and ACK-2 output
  //
  fIsIRQEnabled = kTRUE;
}

//-----------------------------------------------------------------------------
inline void TVME_VUPROM::SendEventID(UInt_t ID)
{
  // Send Event ID to remote system
  Write(EVU_EvID, ID);        // load the register
  Write(EVU_TrigEvID, 1);     // trigger the sending
}

//-----------------------------------------------------------------------------
inline UInt_t TVME_VUPROM::GetEventID()
{
  // Read Event ID from remote system
  // Time out after 100 tries
  UInt_t id;
  Int_t i;
  for(i=0;i<100;i++){
    id = Read(EVU_EvID);           // read the ID register
    if( id != 0xf0000000 ) break;  // 0xffffffff = nothing there yet
    usleep(1);
  }
  Write(EVU_TrigEvID, 1);          // reset the ID register
  return id;
}

//-----------------------------------------------------------------------------
inline void TVME_VUPROM::StartTCS()
{
  // Start the TCS system
  // but check 1st if its already running
  if( fIsTCSStarted ){
    printf("Must stop the TCS before starting\n");
    return;
  }
  Write( EVU_TCS, 0x1 );      // Issue TCS start
  fIsTCSStarted = kTRUE;      // flag that its started
}

//-----------------------------------------------------------------------------
inline void TVME_VUPROM::StopTCS()
{
  // Stop the TCS system
  // but check 1st if its already running
  if( !fIsTCSStarted ){
    printf("Must start the TCS before stopping\n");
    return;
  }
  Write( EVU_TCS, 0x2 );      // Issue TCS start
  fIsTCSStarted = kFALSE;     // flag that its stopped
}

#endif

//--Author	JRM Annand    2nd Dec 2007
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand... 1st Sep 2009 delete[]
//--Rev 	JRM Annand...12th May 2011 general bug fix and upgrade
//--Rev 	JRM Annand...16th May 2011 debug auto running
//--Rev 	JRM Annand... 6th Jul 2011 gcc4.6-x86_64, Button ptrs.
//--Rev 	JRM Annand...10th Jul 2011 acknowledge change of comment
//--Rev 	JRM Annand...12th Jul 2012 extend for trigger control
//--Rev 	JRM Annand...28th Sep 2012 new trigger control buttons
//--Update	JRM Annand...18th Sep 2013 path for gui logos
//--Description
//                *** AcquDAQ++ <-> Root ***
// DAQ for Sub-Atomic Physics Experiments.
//
// TDAQguiCtrl
// GUI control of online DAQ and Status of Analyser
// Run from AcquRoot thread, controlling either local readout, or via
// network connection to remote DAQ.
// Note the GUI requires ASimage library
// Build root with: ./configure linuxx8664gcc  --gminimal --enable-asimage
//
#ifndef __TDAQguiCtrl_h__
#define __TDAQguiCtrl_h__

#include "TGClient.h"
#include "TApplication.h"
#include "TGDockableFrame.h"
#include "TGMenu.h"
#include "TGMdiDecorFrame.h"
#include "TG3DLine.h"
#include "TGMdiFrame.h"
#include "TGMdiMainFrame.h"
#include "TGuiBldHintsButton.h"
#include "TGMdiMenu.h"
#include "TGColorDialog.h"
#include "TGListBox.h"
#include "TGNumberEntry.h"
#include "TGScrollBar.h"
#include "TGComboBox.h"
#include "TGuiBldHintsEditor.h"
#include "TGFrame.h"
#include "TGFileDialog.h"
#include "TGShutter.h"
#include "TGButtonGroup.h"
#include "TGCanvas.h"
#include "TGFSContainer.h"
#include "TGFontDialog.h"
#include "TGuiBldEditor.h"
#include "TGColorSelect.h"
#include "TGTextEdit.h"
#include "TGButton.h"
#include "TGFSComboBox.h"
#include "TGLabel.h"
#include "TGView.h"
#include "TRootGuiBuilder.h"
#include "TGTab.h"
#include "TGListView.h"
#include "TGSplitter.h"
#include "TRootCanvas.h"
#include "TGStatusBar.h"
#include "TGToolTip.h"
#include "TGToolBar.h"
#include "TRootEmbeddedCanvas.h"
#include "TCanvas.h"
#include "TGuiBldDragManager.h"
#include "TTimer.h"
#include "TH1F.h"
#include "Riostream.h"

#include <unistd.h>

#include "TDAQsupervise.h"
//class TDAQsupervise;
class TAcquRoot;

class TDAQguiCtrl : public TGMainFrame {
//class TDAQguiCtrl {
 private:
  TDAQsupervise* fSupervise;
  TAcquRoot* fAR;
  TGHorizontalFrame* fHFrame;
  TGVerticalFrame* fTextFrame;
  TGTextButton** fButton;
  TGTextEntry* fCom;
  TGNumberEntry* fRun;
  TGTextEntry* fRunT;
  TGTextEntry* fFile;
  TGTextEntry* fExp;
  TGTextEntry* fCmd;
  TGTextEntry* fHardErr;
  TGTextEntry* fScalerErr;
  TGTextEntry* fADCErr;
  TGTextEntry* fEvent;
  TGTextEntry* fBuffRec;
  TGTextEntry* fBuffSort;
  TGTextEntry* fMByte;
  TGTextEntry* fRunTime;
  TGTextView* fMsg;
  TGCheckButton* fStoreButton;
  TGCheckButton* fAutoButton;
  TGTextButton* fRunButton;
  TGTextButton* fEndButton;
  TGTextButton* fStartButton;
  TGTextButton* fPauseButton;
  TGTextButton* fGoButton;
  TGTextButton* fStopButton;
  TGTextButton* fKillButton;
  TGTextButton* fDumpButton;
  TGTextButton* fTestButton;
  TGTextButton* fUpdateButton;
  TGNumberEntry* fVuEnCPU;
  TGNumberEntry* fVuIntDelay;
  TGNumberEntry* fVuFCDelay;
  TGNumberEntry* fVuRAMDownload;
  TGNumberEntry* fVuInputMask;
  TGNumberEntry* fVuInputPrescale;
  TGNumberEntry* fVuL1Prescale;
  TGNumberEntry* fVuL2Prescale;
  TGNumberEntry* fVuSetDebug;
  TGNumberEntry* fVuSetChan;
  TGNumberEntry* fVuSetL1Width;
  TGNumberEntry* fVuSetDelay;
  TGNumberEntry* fVuSetMThresh;
  TGGroupFrame* fAnStat;
  TRootEmbeddedCanvas* fECanv;
  TCanvas* fCanv;
  //  TGMainFrame *fMain;
  TTimer* fUpdate;
  //  TTimer* fUpdateRun;
  TH1F* fEventRate;
  Int_t fStartDataBuff;
  Int_t fLastEvent;
  Int_t fTick;
  Int_t fTickLength;
  Int_t fMaxTick;
  Int_t fChan;
  ULong_t fTextBack;
  Bool_t fAutoRun;
  Bool_t fAutoEnd;
  //  Bool_t fIsRunning;
 public:
  TDAQguiCtrl(TDAQsupervise*, const TGWindow* );
  virtual ~TDAQguiCtrl();
  TGGroupFrame* SetGroup( TGCompositeFrame*, Int_t, Int_t, Int_t, Int_t,
			  ULong_t, ULong_t, TGFont*, const Char_t* );
  TGTextButton* SetButton( TGGroupFrame*, Int_t, Int_t, Int_t, Int_t,
			   ULong_t, ULong_t, TGFont*, const Char_t*, const Char_t* );
  TGNumberEntry* SetNumber( TGGroupFrame*, Int_t, Int_t, Int_t, Int_t,
			    ULong_t, ULong_t, TGFont*, const Char_t*,
			    const Char_t*, Bool_t=kFALSE );
  void SetLabel( TGGroupFrame*, Int_t, Int_t, Int_t, Int_t,
		 ULong_t, ULong_t, TGFont*, const Char_t* );
  TGTextEntry* SetText( TGGroupFrame*, Int_t, Int_t, Int_t, Int_t,
			ULong_t, ULong_t, TGFont*, const Char_t* = NULL, 
			Int_t = 0, const Char_t* = NULL );
  void Start();
  void Go();
  void ShowRun();
  void ExecRun();
  void Pause();
  void Stop();
  void End();
  void Kill();
  void Dump();
  void Update();
  void LoadMsgBuffer(const Char_t*);
  void SaveRun();
  void SaveComment();
  void SaveExperiment();
  void SaveFile();
  void SaveNumber( TGTextEntry*, Int_t );
  void GetAnalysis();
  void GetTrigger();
  void CheckStore();
  void CheckAuto();
  void UpdateStartBuff();
  void IssueCmd();
  void ResetEvent();
  void AutoRun(){ fAutoRun = kTRUE; while(fAutoRun)usleep(1000); }
  void AutoEnd(){ fAutoEnd = kTRUE; while(fAutoEnd)usleep(1000); }
  void EnCPU();
  void IntDel();
  void FCDel();
  void RAM();
  void InMask();
  void InPre();
  void L1Pre();
  void L2Pre();
  void SetDeb();
  void SetChan();
  void SetL1Width();
  void SetDelay();
  void SetMThresh();
  Bool_t IsRunning(){ return fSupervise->IsRunning(); }
  ClassDef(TDAQguiCtrl,1)
    };

#endif

//--Author	JRM Annand    9th Jan 2003
//--Rev		JRM Annand...26th Feb 2003...1st "production" version
//--Rev 	JRM Annand...21st Jan 2007...Included in TA2System
//--Update	JRM Annand... 8th Feb 2013...Add EPICS buffer delimiter
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// EnumConst.h
//
// Constants used by AcquRoot

#ifndef _EnumConst_h_
#define _EnumConst_h_


enum { EMk1 = 1, EMk2 };               // Acqu data formats
enum { EMaxCmdList = 32 };             // max number different command lists
enum { ELineSize = 1024 };             // length in characters of line of text
enum { EKeyWordSize = 256 };           // length in characters of keyword
enum { EErrNonFatal, EErrFatal };      // error level
enum { ENOKeyWord, EKeyWord };         // Config file readin...line format
enum { ENsem = 16 };                   // # of semaphores	               
enum { EHostLen = 64 };	               // max length of host name (socket)
enum { ESkDefPacket = 1024 };   // Default transfer length for stream socket
enum { ESkLocal, ESkRemote };   // local/remote socket connect
enum { ESkBacklog = 8 };        // max length pending connection queue (socket)
enum { ESkInitBuff = 2 };       // # ints in initial handshake buffer (socket)
enum { EMaxDataLength = 524288 };// max data buffer length
enum { EMaxInputFiles = 1024 }; // size of input-file pointer buffer
enum { EFalse, ETrue };         // Logic...should use kTRUE, kFALSE

// ACQU Mk1 data buffer headers are unsigned 32-bit words.  Keep them
// separate from the signed analysis sentinels below; mixing both groups in
// one enum makes the enum unsigned and silently converts -1 sentinels when
// they are assigned to Int_t/Short_t buffers.
constexpr unsigned int EHeadBuff      = 0x10101010u;
constexpr unsigned int EDataBuff      = 0x20202020u;
constexpr unsigned int EEndBuff       = 0x30303030u;
constexpr unsigned int EKillBuff      = 0x40404040u;
constexpr unsigned int EPhysBuff      = 0x50505050u;
constexpr unsigned int EHeadPhysBuff  = 0x60606060u;
constexpr unsigned int EMk2DataBuff   = 0x70707070u;
constexpr unsigned int EEndEvent      = 0xFFFFFFFFu;
constexpr unsigned int EScalerBuffer  = 0xFEFEFEFEu;
constexpr unsigned int EEPICSBuffer   = 0xFDFDFDFDu;
constexpr unsigned int EReadError     = 0xEFEFEFEFu;

constexpr int EBufferEnd = -1;       // end marker in signed analysis buffers
constexpr int ENullADC   = -1;       // undefined ADC value
constexpr int ENullHit   = -1;       // undefined hit index
constexpr short ENullStore = -32768; // 0x8000 in a signed 16-bit store
constexpr int ENullFloat = -999999999;

// Constants for ROOT storage and analysis
enum{ EMaxEventSize = 524288, EMaxName = 256 };

// Definitions for ADC setup
enum{ EUndefinedADC = 0,          // ADC index not registered in analysis
      EPatternADC = 0xffff,       // its a bit-pattern unit
      EForeignADC = 0x3000,       // foreign data formats
      EForeignScaler = 0x4000,    // ditto
      EMultiADC = 0x10000,        // multi-hit ADC
      EFlashADC = 0x20000         // flash ADC
}; 

#endif

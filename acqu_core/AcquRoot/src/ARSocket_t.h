//--Author	JRM Annand   19th Jan 2004   Adapt from TAcquSocket
//--Rev 	JRM Annand...21st Jan 2007   4v0 update adapt from TA2Socket
//--Rev 	JRM Annand....1st Feb 2007   Feedback from TDAQsocket
//--Rev 	JRM Annand....7th Dec 2007   Test read with select
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Update	JRM Annand...22nd Oct 2009...fRetry = 5
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// ARSocket_t
// 
//  Wrapper class for TCP/IP network socket. Used to read/write data from/to
//  remote computer node via the net. Long data buffers are "divided" into
//  chunks and the send/receive of each chunk is "handshaked"
//  between the connected nodes
//
//---------------------------------------------------------------------------

#ifndef _ARSocket_t_h_
#define _ARSocket_t_h_


#include "TA2System.h"
#include "EnumConst.h"			// integer constant defs
#include <unistd.h>
 

extern "C"{
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
}

//class ARSocket_t : public TA2System {
class ARSocket_t {
private:
  TA2System* fSys;              // calling class
  Int_t fID;		        // socket path or descriptor
  Int_t fPort;		        // port #...to communicate with desired remote
  Int_t fMode;		        // local or remote (client/server)
  Int_t fInit[ESkInitBuff];	// buffer and packet lengths
  Int_t fRetry;                 // # attempts to connect
  char* fHost;	                // host name
  fd_set fRfds;                 // for select
  timeval fTV;                  // timeout value
public:
  ARSocket_t( const Char_t*, const Char_t*, Int_t, Int_t = ESkLocal,
	      Int_t = ESkDefPacket, Int_t = ESkDefPacket, TA2System* = NULL );
  virtual ~ARSocket_t();
  virtual void Initialise();
  void KillSk();                            // shutdown, default fatal err
  Int_t ReadChunked( void*, Int_t, Int_t ); // read from remote
  void WriteChunked( void*, Int_t, Int_t ); // write to remote
  void HandShakeSk( Int_t );                // ensure both ends in synch
  Bool_t IsReady();
//
// Getters of private class members
//
  Int_t GetID(){ return fID; }		        // socket descriptor
  Int_t GetPort(){ return fPort; }	        // network port number
  Int_t GetMode(){ return fMode; }	        // transfer mode send/receive
  Int_t GetBuffLen(){ return fInit[0]; }        // data buffer length
  Int_t GetPacketLen(){ return fInit[1]; }      // net transfer packet length
  Char_t* GetHost(){ return fHost; }            // remote host name
  Int_t GetRetry(){ return fRetry; }            // connect retries
  void SetRetry( Int_t r ){ fRetry = r; }
  void PrintError( const Char_t*, Int_t = EErrNonFatal );

};

//---------------------------------------------------------------------------
inline void ARSocket_t::HandShakeSk( Int_t bytes )
{
// Return write to remote node through internet socket
//  if( fIsSwap ) bytes = Swap4Byte(bytes);
  if( write( fID, &bytes, sizeof(Int_t)) != sizeof(Int_t) )
    PrintError("<Socket Incompatible Handshake>",EErrFatal);
  return;
}

#endif

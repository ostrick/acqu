//--Author	JRM Annand   22nd Jun 2004
//--Rev 	JRM Annand... 5th Nov 2004 Some convenient getters
//--Rev 	JRM Annand...19th Jan 2005 bug-fix...init fNpat=0
//--Update	JRM Annand...29th Sep 2012 GetNHits[i]
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2BitPattern
// Decode bit-pattern-unit output to create a hit pattern
//

#ifndef __TA2BitPattern_h__
#define __TA2BitPattern_h__

#include "TA2System.h"

enum { ENPatterns, EPatternSize, EPatternDatum,EPatternDatumAuto  };
// Command-line default detector key words which determine
// what information is read in
static const Map_t kPatternDefaultKeys[] = {
  {"Number-Patterns:",ENPatterns},
  {"PatternSize:",EPatternSize},
  {"PatternADC:", EPatternDatum},
  {"PatternADCAuto:", EPatternDatumAuto},
  {NULL,          -1}
};
enum { EPatternADCsize = 16 };

class TA2BitPattern : public TA2System{
private:
  Char_t** fPatternName;
  Int_t** fADCList;          
  Int_t** fPatternList;
  UShort_t* fADC;
  Int_t** fHits;
  Int_t fNPattern;
  Int_t fNpat;
  Int_t* fNADC;
  Int_t fNadc;
  Int_t* fNelement;
  Int_t fNelem;
  UInt_t* fNHits;
public:
  TA2BitPattern( char*, UShort_t* );
  virtual ~TA2BitPattern();
  virtual void SetConfig( char*, int );
  virtual void Decode();
  Char_t** GetPatternName(){ return fPatternName; }
  Char_t* GetPatternName(Int_t i){ return fPatternName[i]; }
  Int_t GetNPattern(){ return fNPattern; }
  Int_t** GetADCList(){ return fADCList; }
  Int_t* GetADCList(Int_t i){ return fADCList[i]; }
  Int_t** GetPatternList(){ return fPatternList; }
  Int_t* GetPatternList(Int_t i){ return fPatternList[i]; }
  Int_t** GetHits(){ return fHits; }
  Int_t* GetHits(Int_t i){ return fHits[i]; }
  Int_t GetHits(Int_t i, Int_t j){ return fHits[i][j]; }
  Int_t* GetNADC(){ return fNADC; }
  Int_t GetNADC(Int_t i){ return fNADC[i]; }
  Int_t* GetNelement(){ return fNelement; }
  Int_t GetNelement(Int_t i){ return fNelement[i]; }
  UInt_t* GetNHits(){ return fNHits; }
  UInt_t* GetNHits(Int_t i){ return fNHits+i; }

  ClassDef(TA2BitPattern,1)
};

//---------------------------------------------------------------------------
inline void TA2BitPattern::Decode( )
{
  // Build set of patterns from pattern ADCs
  // Apply window cuts and return true/false to cut decision

  UShort_t datum;
  Int_t chan,n,i,j,k,nadc,nhits;
  Int_t* adclist;
  Int_t* pattlist;
  Int_t* hits;

  // loop round separate pattern spectra
  for( n=0; n<fNPattern; n++ ){
    adclist = fADCList[n];
    pattlist = fPatternList[n];
    hits = fHits[n];
    nadc = fNADC[n];
    nhits = 0;
    // loop round ADCs related to single pattern spectrum
    for( i=0,k=0; i<nadc; i++ ){
      j = adclist[i];
      datum = fADC[j];
      if( (datum == (UShort_t)ENullADC) || (datum == 0) ) continue;
      k = i * EPatternADCsize;
      // loop round 16 bits of single ADC
      for( j=0; j<EPatternADCsize; j++ ){
	chan = pattlist[k++];
	if( chan != ENullADC ){    // channel turned on
	  if( (datum & 1) && (nhits < fNelement[n]) ){
	    hits[nhits++] = chan;
	  }
	}
	datum >>= 1;                        // next bit
	if( !datum ) break;                 // no set bits left
      }
    }
    hits[nhits] = EBufferEnd;               // hits buffer end marker
    fNHits[n] = nhits;
  }
}
#endif

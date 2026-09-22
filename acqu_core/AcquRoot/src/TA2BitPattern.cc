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

#include "TA2BitPattern.h"


//---------------------------------------------------------------------------
TA2BitPattern::TA2BitPattern( char* fname, UShort_t* adc ) :
  TA2System( fname, kPatternDefaultKeys )
{
  // Setup the pattern from config file
  fADC = adc;
  fNPattern = fNpat = 0;
  fPatternName = NULL;
  fNADC = fNelement = NULL;
  fADCList = NULL;
  fPatternList = NULL;
  fHits = NULL;
  fNHits = NULL;
  FileConfig(fname);
  return;
}

//---------------------------------------------------------------------------
TA2BitPattern::~TA2BitPattern( )
{
  // Free memory allocated for pattern arrays
  for( Int_t i = 0; i < fNpat; i++ ){
    delete[] fPatternName[i];
    delete[] fADCList[i];
    delete[] fPatternList[i];
    delete[] fHits[i];
  }
  delete[] fPatternName;
  delete[] fNADC;
  delete[] fNelement;
  delete[] fNHits;
  delete[] fADCList;
  delete[] fPatternList;
  delete[] fHits;
}

//---------------------------------------------------------------------------
void TA2BitPattern::SetConfig( char* line, int key )
{
  // Load basic bit-pattern parameters from file or command line
  Int_t i,j,iadc,nadc;
  Int_t ebit[EPatternADCsize];
  Char_t name[256];
  Int_t* adclist;
  Int_t* pattlist;
  switch( key ){
  case ENPatterns:
    // Number of bit patterns to create
    if( sscanf( line, "%d", &fNPattern ) != 1 ){
      PrintError(line,"<Number of bit pattern spectra>");
      return;
    }
    fPatternName = new Char_t*[fNPattern];
    fNADC = new Int_t[fNPattern];
    fNelement = new Int_t[fNPattern];
    fNHits  = new UInt_t[fNPattern];
    fADCList = new Int_t*[fNPattern];
    fPatternList  = new Int_t*[fNPattern];
    fHits = new Int_t*[fNPattern];
    break;
  case EPatternSize:
    // # of elements in individual hit pattern
    // check sufficient patterns declared to enter this one
    if( fNpat >= fNPattern ){
      PrintError(line,"<Too many pattern spectra input>");
      return;
    }
    if( sscanf( line,"%s%d%d",name,&nadc,&iadc ) != 3 ){
      PrintError(line,"<Bit-pattern size setup>");
      return;
    }
    fPatternName[fNpat] = new Char_t[strlen(name) + 1];
    strcpy(fPatternName[fNpat],name);
    if( (nadc <= 0) || (iadc <= 0) ){
      PrintError(line,"<Invalid bit-pattern size>");
      return;
    }
    fADCList[fNpat] = new Int_t[nadc];
    // Each ADC contributes 16 entries, including disabled (-1) bits.
    fPatternList[fNpat] = new Int_t[nadc * EPatternADCsize];
    // Reserve one additional entry for the EBufferEnd marker.
    fHits[fNpat] = new Int_t[iadc + 1];
    fNADC[fNpat] = nadc;
    fNelement[fNpat] = iadc;
    fNpat++;
    fNelem = fNadc = 0;
    break;
  case EPatternDatum:
    // parameters for each pattern ADC
    // input ADC index and 16 hit channel values..one for each bit of the ADC
    if( (fNpat <= 0) || (fNpat > fNPattern) ) return;
    if( sscanf( line,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",&iadc,
		ebit,   ebit+1, ebit+2, ebit+3,
		ebit+4, ebit+5, ebit+6, ebit+7,
		ebit+8, ebit+9, ebit+10,ebit+11,
		ebit+12,ebit+13,ebit+14,ebit+15 ) < 17 ){
      PrintError(line,"<Bit-pattern ADC setup>");
      return;
    }
    adclist = fADCList[fNpat-1];
    pattlist = fPatternList[fNpat-1];
    if( (fNadc >= fNADC[fNpat-1]) ||
        ((fNelem + EPatternADCsize) >
         (fNADC[fNpat-1] * EPatternADCsize)) ){
      PrintError(line,"<Too many bit-pattern ADC entries>");
      return;
    }
    adclist[fNadc++] = iadc;
    for( i=0; i<EPatternADCsize; i++ ){
      //      if( ebit[i] == ENullADC ) continue;
      pattlist[fNelem++] = ebit[i];
    }
    break;
  case EPatternDatumAuto:
    // parameters for pattern ADCs...assume regular progression
    // input start ADC index, number of ADCs in +1 progression and
    // start hit channel...again regular +1 progression
    if( (fNpat <= 0) || (fNpat > fNPattern) ) return;
    if( (i = sscanf( line,"%d%d%d",&iadc,&nadc,ebit )) < 3 ){
      PrintError(line,"<Auto Bit-pattern ADC setup>");
      return;
    }
    adclist = fADCList[fNpat-1];
    pattlist = fPatternList[fNpat-1];
    if( (nadc > (fNADC[fNpat-1] - fNadc)) ||
        ((nadc * EPatternADCsize) >
         (fNADC[fNpat-1] * EPatternADCsize - fNelem)) ){
      PrintError(line,"<Too many automatic bit-pattern ADC entries>");
      return;
    }
    for( i=0; i<nadc; i++ ){
      adclist[fNadc] = iadc + i;
      for( j=0; j<EPatternADCsize; j++ ) pattlist[fNelem++] = ebit[0] + j;
      fNadc++;
      ebit[0] += EPatternADCsize;
    }
    break;
  default:
    // Pass on any unrecognised line
    PrintError(line,"<Unrecognised command>");
    break;
  }
  return;
}
ClassImp(TA2BitPattern)

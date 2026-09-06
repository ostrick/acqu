// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AddSet.C                                                             //
//                                                                      //
// Add manually a new set to the database.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TError.h"
#include "TString.h"
#include "TSystem.h"


//______________________________________________________________________________
void AddSet(Int_t sourceSet = 0, Int_t firstRun = 32380, Int_t lastRun = 32389)
{
    TString program = "$CALIB/../build/bin/add_calib_sets";
    gSystem->ExpandPathName(program);
    if (gSystem->AccessPathName(program.Data(), kExecutePermission))
    {
        Error("AddSet", "Program '%s' is missing; rebuild AcquRoot first",
              program.Data());
        return;
    }

    TString command = TString::Format("'%s' %d %d %d '/rundata/2025-07_He'",
                                      program.Data(), sourceSet, firstRun, lastRun);
    const Int_t status = gSystem->Exec(command.Data());
    if (status != 0)
        Error("AddSet", "Creating the runsets failed (exit status %d)", status);
}

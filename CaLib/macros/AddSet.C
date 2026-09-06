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
    // Beamtime configuration
    const Char_t calibration[] = "2025-Compton-4He";
    const Char_t description[] = "Calibration of 2025 4He Beamtime";
    const Char_t rawfilePath[] = "/rundata/2025-07_He";
    const Char_t target[] = "LHe4";
    const Char_t* types[] = {
        "Type.Tagger.Time",
        "Type.CB.Time",
        "Type.CB.Energy",
        "Type.PID.Time"
    };

    TString program = "$CALIB/../build/bin/add_calib_sets";
    gSystem->ExpandPathName(program);
    if (gSystem->AccessPathName(program.Data(), kExecutePermission))
    {
        Error("AddSet", "Program '%s' is missing; rebuild AcquRoot first",
              program.Data());
        return;
    }

    TString calArg(calibration);
    TString descArg(description);
    TString pathArg(rawfilePath);
    TString targetArg(target);
    calArg.ReplaceAll("'", "'\\''");
    descArg.ReplaceAll("'", "'\\''");
    pathArg.ReplaceAll("'", "'\\''");
    targetArg.ReplaceAll("'", "'\\''");

    TString command = TString::Format("'%s' %d %d %d '%s' '%s' '%s' '%s'",
                                      program.Data(), sourceSet, firstRun, lastRun,
                                      pathArg.Data(), calArg.Data(), descArg.Data(),
                                      targetArg.Data());
    for (UInt_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
    {
        TString typeArg(types[i]);
        typeArg.ReplaceAll("'", "'\\''");
        command.Append(TString::Format(" '%s'", typeArg.Data()));
    }

    const Int_t status = gSystem->Exec(command.Data());
    if (status != 0)
        Error("AddSet", "Creating the runsets failed (exit status %d)", status);
}

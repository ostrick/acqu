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

#include "../include/TCConfig.h"
#include "../include/TCMySQLManager.h"

#include "TSystem.h"


//______________________________________________________________________________
void AddSet(Int_t sourceSet = 0, Int_t firstRun = 32380, Int_t lastRun = 32389)
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    const Char_t calibName[]        = "2025-Compton-4He";
    const Char_t calibDesc[]        = "Calibration of 2025 4He Beamtime";
    const Char_t rawfilePath[]      = "/rundata/2025-07_He";
    const Char_t target[]           = "LHe4";

    TCMySQLManager* manager = TCMySQLManager::GetManager();

    // Register only the runs needed as boundaries and input of this runset.
    // Existing run entries are left unchanged by the database's unique key.
    manager->AddRunFiles(rawfilePath, target, firstRun, lastRun);
    if (!manager->ContainsRun(firstRun) || !manager->ContainsRun(lastRun))
    {
        Error("AddSet", "Boundary runs %d and %d must exist in run_main; no runsets were copied",
              firstRun, lastRun);
        gSystem->Exit(1);
        return;
    }

    // Copy the selected existing runset for all calibration quantities needed
    // for Tagger time, CB time/energy and PID time.
    const Char_t* calibTypes[] = {
        "Type.Tagger.Time",
        "Type.CB.Time",
        "Type.CB.Energy",
        "Type.PID.Time"
    };

    Bool_t ok = kTRUE;
    for (UInt_t i = 0; i < sizeof(calibTypes) / sizeof(calibTypes[0]); ++i)
        if (!manager->CloneSet(calibTypes[i], calibName, calibDesc,
                               sourceSet, firstRun, lastRun))
            ok = kFALSE;

    if (!ok)
        Error("AddSet", "At least one runset could not be copied");

    gSystem->Exit(ok ? 0 : 1);
}

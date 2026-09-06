
//______________________________________________________________________________
void AddRuns()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // Adding the runfiles should be done by maintainers only!
    // add raw files to the database

    const Char_t rawfilePath[]      = "/rundata/2025-07_He";
    const Char_t target[]           = "LHe4";
    TCMySQLManager::GetManager()->AddRunFiles(rawfilePath, target);


    gSystem->Exit(0);
}


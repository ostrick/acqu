// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Install.C                                                            //
//                                                                      //
// Install the CaLib database.                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "../include/TCMySQLManager.h"


//______________________________________________________________________________
void Install()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // install CaLib
    TCMySQLManager* manager = TCMySQLManager::GetManager();
    if (!manager)
    {
        Error("Install", "Could not connect to the configured database; database was not modified");
        return;
    }
    manager->InitDatabase();
    
    gSystem->Exit(0);
}

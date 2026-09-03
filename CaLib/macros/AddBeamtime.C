// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AddBeamtime.C                                                        //
//                                                                      //
// Add a beamtime including raw data files and initial calibrations     //
// from AcquRoot configuration files to a CaLib database.               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void AddBeamtime()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // macro configuration: just change here for your beamtime and leave
    // the other parts of the code unchanged
//    const Int_t firstRun            = 31774;
//    const Int_t lastRun             = 32419;
    const Int_t firstRun            = 31840;
    const Int_t lastRun             = 31849;
    const Char_t calibName[]        = "2025-Compton-4He";
    const Char_t calibDesc[]        = "Calibration of 2025 4He Beamtime";
    const Char_t calibFileTagger[]  = "../acqu_user/data.2025/FPD_855_new.dat";
    const Char_t calibFileCB[]      = "../acqu_user/data.2025/Detector-NaI.dat";
//    const Char_t calibFileTAPS[]    = "../acqu_user/data.2025/Detector-BaF2-PbWO4.dat";
    const Char_t calibFilePID[]     = "../acqu_user/data.2025/Detector-PID.dat";
//    const Char_t calibFileVeto[]    = "../acqu_user/data.2025/Detector-Veto.dat";

 //    Adding the runfiles should be done by maintainers only!
 //    add raw files to the database

    const Char_t rawfilePath[]      = "/media/ostrick/INTENSO/rundata/";
    const Char_t target[]           = "LHe4";
    TCMySQLManager::GetManager()->AddRunFiles(rawfilePath, target);
    
    // set MC mode to pass 0s and 1s to the database for gains, offset, pedestals, etc.
    //TCMySQLManager::GetManager()->SetMC(true);

    // read AcquRoot calibration of tagger
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAGG, calibFileTagger,
                                             calibName, calibDesc,
                                             firstRun, lastRun);
    
    // init tagging efficiency table
    TCMySQLManager::GetManager()->AddSet("Type.Tagger.Eff", calibName, calibDesc,
                                         firstRun, lastRun, 0);
      
    // read AcquRoot calibration of CB
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_CB, calibFileCB,
                                             calibName, calibDesc,
                                             firstRun, lastRun);
    
    // init CB time walk calibration
    TCMySQLManager::GetManager()->AddSet("Type.CB.Time.Walk", calibName, calibDesc,
                                         firstRun, lastRun, 0);
     
    // init CB quadratic energy correction
    TCMySQLManager::GetManager()->AddSet("Type.CB.Energy.Quad", calibName, calibDesc,
                                         firstRun, lastRun, 0);
    
    // init CB LED calibration
    TCMySQLManager::GetManager()->AddSet("Type.CB.LED", calibName, calibDesc,
                                         firstRun, lastRun, 0);
  
    // read AcquRoot calibration of TAPS
  //  TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAPS, calibFileTAPS,
  //                                           calibName, calibDesc,
  //                                           firstRun, lastRun);
    
    // init TAPS quadratic energy correction
    //TCMySQLManager::GetManager()->AddSet("Type.TAPS.Energy.Quad", calibName, calibDesc,
    //                                     firstRun, lastRun, 0);
 
    // init TAPS LED calibration
    //TCMySQLManager::GetManager()->AddSet("Type.TAPS.LED1", calibName, calibDesc,
       //                                  firstRun, lastRun, 0);
    //TCMySQLManager::GetManager()->AddSet("Type.TAPS.LED2", calibName, calibDesc,
     //                                    firstRun, lastRun, 0);
 
    // read AcquRoot calibration of PID
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_PID, calibFilePID,
                                             calibName, calibDesc,
                                            firstRun, lastRun);
    
    // read AcquRoot calibration of Veto 
    //TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_VETO, calibFileVeto,
    //                                         calibName, calibDesc,
    //                                         firstRun, lastRun);
     
    gSystem->Exit(0);
}

// Compare the reconstructed photon in single-track events with the values
// expected from elastic Compton kinematics on He-3.
//
// Usage from ROOT:
//   root -l -q 'root/macros/AnalyzeMissE.C("output/Acqu_input.root")'
//
// The input file must contain the event-synchronous trees "tracks" and
// "tagger", as well as "setupParameters". Angles in the input and output are
// expressed in degrees.

#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <chrono>

void AnalyzeMissE(const char* inputFileName,
                  const char* outputFileName,
                  Double_t promptMin,
                  Double_t promptMax,
                  Double_t randomMin,
                  Double_t randomMax,
                  Double_t random2Min,
                  Double_t random2Max)
{
  const Int_t kMaxTracks = 128;
  const Int_t kMaxTaggerHits = 1024;
  const Int_t kMaxTaggerChannels = 1024;
  const Double_t M4He = 3727.4; // He-4 nuclear mass in MeV
  const Double_t acceleratorBeamEnergy = 855.0; // MeV

  const Double_t MTarget = M4He;



  TFile* inputFile = TFile::Open(inputFileName, "READ");
  if(!inputFile || inputFile->IsZombie())
    {
      std::cerr << "Cannot open input file: " << inputFileName << std::endl;
      delete inputFile;
      return;
    }

  TTree* tracks = dynamic_cast<TTree*>(inputFile->Get("tracks"));
  TTree* tagger = dynamic_cast<TTree*>(inputFile->Get("tagger"));
  TTree* setupParameters =
    dynamic_cast<TTree*>(inputFile->Get("setupParameters"));
  if(!tracks || !tagger || !setupParameters)
    {
      std::cerr << "Trees 'tracks', 'tagger' and/or 'setupParameters' "
                   "not found in "
                << inputFileName << std::endl;
      inputFile->Close();
      delete inputFile;
      return;
    }

  Int_t nTracks = 0;
  Double_t clusterEnergy[kMaxTracks] = {0.0};
  Double_t theta[kMaxTracks] = {0.0};
  Double_t phi[kMaxTracks] = {0.0};
  Int_t nTagged = 0;
  Int_t taggedChannel[kMaxTaggerHits] = {0};
  Double_t taggedTime[kMaxTaggerHits] = {0.0};
  Int_t nTagger = 0;
  Double_t taggerElectronEnergy[kMaxTaggerChannels] = {0.0};

  tracks->SetBranchAddress("nTracks", &nTracks);
  tracks->SetBranchAddress("clusterEnergy", clusterEnergy);
  tracks->SetBranchAddress("theta", theta);
  tracks->SetBranchAddress("phi", phi);
  tagger->SetBranchAddress("nTagged", &nTagged);
  tagger->SetBranchAddress("taggedChannel", taggedChannel);
  tagger->SetBranchAddress("taggedTime", taggedTime);
  setupParameters->SetBranchAddress("nTagger", &nTagger);
  setupParameters->SetBranchAddress("TaggerElectronEnergy",
                                    taggerElectronEnergy);

  if(setupParameters->GetEntries() < 1)
    {
      std::cerr << "Tree 'setupParameters' is empty" << std::endl;
      inputFile->Close();
      delete inputFile;
      return;
    }
  setupParameters->GetEntry(0);
  if(nTagger <= 0 || nTagger > kMaxTaggerChannels)
    {
      std::cerr << "Invalid number of tagger channels: " << nTagger
                << std::endl;
      inputFile->Close();
      delete inputFile;
      return;
    }

  if(tracks->GetEntries() != tagger->GetEntries())
    {
      std::cerr << "Trees 'tracks' and 'tagger' have different numbers of "
                   "entries" << std::endl;
      inputFile->Close();
      delete inputFile;
      return;
    }

  TFile outputFile(outputFileName, "RECREATE");
  if(outputFile.IsZombie())
    {
      std::cerr << "Cannot create output file: " << outputFileName << std::endl;
      inputFile->Close();
      delete inputFile;
      return;
    }

  TTree gammaTree("compton", "Single-photon Compton candidates");
  TLorentzVector gamma;
  Long64_t sourceEntry = -1;
  Double_t beamEnergy = 0.0;
  Double_t energyMeasured = 0.0;
  Double_t thetaMeasured = 0.0;
  Double_t energyKin = 0.0;
  Double_t thetaKin = 0.0;
  Double_t deltaEnergy = 0.0;
  Double_t taggerTime = 0.0;
  Int_t prompt = -1;

  gammaTree.Branch("sourceEntry", &sourceEntry, "sourceEntry/L");
  gammaTree.Branch("gamma", &gamma);
  gammaTree.Branch("beamEnergy", &beamEnergy, "beamEnergy/D");
  gammaTree.Branch("energyMeasured", &energyMeasured, "energyMeasured/D");
  gammaTree.Branch("thetaMeasured", &thetaMeasured, "thetaMeasured/D");
  gammaTree.Branch("energyKin", &energyKin, "energyKin/D");
  gammaTree.Branch("thetaKin", &thetaKin, "thetaKin/D");
  gammaTree.Branch("deltaEnergy", &deltaEnergy, "deltaEnergy/D");
  gammaTree.Branch("taggedTime", &taggerTime, "taggedTime/D");
  gammaTree.Branch("prompt", &prompt, "prompt/I");

  const Long64_t nEntries = tracks->GetEntries();
  Long64_t nSelected = 0;
  const std::chrono::steady_clock::time_point startTime =
    std::chrono::steady_clock::now();
  const auto reportProgress = [&](Long64_t events)
    {
      if(events % 10000 != 0)
        return;
      const double elapsedSeconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                      startTime).count();
      std::cout << "Events: " << events
                << ", Selected: " << nSelected
                << ", time: " << std::fixed << std::setprecision(1)
                << elapsedSeconds << " s" << std::endl;
    };
  for(Long64_t entry = 0; entry < nEntries; ++entry)
    {
      tracks->GetEntry(entry);
      tagger->GetEntry(entry);
      if(nTracks != 1)
        {
          reportProgress(entry + 1);
          continue;
        }
      if(nTagged < 0 || nTagged > kMaxTaggerHits)
        {
          std::cerr << "Invalid tagger multiplicity " << nTagged
                    << " in entry " << entry << std::endl;
          reportProgress(entry + 1);
          continue;
        }

      // GoAT stores angles in degrees. For a photon |p| = E.
      const Double_t thetaRad = theta[0] * TMath::DegToRad();
      const Double_t phiRad = phi[0] * TMath::DegToRad();
      energyMeasured = clusterEnergy[0];
      thetaMeasured = theta[0];
      const Double_t pSinTheta = energyMeasured * TMath::Sin(thetaRad);

      gamma.SetPxPyPzE(pSinTheta * TMath::Cos(phiRad),
                       pSinTheta * TMath::Sin(phiRad),
                       energyMeasured * TMath::Cos(thetaRad),
                       energyMeasured);

      if(energyMeasured <= 0.0)
        {
          reportProgress(entry + 1);
          continue;
        }

      for(Int_t i = 0; i < nTagged; ++i)
        {
          const Int_t channel = taggedChannel[i];
          if(channel < 0 || channel >= nTagger)
            continue;

          taggerTime = taggedTime[i];
          if(promptMin < taggerTime && taggerTime < promptMax)
            prompt = 1;
          else if((randomMin < taggerTime && taggerTime < randomMax) ||
                  (random2Min < taggerTime && taggerTime < random2Max))
            prompt = 0;
          else
            continue;

          beamEnergy = acceleratorBeamEnergy - taggerElectronEnergy[channel];
          if(beamEnergy <= 0.0)
            continue;

          energyKin = MTarget * beamEnergy /
            (MTarget + beamEnergy * (1.0 - TMath::Cos(thetaRad)));
          deltaEnergy = energyMeasured - energyKin;

          const Double_t acosArgument =
            1.0 - MTarget * (1.0 / energyMeasured - 1.0 / beamEnergy);
          if(acosArgument >= -1.0 && acosArgument <= 1.0)
            thetaKin = TMath::ACos(acosArgument) * TMath::RadToDeg();
          else
            thetaKin = std::numeric_limits<Double_t>::quiet_NaN();

          sourceEntry = entry;
          gammaTree.Fill();
          ++nSelected;
        }

      reportProgress(entry + 1);
    }

  outputFile.cd();
  gammaTree.Write();
  outputFile.Close();
  inputFile->Close();
  delete inputFile;

  std::cout << "Selected " << nSelected << " of " << nEntries
            << " events; wrote " << outputFileName << std::endl;
}

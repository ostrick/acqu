// Analyse the GoAT "tracks" tree and build a photon four-vector for events
// containing exactly one reconstructed track.
//
// Usage from ROOT:
//   root -l -q 'root/macros/SingleTrackGamma.C("output/Acqu_input.root")'
//
// The resulting file contains a tree named "singleTrackGamma" with one
// TLorentzVector per selected event.

#include "TFile.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>

void SingleTrackGamma(const char* inputFileName,
                      const char* outputFileName = "SingleTrackGamma.root")
{
  const Int_t kMaxTracks = 128;

  TFile* inputFile = TFile::Open(inputFileName, "READ");
  if(!inputFile || inputFile->IsZombie())
    {
      std::cerr << "Cannot open input file: " << inputFileName << std::endl;
      delete inputFile;
      return;
    }

  TTree* tracks = dynamic_cast<TTree*>(inputFile->Get("tracks"));
  if(!tracks)
    {
      std::cerr << "Tree 'tracks' not found in " << inputFileName << std::endl;
      inputFile->Close();
      delete inputFile;
      return;
    }

  Int_t nTracks = 0;
  Double_t clusterEnergy[kMaxTracks] = {0.0};
  Double_t theta[kMaxTracks] = {0.0};
  Double_t phi[kMaxTracks] = {0.0};

  tracks->SetBranchAddress("nTracks", &nTracks);
  tracks->SetBranchAddress("clusterEnergy", clusterEnergy);
  tracks->SetBranchAddress("theta", theta);
  tracks->SetBranchAddress("phi", phi);

  TFile outputFile(outputFileName, "RECREATE");
  if(outputFile.IsZombie())
    {
      std::cerr << "Cannot create output file: " << outputFileName << std::endl;
      inputFile->Close();
      delete inputFile;
      return;
    }

  TTree gammaTree("singleTrackGamma", "Photons from single-track events");
  TLorentzVector gamma;
  Long64_t sourceEntry = -1;

  gammaTree.Branch("sourceEntry", &sourceEntry, "sourceEntry/L");
  gammaTree.Branch("gamma", &gamma);

  const Long64_t nEntries = tracks->GetEntries();
  Long64_t nSelected = 0;
  for(Long64_t entry = 0; entry < nEntries; ++entry)
    {
      tracks->GetEntry(entry);
      if(nTracks != 1)
        continue;

      // GoAT stores angles in degrees. For a photon |p| = E.
      const Double_t thetaRad = theta[0] * TMath::DegToRad();
      const Double_t phiRad = phi[0] * TMath::DegToRad();
      const Double_t energy = clusterEnergy[0];
      const Double_t pSinTheta = energy * TMath::Sin(thetaRad);

      gamma.SetPxPyPzE(pSinTheta * TMath::Cos(phiRad),
                       pSinTheta * TMath::Sin(phiRad),
                       energy * TMath::Cos(thetaRad),
                       energy);

      sourceEntry = entry;
      gammaTree.Fill();
      ++nSelected;
    }

  outputFile.cd();
  gammaTree.Write();
  outputFile.Close();
  inputFile->Close();
  delete inputFile;

  std::cout << "Selected " << nSelected << " of " << nEntries
            << " events; wrote " << outputFileName << std::endl;
}

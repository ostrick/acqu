// Compare the reconstructed photon in single-track events with the values
// expected from elastic Compton kinematics on He-3.
//
// Usage from ROOT:
//   root -l -q 'root/macros/AnalyzeCompton.C("output/Acqu_input.root")'
//
// The input file must contain the event-synchronous trees "tracks" and
// "mcTruth". Angles in the input and output are expressed in degrees.

#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>
#include <limits>
#include <cmath>

void AnalyzeCompton(const char* inputFileName,
                    const char* outputFileName = "AnalyzeCompton.root")
{
  const Int_t kMaxTracks = 128;
  const Int_t kMaxMCParticles = 512;
  const Double_t M3He = 2808.3916; // He-3 nuclear mass in MeV

  TFile* inputFile = TFile::Open(inputFileName, "READ");
  if(!inputFile || inputFile->IsZombie())
    {
      std::cerr << "Cannot open input file: " << inputFileName << std::endl;
      delete inputFile;
      return;
    }

  TTree* tracks = dynamic_cast<TTree*>(inputFile->Get("tracks"));
  TTree* mcTruth = dynamic_cast<TTree*>(inputFile->Get("mcTruth"));
  if(!tracks || !mcTruth)
    {
      std::cerr << "Trees 'tracks' and/or 'mcTruth' not found in "
                << inputFileName << std::endl;
      inputFile->Close();
      delete inputFile;
      return;
    }

  Int_t nTracks = 0;
  Double_t clusterEnergy[kMaxTracks] = {0.0};
  Double_t theta[kMaxTracks] = {0.0};
  Double_t phi[kMaxTracks] = {0.0};
  Float_t beam[5] = {0.0};
  Int_t npart = 0;
  Float_t plab[kMaxMCParticles] = {0.0};
  Float_t klab[kMaxMCParticles] = {0.0};
  Float_t dircos[kMaxMCParticles][3] = {{0.0}};

  tracks->SetBranchAddress("nTracks", &nTracks);
  tracks->SetBranchAddress("clusterEnergy", clusterEnergy);
  tracks->SetBranchAddress("theta", theta);
  tracks->SetBranchAddress("phi", phi);
  mcTruth->SetBranchAddress("beam", beam);
  mcTruth->SetBranchAddress("npart", &npart);
  mcTruth->SetBranchAddress("plab", plab);
  mcTruth->SetBranchAddress("klab", klab);
  mcTruth->SetBranchAddress("dircos", dircos);

  if(tracks->GetEntries() != mcTruth->GetEntries())
    {
      std::cerr << "Trees 'tracks' and 'mcTruth' have different numbers of entries"
                << std::endl;
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
  Double_t energyMCTrue = 0.0;
  Double_t thetaKin = 0.0;
  Double_t thetaMCTrue = 0.0;
  Double_t deltaEnergy = 0.0;
  Double_t deltaEnergyMCTrue = 0.0;
  Double_t deltaTheta = 0.0;
  Double_t deltaThetaMCTrue = 0.0;
  Int_t mcPhotonIndex = -1;

  gammaTree.Branch("sourceEntry", &sourceEntry, "sourceEntry/L");
  gammaTree.Branch("gamma", &gamma);
  gammaTree.Branch("beamEnergy", &beamEnergy, "beamEnergy/D");
  gammaTree.Branch("energyMeasured", &energyMeasured, "energyMeasured/D");
  gammaTree.Branch("thetaMeasured", &thetaMeasured, "thetaMeasured/D");
  gammaTree.Branch("energyKin", &energyKin, "energyKin/D");
  gammaTree.Branch("energyMCTrue", &energyMCTrue, "energyMCTrue/D");
  gammaTree.Branch("thetaKin", &thetaKin, "thetaKin/D");
  gammaTree.Branch("thetaMCTrue", &thetaMCTrue, "thetaMCTrue/D");
  gammaTree.Branch("mcPhotonIndex", &mcPhotonIndex, "mcPhotonIndex/I");
  gammaTree.Branch("deltaEnergy", &deltaEnergy, "deltaEnergy/D");
  gammaTree.Branch("deltaEnergyMCTrue", &deltaEnergyMCTrue,
                   "deltaEnergyMCTrue/D");
  gammaTree.Branch("deltaTheta", &deltaTheta, "deltaTheta/D");
  gammaTree.Branch("deltaThetaMCTrue", &deltaThetaMCTrue,
                   "deltaThetaMCTrue/D");

  TH2D hEnergyComparison("hEnergyComparison",
                         "Photon energy;E_{kin} [MeV];E_{meas} [MeV]",
                         500, 0.0, 1000.0, 500, 0.0, 1000.0);
  TH2D hThetaComparison("hThetaComparison",
                        "Photon angle;#theta_{kin} [deg];#theta_{meas} [deg]",
                        180, 0.0, 180.0, 180, 0.0, 180.0);
  TH1D hDeltaEnergy("hDeltaEnergy", "E_{meas}-E_{kin};#Delta E [MeV];Events",
                    400, -400.0, 400.0);
  TH2D hEnergyMeasuredVsMCTrue(
    "hEnergyMeasuredVsMCTrue",
    "Measured and generated photon energy;E_{MC true} [MeV];E_{meas} [MeV]",
    500, 0.0, 1000.0, 500, 0.0, 1000.0);
  TH1D hDeltaEnergyMCTrue(
    "hDeltaEnergyMCTrue",
    "E_{meas}-E_{MC true};#Delta E [MeV];Events",
    400, -400.0, 400.0);
  TH1D hDeltaTheta("hDeltaTheta",
                   "#theta_{meas}-#theta_{kin};#Delta#theta [deg];Events",
                   360, -180.0, 180.0);
  TH2D hThetaMeasuredVsMCTrue(
    "hThetaMeasuredVsMCTrue",
    "Measured and generated photon angle;#theta_{MC true} [deg];#theta_{meas} [deg]",
    180, 0.0, 180.0, 180, 0.0, 180.0);
  TH2D hThetaKinVsMCTrue(
    "hThetaKinVsMCTrue",
    "Kinematic and generated photon angle;#theta_{MC true} [deg];#theta_{kin} [deg]",
    180, 0.0, 180.0, 180, 0.0, 180.0);
  TH1D hDeltaThetaMCTrue(
    "hDeltaThetaMCTrue",
    "#theta_{meas}-#theta_{MC true};#Delta#theta [deg];Events",
    360, -180.0, 180.0);

  const Long64_t nEntries = tracks->GetEntries();
  Long64_t nSelected = 0;
  for(Long64_t entry = 0; entry < nEntries; ++entry)
    {
      tracks->GetEntry(entry);
      mcTruth->GetEntry(entry);
      if(nTracks != 1)
        continue;

      // GoAT stores angles in degrees. For a photon |p| = E.
      const Double_t thetaRad = theta[0] * TMath::DegToRad();
      const Double_t phiRad = phi[0] * TMath::DegToRad();
      energyMeasured = clusterEnergy[0];
      thetaMeasured = theta[0];
      // Geant stores beam[3] in GeV, whereas GoAT cluster energies and M3He
      // are expressed in MeV.
      beamEnergy = 1000.0 * beam[3];
      const Double_t pSinTheta = energyMeasured * TMath::Sin(thetaRad);

      gamma.SetPxPyPzE(pSinTheta * TMath::Cos(phiRad),
                       pSinTheta * TMath::Sin(phiRad),
                       energyMeasured * TMath::Cos(thetaRad),
                       energyMeasured);

      // thetaKin uses the measured photon energy; energyKin uses its measured
      // angle. Both expressions require energies in the same units as M3He.
      if(energyMeasured <= 0.0 || beamEnergy <= 0.0)
        continue;

      // Locate the generated photon using E = |p|. Geant stores plab in
      // GeV/c and klab in MeV in this file format.
      mcPhotonIndex = -1;
      Double_t bestPhotonDifference = std::numeric_limits<Double_t>::max();
      const Int_t nMCParticles = npart < kMaxMCParticles ? npart : kMaxMCParticles;
      for(Int_t i = 0; i < nMCParticles; ++i)
        {
          const Double_t difference = std::fabs(1000.0 * plab[i] - klab[i]);
          if(difference < bestPhotonDifference)
            {
              bestPhotonDifference = difference;
              mcPhotonIndex = i;
            }
        }
      if(mcPhotonIndex < 0)
        continue;

      Double_t trueCosTheta = dircos[mcPhotonIndex][2];
      if(trueCosTheta < -1.0) trueCosTheta = -1.0;
      if(trueCosTheta > 1.0) trueCosTheta = 1.0;
      thetaMCTrue = TMath::ACos(trueCosTheta) * TMath::RadToDeg();
      energyMCTrue = klab[mcPhotonIndex];

//      const Double_t acosArgument =
//        1.0 - M3He * (1.0 / energyMeasured - 1.0 / beamEnergy);
//      if(acosArgument < -1.0 || acosArgument > 1.0)
//        continue;

//      thetaKin = TMath::ACos(acosArgument) * TMath::RadToDeg();
      energyKin = M3He * beamEnergy /
        (M3He + beamEnergy * (1.0 - TMath::Cos(thetaRad)));
      deltaEnergy = energyMeasured - energyKin;
      deltaEnergyMCTrue = energyMeasured - energyMCTrue;
      deltaTheta = thetaMeasured - thetaKin;
      deltaThetaMCTrue = thetaMeasured - thetaMCTrue;

      sourceEntry = entry;
      gammaTree.Fill();
      hEnergyComparison.Fill(energyKin, energyMeasured);
      hThetaComparison.Fill(thetaKin, thetaMeasured);
      hDeltaEnergy.Fill(deltaEnergy);
      hEnergyMeasuredVsMCTrue.Fill(energyMCTrue, energyMeasured);
      hDeltaEnergyMCTrue.Fill(deltaEnergyMCTrue);
      hDeltaTheta.Fill(deltaTheta);
      hThetaMeasuredVsMCTrue.Fill(thetaMCTrue, thetaMeasured);
      hThetaKinVsMCTrue.Fill(thetaMCTrue, thetaKin);
      hDeltaThetaMCTrue.Fill(deltaThetaMCTrue);
      ++nSelected;
    }

  outputFile.cd();
  gammaTree.Write();
  hEnergyComparison.Write();
  hThetaComparison.Write();
  hDeltaEnergy.Write();
  hEnergyMeasuredVsMCTrue.Write();
  hDeltaEnergyMCTrue.Write();
  hDeltaTheta.Write();
  hThetaMeasuredVsMCTrue.Write();
  hThetaKinVsMCTrue.Write();
  hDeltaThetaMCTrue.Write();
  outputFile.Close();
  inputFile->Close();
  delete inputFile;

  std::cout << "Selected " << nSelected << " of " << nEntries
            << " events; wrote " << outputFileName << std::endl;
}

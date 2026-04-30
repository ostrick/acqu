#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "TTree.h"

#include <iostream>

namespace {

Double_t SafeThetaDeg(Double_t px, Double_t py, Double_t pz)
{
  const Double_t p = TMath::Sqrt(px*px + py*py + pz*pz);
  if( p <= 0.0 ) return 0.0;
  Double_t ctheta = pz / p;
  if( ctheta > 1.0 ) ctheta = 1.0;
  if( ctheta < -1.0 ) ctheta = -1.0;
  return TMath::ACos(ctheta) * TMath::RadToDeg();
}

Double_t PhiDeg(Double_t px, Double_t py)
{
  return TMath::ATan2(py, px) * TMath::RadToDeg();
}

}

void AcquMCOutputKinematics(const Char_t* inputName = "MC-Output.root",
                            const Char_t* outputName = "MC-Output-kin.root",
                            Long64_t maxEntries = -1)
{
  TFile* input = TFile::Open(inputName, "READ");
  if( !input || input->IsZombie() ){
    std::cerr << "Cannot open input file: " << inputName << std::endl;
    return;
  }

  TTree* h1 = (TTree*)input->Get("h1");
  if( !h1 ){
    std::cerr << "Cannot find tree 'h1' in: " << inputName << std::endl;
    input->Close();
    return;
  }

  Float_t pxDir_l0101 = 0.0;
  Float_t pyDir_l0101 = 0.0;
  Float_t pzDir_l0101 = 0.0;
  Float_t p_l0101 = 0.0;
  Float_t e_l0101 = 0.0;
  Float_t eBeam = 0.0;

  Float_t pxDir_l0247 = 0.0;
  Float_t pyDir_l0247 = 0.0;
  Float_t pzDir_l0247 = 0.0;
  Float_t p_l0247 = 0.0;
  Float_t e_l0247 = 0.0;

  h1->SetBranchAddress("En_bm", &eBeam);

  h1->SetBranchAddress("Px_l0101", &pxDir_l0101);
  h1->SetBranchAddress("Py_l0101", &pyDir_l0101);
  h1->SetBranchAddress("Pz_l0101", &pzDir_l0101);
  h1->SetBranchAddress("Pt_l0101", &p_l0101);
  h1->SetBranchAddress("En_l0101", &e_l0101);

  h1->SetBranchAddress("Px_l0247", &pxDir_l0247);
  h1->SetBranchAddress("Py_l0247", &pyDir_l0247);
  h1->SetBranchAddress("Pz_l0247", &pzDir_l0247);
  h1->SetBranchAddress("Pt_l0247", &p_l0247);
  h1->SetBranchAddress("En_l0247", &e_l0247);

  TFile* output = TFile::Open(outputName, "RECREATE");
  if( !output || output->IsZombie() ){
    std::cerr << "Cannot create output file: " << outputName << std::endl;
    input->Close();
    return;
  }

  Double_t energyBeam = 0.0;

  Double_t px_l0101 = 0.0;
  Double_t py_l0101 = 0.0;
  Double_t pz_l0101 = 0.0;
  Double_t energy_l0101 = 0.0;
  Double_t kinetic_l0101 = 0.0;
  Double_t theta_l0101 = 0.0;
  Double_t phi_l0101 = 0.0;

  Double_t px_l0247 = 0.0;
  Double_t py_l0247 = 0.0;
  Double_t pz_l0247 = 0.0;
  Double_t energy_l0247 = 0.0;
  Double_t kinetic_l0247 = 0.0;
  Double_t theta_l0247 = 0.0;
  Double_t phi_l0247 = 0.0;

  TTree* outTree = new TTree("kinematics", "AcquMC particle kinematics");
  outTree->Branch("E-Beam", &energyBeam, "E-Beam/D");

  outTree->Branch("Px_gamma", &px_l0101, "Px_gamma/D");
  outTree->Branch("Py_gamma", &py_l0101, "Py_gamma/D");
  outTree->Branch("Pz_gamma", &pz_l0101, "Pz_gamma/D");
  outTree->Branch("E_gamma", &energy_l0101, "E_gamma/D");
  outTree->Branch("T_gamma", &kinetic_l0101, "T_gamma/D");
  outTree->Branch("Theta_gamma", &theta_l0101, "Theta_gamma/D");
  outTree->Branch("Phi_gamma", &phi_l0101, "Phi_gamma/D");

  outTree->Branch("Px_He", &px_l0247, "Px_He/D");
  outTree->Branch("Py_He", &py_l0247, "Py_He/D");
  outTree->Branch("Pz_He", &pz_l0247, "Pz_He/D");
  outTree->Branch("E_He", &energy_l0247, "E_He/D");
  outTree->Branch("T_He", &kinetic_l0247, "T_He/D");
  outTree->Branch("Theta_He", &theta_l0247, "Theta_He/D");
  outTree->Branch("Phi_He", &phi_l0247, "Phi_He/D");

  TH1D* hEnergy_gamma = new TH1D("hEnergy_gamma", "gamma energy;E [GeV];Counts", 400, 0.0, 1.0);
  TH1D* hTheta_gamma = new TH1D("hTheta_gamma", "gamma polar angle;#theta [deg];Counts", 180, 0.0, 180.0);
  TH1D* hPhi_gamma = new TH1D("hPhi_gamma", "gamma azimuth angle;#phi [deg];Counts", 360, -180.0, 180.0);

  TH1D* hEnergy_He = new TH1D("hEnergy_He", "He kinetic energy;T [GeV];Counts", 400, 0.0, 1.0);
  TH1D* hTheta_He = new TH1D("hTheta_He", "He polar angle;#theta [deg];Counts", 180, 0.0, 180.0);
  TH1D* hPhi_He = new TH1D("hPhi_He", "He azimuth angle;#phi [deg];Counts", 360, -180.0, 180.0);

  const Double_t alphaMass = 3.727417;
  Long64_t entries = h1->GetEntries();
  if( maxEntries >= 0 && maxEntries < entries ) entries = maxEntries;

  for( Long64_t i=0; i<entries; i++ ){
    h1->GetEntry(i);
    energyBeam = eBeam;

    px_l0101 = pxDir_l0101 * p_l0101;
    py_l0101 = pyDir_l0101 * p_l0101;
    pz_l0101 = pzDir_l0101 * p_l0101;
    energy_l0101 = e_l0101;
    kinetic_l0101 = e_l0101;
    theta_l0101 = SafeThetaDeg(px_l0101, py_l0101, pz_l0101);
    phi_l0101 = PhiDeg(px_l0101, py_l0101);

    px_l0247 = pxDir_l0247 * p_l0247;
    py_l0247 = pyDir_l0247 * p_l0247;
    pz_l0247 = pzDir_l0247 * p_l0247;
    energy_l0247 = e_l0247;
    kinetic_l0247 = e_l0247 - alphaMass;
    theta_l0247 = SafeThetaDeg(px_l0247, py_l0247, pz_l0247);
    phi_l0247 = PhiDeg(px_l0247, py_l0247);

    outTree->Fill();
    hEnergy_gamma->Fill(energy_l0101);
    hTheta_gamma->Fill(theta_l0101);
    hPhi_gamma->Fill(phi_l0101);
    hEnergy_He->Fill(kinetic_l0247);
    hTheta_He->Fill(theta_l0247);
    hPhi_He->Fill(phi_l0247);
  }

  TCanvas* canvas = new TCanvas("cAcquMCKinematics", "AcquMC kinematics", 1200, 800);
  canvas->Divide(3, 2);
  canvas->cd(1); hEnergy_gamma->Draw();
  canvas->cd(2); hTheta_gamma->Draw();
  canvas->cd(3); hPhi_gamma->Draw();
  canvas->cd(4); hEnergy_He->Draw();
  canvas->cd(5); hTheta_He->Draw();
  canvas->cd(6); hPhi_He->Draw();

  output->Write();
  output->Close();
  input->Close();

  std::cout << "Processed " << entries << " events from " << inputName << std::endl;
  std::cout << "Wrote " << outputName << std::endl;
}

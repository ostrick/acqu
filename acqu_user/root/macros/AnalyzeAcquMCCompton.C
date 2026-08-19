#include <TFile.h>
#include <TTree.h>
#include <TMath.h>

#include <algorithm>
#include <cmath>
#include <iostream>

void AnalyzeAcquMCCompton(
    const char* inputName = "/home/ostrick/Work/AcquMC/AcquMC-Output/AcquMC-3He-Compton.root",
    const char* outputName = "/home/ostrick/Work/AcquMC/AcquMC-Output/AcquMC-Compton-kinematics.root")
{
    TFile input(inputName, "READ");
    if (input.IsZombie()) {
        std::cerr << "Fehler: Eingabedatei '" << inputName
                  << "' konnte nicht geoeffnet werden.\n";
        return;
    }

    TTree* inputTree = nullptr;
    input.GetObject("h1", inputTree);
    if (!inputTree) {
        std::cerr << "Fehler: TTree/TNtuple 'h1' wurde in '" << inputName
                  << "' nicht gefunden.\n";
        return;
    }

    // AcquMC speichert Px, Py und Pz in diesem Ntuple als
    // Richtungskosinusse p_i/|p|, nicht als Impulskomponenten.
    Float_t beamEnergyIn = 0.0F;
    Float_t alphaPz = 0.0F;
    Float_t alphaPy = 0.0F;
    Float_t alphaPx = 0.0F;
    Float_t alphaEnergy = 0.0F;
    Float_t gammaPz = 0.0F;
    Float_t gammaPy = 0.0F;
    Float_t gammaPx = 0.0F;
    Float_t gammaEnergy = 0.0F;

    inputTree->SetBranchAddress("En_bm", &beamEnergyIn);
    inputTree->SetBranchAddress("Px_l0149", &alphaPx);
    inputTree->SetBranchAddress("Py_l0149", &alphaPy);
    inputTree->SetBranchAddress("Pz_l0149", &alphaPz);
    inputTree->SetBranchAddress("En_l0149", &alphaEnergy);
    inputTree->SetBranchAddress("Px_l0201", &gammaPx);
    inputTree->SetBranchAddress("Py_l0201", &gammaPy);
    inputTree->SetBranchAddress("Pz_l0201", &gammaPz);
    inputTree->SetBranchAddress("En_l0201", &gammaEnergy);

    TFile output(outputName, "RECREATE");
    if (output.IsZombie()) {
        std::cerr << "Fehler: Ausgabedatei '" << outputName
                  << "' konnte nicht angelegt werden.\n";
        return;
    }

    Double_t beamEnergy = 0.0;
    Double_t alphaKineticEnergy = 0.0;
    Double_t alphaTheta = 0.0;
    Double_t alphaPhi = 0.0;
    Double_t gammaKineticEnergy = 0.0;
    Double_t gammaTheta = 0.0;
    Double_t gammaPhi = 0.0;

    TTree result("ComptonKinematics", "gamma + 4He -> gamma + 4He");
    result.Branch("BeamEnergy", &beamEnergy, "BeamEnergy/D");
    result.Branch("AlphaKineticEnergy", &alphaKineticEnergy,
                  "AlphaKineticEnergy/D");
    result.Branch("AlphaTheta", &alphaTheta, "AlphaTheta/D");
    result.Branch("AlphaPhi", &alphaPhi, "AlphaPhi/D");
    result.Branch("GammaKineticEnergy", &gammaKineticEnergy,
                  "GammaKineticEnergy/D");
    result.Branch("GammaTheta", &gammaTheta, "GammaTheta/D");
    result.Branch("GammaPhi", &gammaPhi, "GammaPhi/D");

    // PDG-Masse des 4He-Kerns in GeV/c^2. AcquMC verwendet GeV-Einheiten.
    constexpr Double_t alphaMass = 3.727379378;

    const auto polarAngleDeg = [](Double_t px, Double_t py, Double_t pz) {
        const Double_t norm = std::sqrt(px * px + py * py + pz * pz);
        if (norm == 0.0)
            return 0.0;
        const Double_t cosTheta = std::clamp(pz / norm, -1.0, 1.0);
        return std::acos(cosTheta) * TMath::RadToDeg();
    };

    const auto azimuthDeg = [](Double_t px, Double_t py) {
        Double_t phi = std::atan2(py, px) * TMath::RadToDeg();
        if (phi < 0.0)
            phi += 360.0;
        return phi;
    };

    const Long64_t entries = inputTree->GetEntries();
    for (Long64_t i = 0; i < entries; ++i) {
        inputTree->GetEntry(i);

        beamEnergy = beamEnergyIn;
        alphaKineticEnergy = alphaEnergy - alphaMass;
        gammaKineticEnergy = gammaEnergy; // m_gamma = 0

        alphaTheta = polarAngleDeg(alphaPx, alphaPy, alphaPz);
        alphaPhi = azimuthDeg(alphaPx, alphaPy);
        gammaTheta = polarAngleDeg(gammaPx, gammaPy, gammaPz);
        gammaPhi = azimuthDeg(gammaPx, gammaPy);

        result.Fill();
    }

    output.cd();
    result.Write();
    output.Close();

    std::cout << entries << " Ereignisse nach '" << outputName
              << "' geschrieben.\n"
              << "Energien: GeV, Winkel: Grad\n";
}

#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
constexpr Int_t kEDetNaI = 1;
constexpr Int_t kEDetPID = 2;
constexpr Int_t kEDetMWPC = 4;
constexpr Int_t kMaxTaggerChannels = 4096;

struct Config
{
  std::vector<std::string> inputFiles;
  std::string outputFile;
  Double_t targetMass = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t pi0Mass = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t pi0MassMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t pi0MassMax = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t promptMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t promptMax = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t random1Min = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t random1Max = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t random2Min = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t random2Max = std::numeric_limits<Double_t>::quiet_NaN();
  Int_t missingEnergyBins = 0;
  Double_t missingEnergyMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t missingEnergyMax = std::numeric_limits<Double_t>::quiet_NaN();
  Int_t openingAngleBins = 0;
  Double_t openingAngleMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t openingAngleMax = std::numeric_limits<Double_t>::quiet_NaN();
  Int_t pi0MassBins = 0;
  Double_t pi0MassHistMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t pi0MassHistMax = std::numeric_limits<Double_t>::quiet_NaN();
  Int_t missingMassBins = 0;
  Double_t missingMassMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t missingMassMax = std::numeric_limits<Double_t>::quiet_NaN();
  Int_t pi0ThetaCMBins = 0;
  Double_t pi0ThetaCMMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t pi0ThetaCMMax = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t openingAngleCutMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t openingAngleCutMax = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t pi0MassCutMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t pi0MassCutMax = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t missingEnergyCutMin = std::numeric_limits<Double_t>::quiet_NaN();
  Double_t missingEnergyCutMax = std::numeric_limits<Double_t>::quiet_NaN();
  Int_t taggerChannelCutMin = -1;
  Int_t taggerChannelCutMax = -1;
  Long64_t progressInterval = 100000;
};

std::string Trim(const std::string& text)
{
  const std::string whitespace = " \t\r\n";
  const std::string::size_type first = text.find_first_not_of(whitespace);
  if(first == std::string::npos)
    return "";
  const std::string::size_type last = text.find_last_not_of(whitespace);
  return text.substr(first, last - first + 1);
}

Double_t ParseDouble(const std::string& value, const std::string& key,
                     unsigned int lineNumber)
{
  std::istringstream input(value);
  Double_t result = 0.0;
  std::string extra;
  if(!(input >> result) || (input >> extra) || !std::isfinite(result))
    throw std::runtime_error("line " + std::to_string(lineNumber) +
                             ": invalid value for " + key);
  return result;
}

Int_t ParseInt(const std::string& value, const std::string& key,
               unsigned int lineNumber)
{
  const Double_t parsed = ParseDouble(value, key, lineNumber);
  if(std::floor(parsed) != parsed || parsed < std::numeric_limits<Int_t>::min() ||
     parsed > std::numeric_limits<Int_t>::max())
    throw std::runtime_error("line " + std::to_string(lineNumber) +
                             ": " + key + " must be an integer");
  return static_cast<Int_t>(parsed);
}

Config ReadConfig(const std::string& fileName)
{
  std::ifstream input(fileName.c_str());
  if(!input)
    throw std::runtime_error("cannot open configuration file " + fileName);

  Config config;
  std::string line;
  unsigned int lineNumber = 0;
  while(std::getline(input, line))
    {
      ++lineNumber;
      const std::string::size_type comment = line.find('#');
      if(comment != std::string::npos)
        line.erase(comment);
      line = Trim(line);
      if(line.empty())
        continue;

      const std::string::size_type separator = line.find_first_of("=:");
      if(separator == std::string::npos)
        throw std::runtime_error("line " + std::to_string(lineNumber) +
                                 ": expected key = value");
      const std::string key = Trim(line.substr(0, separator));
      const std::string value = Trim(line.substr(separator + 1));
      if(key.empty() || value.empty())
        throw std::runtime_error("line " + std::to_string(lineNumber) +
                                 ": empty key or value");

      if(key == "input") config.inputFiles.push_back(value);
      else if(key == "output") config.outputFile = value;
      else if(key == "target_mass")
        config.targetMass = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_mass")
        config.pi0Mass = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_mass_min")
        config.pi0MassMin = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_mass_max")
        config.pi0MassMax = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_prompt_min")
        config.promptMin = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_prompt_max")
        config.promptMax = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_random1_min")
        config.random1Min = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_random1_max")
        config.random1Max = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_random2_min")
        config.random2Min = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_random2_max")
        config.random2Max = ParseDouble(value, key, lineNumber);
      else if(key == "missing_energy_hist_bins")
        config.missingEnergyBins = static_cast<Int_t>(ParseDouble(value, key, lineNumber));
      else if(key == "missing_energy_hist_min")
        config.missingEnergyMin = ParseDouble(value, key, lineNumber);
      else if(key == "missing_energy_hist_max")
        config.missingEnergyMax = ParseDouble(value, key, lineNumber);
      else if(key == "opening_angle_hist_bins")
        config.openingAngleBins = static_cast<Int_t>(ParseDouble(value, key, lineNumber));
      else if(key == "opening_angle_hist_min")
        config.openingAngleMin = ParseDouble(value, key, lineNumber);
      else if(key == "opening_angle_hist_max")
        config.openingAngleMax = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_mass_hist_bins")
        config.pi0MassBins = static_cast<Int_t>(ParseDouble(value, key, lineNumber));
      else if(key == "pi0_mass_hist_min")
        config.pi0MassHistMin = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_mass_hist_max")
        config.pi0MassHistMax = ParseDouble(value, key, lineNumber);
      else if(key == "missing_mass_hist_bins")
        config.missingMassBins = static_cast<Int_t>(ParseDouble(value, key, lineNumber));
      else if(key == "missing_mass_hist_min")
        config.missingMassMin = ParseDouble(value, key, lineNumber);
      else if(key == "missing_mass_hist_max")
        config.missingMassMax = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_theta_cm_hist_bins")
        config.pi0ThetaCMBins = static_cast<Int_t>(ParseDouble(value, key, lineNumber));
      else if(key == "pi0_theta_cm_hist_min")
        config.pi0ThetaCMMin = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_theta_cm_hist_max")
        config.pi0ThetaCMMax = ParseDouble(value, key, lineNumber);
      else if(key == "opening_angle_cut_min")
        config.openingAngleCutMin = ParseDouble(value, key, lineNumber);
      else if(key == "opening_angle_cut_max")
        config.openingAngleCutMax = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_mass_cut_min")
        config.pi0MassCutMin = ParseDouble(value, key, lineNumber);
      else if(key == "pi0_mass_cut_max")
        config.pi0MassCutMax = ParseDouble(value, key, lineNumber);
      else if(key == "missing_energy_cut_min")
        config.missingEnergyCutMin = ParseDouble(value, key, lineNumber);
      else if(key == "missing_energy_cut_max")
        config.missingEnergyCutMax = ParseDouble(value, key, lineNumber);
      else if(key == "tagger_channel_cut_min")
        config.taggerChannelCutMin = ParseInt(value, key, lineNumber);
      else if(key == "tagger_channel_cut_max")
        config.taggerChannelCutMax = ParseInt(value, key, lineNumber);
      else if(key == "progress_interval")
        {
          const Double_t parsed = ParseDouble(value, key, lineNumber);
          if(parsed < 0.0 || std::floor(parsed) != parsed)
            throw std::runtime_error("line " + std::to_string(lineNumber) +
                                     ": progress_interval must be a non-negative integer");
          config.progressInterval = static_cast<Long64_t>(parsed);
        }
      else
        throw std::runtime_error("line " + std::to_string(lineNumber) +
                                 ": unknown key " + key);
    }

  const auto requireFinite = [](Double_t value, const char* name)
    {
      if(!std::isfinite(value))
        throw std::runtime_error(std::string("missing parameter ") + name);
    };
  if(config.inputFiles.empty())
    throw std::runtime_error("at least one input entry is required");
  if(config.outputFile.empty())
    throw std::runtime_error("missing parameter output");
  requireFinite(config.targetMass, "target_mass");
  requireFinite(config.pi0Mass, "pi0_mass");
  requireFinite(config.pi0MassMin, "pi0_mass_min");
  requireFinite(config.pi0MassMax, "pi0_mass_max");
  requireFinite(config.promptMin, "pi0_prompt_min");
  requireFinite(config.promptMax, "pi0_prompt_max");
  requireFinite(config.random1Min, "pi0_random1_min");
  requireFinite(config.random1Max, "pi0_random1_max");
  requireFinite(config.random2Min, "pi0_random2_min");
  requireFinite(config.random2Max, "pi0_random2_max");
  requireFinite(config.missingEnergyMin, "missing_energy_hist_min");
  requireFinite(config.missingEnergyMax, "missing_energy_hist_max");
  requireFinite(config.openingAngleMin, "opening_angle_hist_min");
  requireFinite(config.openingAngleMax, "opening_angle_hist_max");
  requireFinite(config.pi0MassHistMin, "pi0_mass_hist_min");
  requireFinite(config.pi0MassHistMax, "pi0_mass_hist_max");
  requireFinite(config.missingMassMin, "missing_mass_hist_min");
  requireFinite(config.missingMassMax, "missing_mass_hist_max");
  requireFinite(config.pi0ThetaCMMin, "pi0_theta_cm_hist_min");
  requireFinite(config.pi0ThetaCMMax, "pi0_theta_cm_hist_max");
  requireFinite(config.openingAngleCutMin, "opening_angle_cut_min");
  requireFinite(config.openingAngleCutMax, "opening_angle_cut_max");
  requireFinite(config.pi0MassCutMin, "pi0_mass_cut_min");
  requireFinite(config.pi0MassCutMax, "pi0_mass_cut_max");
  requireFinite(config.missingEnergyCutMin, "missing_energy_cut_min");
  requireFinite(config.missingEnergyCutMax, "missing_energy_cut_max");

  if(config.targetMass <= 0.0)
    throw std::runtime_error("target_mass must be positive");
  if(config.pi0Mass <= 0.0)
    throw std::runtime_error("pi0_mass must be positive");
  if(config.pi0MassMin >= config.pi0MassMax)
    throw std::runtime_error("pi0_mass_min must be smaller than pi0_mass_max");
  if(config.promptMin >= config.promptMax ||
     config.random1Min >= config.random1Max ||
     config.random2Min >= config.random2Max)
    throw std::runtime_error("every time window must satisfy min < max");
  if(config.missingEnergyBins <= 0 ||
     config.missingEnergyMin >= config.missingEnergyMax)
    throw std::runtime_error("invalid missing-energy histogram binning");
  if(config.openingAngleBins <= 0 ||
     config.openingAngleMin >= config.openingAngleMax)
    throw std::runtime_error("invalid opening-angle histogram binning");
  if(config.pi0MassBins <= 0 || config.pi0MassHistMin >= config.pi0MassHistMax)
    throw std::runtime_error("invalid pi0-mass histogram binning");
  if(config.missingMassBins <= 0 || config.missingMassMin >= config.missingMassMax)
    throw std::runtime_error("invalid missing-mass histogram binning");
  if(config.pi0ThetaCMBins <= 0 || config.pi0ThetaCMMin >= config.pi0ThetaCMMax)
    throw std::runtime_error("invalid pi0-theta-CM histogram binning");
  if(config.openingAngleCutMin > config.openingAngleCutMax)
    throw std::runtime_error("opening_angle_cut_min must not exceed max");
  if(config.pi0MassCutMin > config.pi0MassCutMax)
    throw std::runtime_error("pi0_mass_cut_min must not exceed max");
  if(config.missingEnergyCutMin > config.missingEnergyCutMax)
    throw std::runtime_error("missing_energy_cut_min must not exceed max");
  if(config.taggerChannelCutMin < 0 ||
     config.taggerChannelCutMin > config.taggerChannelCutMax)
    throw std::runtime_error("invalid tagger-channel cut");

  return config;
}

bool InWindow(Double_t value, Double_t lower, Double_t upper)
{
  return value >= lower && value <= upper;
}

TLorentzVector PhotonP4(Double_t energy, Double_t thetaDeg, Double_t phiDeg)
{
  const Double_t theta = thetaDeg * TMath::DegToRad();
  const Double_t phi = phiDeg * TMath::DegToRad();
  const Double_t transverse = energy * std::sin(theta);
  return TLorentzVector(transverse * std::cos(phi),
                        transverse * std::sin(phi),
                        energy * std::cos(theta), energy);
}

bool CalculateMissingEnergy(const TLorentzVector& pi0, Double_t egamma,
                            Double_t targetMass, Double_t pi0Mass,
                            Double_t& missingEnergy, Double_t& pi0ThetaCM)
{
  if(egamma <= 0.0 || targetMass <= 0.0 || pi0Mass <= 0.0)
    return false;

  const TLorentzVector target(0.0, 0.0, 0.0, targetMass);
  const TLorentzVector beam(0.0, 0.0, egamma, egamma);
  const TLorentzVector totalCM = beam + target;

  TLorentzVector pi0CM = pi0;
  pi0CM.Boost(-totalCM.BoostVector());
  pi0ThetaCM = pi0CM.Theta() * TMath::RadToDeg();

  const Double_t s = targetMass * targetMass + 2.0 * egamma * targetMass;
  const Double_t sqrtS = std::sqrt(s);
  const Double_t expectedEnergy =
    (s + pi0Mass * pi0Mass - targetMass * targetMass) / (2.0 * sqrtS);
  missingEnergy = pi0CM.E() - expectedEnergy;
  return std::isfinite(missingEnergy) && std::isfinite(pi0ThetaCM);
}

double CoherentPionLabEnergy(double egamma, double thetaCmDeg,
                             double mpi0, double targetMass)
{
  const double s = targetMass*targetMass + 2.0*egamma*targetMass;
  const double sqrtS = std::sqrt(s);
  const double ePiCm =
    (s + mpi0*mpi0 - targetMass*targetMass)/(2.0*sqrtS);
  double pPiCm2 = ePiCm*ePiCm - mpi0*mpi0;
  if(pPiCm2 < 0.0) pPiCm2 = 0.0;
  const double pPiCm = std::sqrt(pPiCm2);

  const double betaCm = egamma/(egamma + targetMass);
  const double gammaCm = 1.0/std::sqrt(1.0 - betaCm*betaCm);
  const double cosThetaCm = std::cos(thetaCmDeg*TMath::DegToRad());

  return gammaCm*(ePiCm + betaCm*pPiCm*cosThetaCm);
}



double PhiMinDeg(double ePiLab, double mpi0)
{
  if(ePiLab <= mpi0) return 180.0;

  double arg = std::sqrt(ePiLab*ePiLab - mpi0*mpi0)/ePiLab;
  if(arg < -1.0) arg = -1.0;
  if(arg > 1.0) arg = 1.0;

  return 2.0*std::acos(arg)*TMath::RadToDeg();
}






Int_t ReadMaxTaggerChannels(const Config& config)
{
  Int_t maximum = 0;
  for(const std::string& fileName : config.inputFiles)
    {
      TFile inputFile(fileName.c_str(), "READ");
      TTree* setup = dynamic_cast<TTree*>(inputFile.Get("setupParameters"));
      if(inputFile.IsZombie() || !setup || !setup->GetBranch("nTagger") ||
         setup->GetEntries() < 1)
        throw std::runtime_error(fileName +
                                 ": cannot read setupParameters/nTagger");

      Int_t channels = 0;
      setup->SetBranchAddress("nTagger", &channels);
      setup->GetEntry(0);
      if(channels <= 0 || channels > kMaxTaggerChannels)
        throw std::runtime_error(fileName + ": invalid nTagger");
      maximum = std::max(maximum, channels);
    }
  return maximum;
}

struct Histograms
{
  TH1D missingEnergy;
  TH1D openingAngle;
  TH1D pi0Mass;
  TH1D missingMass;
  TH1D pi0ThetaCM;
  TH2D missingEnergyByTaggerChannel;
  TH2D openingAngleByTaggerChannel;
  TH2D pi0MassByTaggerChannel;
  TH2D missingMassByTaggerChannel;
  TH2D pi0ThetaCMByTaggerChannel;

  Histograms(const Config& config, Int_t taggerChannels)
    : missingEnergy("missingEnergy", "Missing energy, prompt-random;E_{miss} [MeV];Counts",
                    config.missingEnergyBins, config.missingEnergyMin,
                    config.missingEnergyMax),
      openingAngle("openingAngle", "Photon opening-angle excess, prompt-random;Opening angle - minimum opening angle [deg];Counts",
                   config.openingAngleBins, config.openingAngleMin,
                   config.openingAngleMax),
      pi0Mass("pi0Mass", "Two-photon invariant mass, prompt-random;m_{#gamma#gamma} [MeV];Counts",
              config.pi0MassBins, config.pi0MassHistMin, config.pi0MassHistMax),
      missingMass("missingMass", "Missing mass, prompt-random;M_{miss}-M_{target} [MeV];Counts",
                  config.missingMassBins, config.missingMassMin, config.missingMassMax),
      pi0ThetaCM("pi0ThetaCM", "Pi0 CM angle, prompt-random;#theta_{#pi^{0}}^{CM} [deg];Counts",
                 config.pi0ThetaCMBins, config.pi0ThetaCMMin, config.pi0ThetaCMMax),
      missingEnergyByTaggerChannel("missingEnergyByTaggerChannel", "Missing energy by tagger channel, prompt-random;Tagger channel;E_{miss} [MeV]",
                 taggerChannels, -0.5, taggerChannels - 0.5,
                 std::max(1, config.missingEnergyBins / 10),
                 config.missingEnergyMin, config.missingEnergyMax),
      openingAngleByTaggerChannel("openingAngleByTaggerChannel", "Photon opening-angle excess by tagger channel, prompt-random;Tagger channel;Opening angle - minimum opening angle [deg]",
                 taggerChannels, -0.5, taggerChannels - 0.5,
                 std::max(1, config.openingAngleBins / 10),
                 config.openingAngleMin, config.openingAngleMax),
      pi0MassByTaggerChannel("pi0MassByTaggerChannel", "Two-photon invariant mass by tagger channel, prompt-random;Tagger channel;m_{#gamma#gamma} [MeV]",
                 taggerChannels, -0.5, taggerChannels - 0.5,
                 std::max(1, config.pi0MassBins / 10),
                 config.pi0MassHistMin, config.pi0MassHistMax),
      missingMassByTaggerChannel("missingMassByTaggerChannel", "Missing mass by tagger channel, prompt-random;Tagger channel;M_{miss}-M_{target} [MeV]",
                 taggerChannels, -0.5, taggerChannels - 0.5,
                 std::max(1, config.missingMassBins / 10),
                 config.missingMassMin, config.missingMassMax),
      pi0ThetaCMByTaggerChannel("pi0ThetaCMByTaggerChannel", "Pi0 CM angle by tagger channel, prompt-random;Tagger channel;#theta_{#pi^{0}}^{CM} [deg]",
                 taggerChannels, -0.5, taggerChannels - 0.5,
                 std::max(1, config.pi0ThetaCMBins / 10),
                 config.pi0ThetaCMMin, config.pi0ThetaCMMax)
  {
    missingEnergy.Sumw2();
    openingAngle.Sumw2();
    pi0Mass.Sumw2();
    missingMass.Sumw2();
    pi0ThetaCM.Sumw2();
    missingEnergyByTaggerChannel.Sumw2();
    openingAngleByTaggerChannel.Sumw2();
    pi0MassByTaggerChannel.Sumw2();
    missingMassByTaggerChannel.Sumw2();
    pi0ThetaCMByTaggerChannel.Sumw2();
  }

  void Fill(Double_t missingEnergy, Double_t openingAngle, Double_t pi0Mass,
            Double_t missingMass, Double_t pi0ThetaCM,
            Int_t taggedChannel, Int_t timingClass, Double_t timingWeight)
  {
    if(timingClass != 0 && timingClass != 1)
      return;

    this->missingEnergy.Fill(missingEnergy, timingWeight);
    this->openingAngle.Fill(openingAngle, timingWeight);
    this->pi0Mass.Fill(pi0Mass, timingWeight);
    this->missingMass.Fill(missingMass, timingWeight);
    this->pi0ThetaCM.Fill(pi0ThetaCM, timingWeight);
    missingEnergyByTaggerChannel.Fill(taggedChannel, missingEnergy, timingWeight);
    openingAngleByTaggerChannel.Fill(taggedChannel, openingAngle, timingWeight);
    pi0MassByTaggerChannel.Fill(taggedChannel, pi0Mass, timingWeight);
    missingMassByTaggerChannel.Fill(taggedChannel, missingMass, timingWeight);
    pi0ThetaCMByTaggerChannel.Fill(taggedChannel, pi0ThetaCM, timingWeight);
  }

  void Write()
  {
    missingEnergy.Write();
    openingAngle.Write();
    pi0Mass.Write();
    missingMass.Write();
    pi0ThetaCM.Write();
    missingEnergyByTaggerChannel.Write();
    openingAngleByTaggerChannel.Write();
    pi0MassByTaggerChannel.Write();
    missingMassByTaggerChannel.Write();
    pi0ThetaCMByTaggerChannel.Write();
  }
};

struct Output
{
  Int_t inputFileIndex = -1;
  Long64_t sourceEntry = -1;
  Int_t nNeutral = 0;
  Int_t nGammaPairs = 0;
  Int_t nPi0 = 0;
  Int_t nTagged = 0;
  Int_t nPi0Tagger = 0;

  std::vector<Int_t> neutralTrack;
  std::vector<Double_t> neutralEnergy;
  std::vector<Double_t> neutralTheta;
  std::vector<Double_t> neutralPhi;
  std::vector<Double_t> neutralTime;

  std::vector<Int_t> pairTrack1;
  std::vector<Int_t> pairTrack2;
  std::vector<Double_t> pairInvariantMass;

  std::vector<Int_t> pi0Track1;
  std::vector<Int_t> pi0Track2;
  std::vector<Double_t> pi0Mass;
  std::vector<Double_t> pi0TotalEnergy;
  std::vector<Double_t> pi0KineticEnergy;
  std::vector<Double_t> pi0Theta;
  std::vector<Double_t> pi0Phi;
  std::vector<Double_t> pi0Time;
  std::vector<Double_t> pi0OpeningAngle;
  std::vector<Double_t> pi0Px;
  std::vector<Double_t> pi0Py;
  std::vector<Double_t> pi0Pz;

  std::vector<Int_t> taggedChannel;
  std::vector<Double_t> taggedEnergy;
  std::vector<Double_t> taggedTime;

  std::vector<Int_t> combinationPi0;
  std::vector<Int_t> combinationTagger;
  std::vector<Double_t> coincidenceTime;
  std::vector<Int_t> timingClass;
  std::vector<Double_t> timingWeight;
  std::vector<Double_t> missingMass;
  std::vector<Double_t> missingEnergy;
  std::vector<Double_t> pi0ThetaCM;
  std::vector<Double_t> pi0LabEnergy;
  std::vector<Double_t> minimumOpeningAngle;

  void Clear()
  {
    nNeutral = nGammaPairs = nPi0 = nTagged = nPi0Tagger = 0;
    neutralTrack.clear(); neutralEnergy.clear(); neutralTheta.clear();
    neutralPhi.clear(); neutralTime.clear();
    pairTrack1.clear(); pairTrack2.clear(); pairInvariantMass.clear();
    pi0Track1.clear(); pi0Track2.clear(); pi0Mass.clear();
    pi0TotalEnergy.clear(); pi0KineticEnergy.clear(); pi0Theta.clear();
    pi0Phi.clear(); pi0Time.clear(); pi0OpeningAngle.clear();
    pi0Px.clear(); pi0Py.clear(); pi0Pz.clear();
    taggedChannel.clear(); taggedEnergy.clear(); taggedTime.clear();
    combinationPi0.clear(); combinationTagger.clear(); coincidenceTime.clear();
    timingClass.clear(); timingWeight.clear(); missingMass.clear();
    missingEnergy.clear(); pi0ThetaCM.clear(); pi0LabEnergy.clear();
    minimumOpeningAngle.clear();
  }
};

void MakeBranches(TTree& tree, Output& out)
{
  tree.Branch("inputFileIndex", &out.inputFileIndex, "inputFileIndex/I");
  tree.Branch("sourceEntry", &out.sourceEntry, "sourceEntry/L");
  tree.Branch("nNeutral", &out.nNeutral, "nNeutral/I");
  tree.Branch("neutralTrack", &out.neutralTrack);
  tree.Branch("neutralEnergy", &out.neutralEnergy);
  tree.Branch("neutralTheta", &out.neutralTheta);
  tree.Branch("neutralPhi", &out.neutralPhi);
  tree.Branch("neutralTime", &out.neutralTime);
  tree.Branch("nGammaPairs", &out.nGammaPairs, "nGammaPairs/I");
  tree.Branch("pairTrack1", &out.pairTrack1);
  tree.Branch("pairTrack2", &out.pairTrack2);
  tree.Branch("pairInvariantMass", &out.pairInvariantMass);
  tree.Branch("nPi0", &out.nPi0, "nPi0/I");
  tree.Branch("pi0Track1", &out.pi0Track1);
  tree.Branch("pi0Track2", &out.pi0Track2);
  tree.Branch("pi0Mass", &out.pi0Mass);
  tree.Branch("pi0TotalEnergy", &out.pi0TotalEnergy);
  tree.Branch("pi0KineticEnergy", &out.pi0KineticEnergy);
  tree.Branch("pi0Theta", &out.pi0Theta);
  tree.Branch("pi0Phi", &out.pi0Phi);
  tree.Branch("pi0Time", &out.pi0Time);
  tree.Branch("pi0OpeningAngle", &out.pi0OpeningAngle);
  tree.Branch("pi0Px", &out.pi0Px);
  tree.Branch("pi0Py", &out.pi0Py);
  tree.Branch("pi0Pz", &out.pi0Pz);
  tree.Branch("nTagged", &out.nTagged, "nTagged/I");
  tree.Branch("taggedChannel", &out.taggedChannel);
  tree.Branch("taggedEnergy", &out.taggedEnergy);
  tree.Branch("taggedTime", &out.taggedTime);
  tree.Branch("nPi0Tagger", &out.nPi0Tagger, "nPi0Tagger/I");
  tree.Branch("combinationPi0", &out.combinationPi0);
  tree.Branch("combinationTagger", &out.combinationTagger);
  tree.Branch("coincidenceTime", &out.coincidenceTime);
  tree.Branch("timingClass", &out.timingClass);
  tree.Branch("timingWeight", &out.timingWeight);
  tree.Branch("missingMass", &out.missingMass);
  tree.Branch("missingEnergy", &out.missingEnergy);
  tree.Branch("pi0ThetaCM", &out.pi0ThetaCM);
  tree.Branch("pi0LabEnergy", &out.pi0LabEnergy);
  tree.Branch("minimumOpeningAngle", &out.minimumOpeningAngle);
}



//*************************************************************************************************************
int ProcessFile(const Config& config, Int_t fileIndex, TTree& outputTree,
                Output& out, Histograms& histograms,
                Long64_t& totalEvents, Long64_t& totalPi0)
{
  const std::string& fileName = config.inputFiles.at(fileIndex);
  TFile inputFile(fileName.c_str(), "READ");
  if(inputFile.IsZombie())
    {
      std::cerr << "Cannot open input file: " << fileName << std::endl;
      return 1;
    }

  TTree* tracks = dynamic_cast<TTree*>(inputFile.Get("tracks"));
  TTree* tagger = dynamic_cast<TTree*>(inputFile.Get("tagger"));
  TTree* setup = dynamic_cast<TTree*>(inputFile.Get("setupParameters"));
  if(!tracks || !tagger || !setup)
    {
      std::cerr << fileName
                << ": required trees tracks, tagger and/or setupParameters are missing"
                << std::endl;
      return 1;
    }
  if(tracks->GetEntries() != tagger->GetEntries())
    {
      std::cerr << fileName << ": tracks and tagger have different entry counts"
                << std::endl;
      return 1;
    }

  Int_t nTaggerChannels = 0;
  Double_t setupTaggerEnergy[kMaxTaggerChannels] = {0.0};
  if(!setup->GetBranch("nTagger") || !setup->GetBranch("TaggerPhotonEnergy") ||
     setup->GetEntries() < 1)
    {
      std::cerr << fileName
                << ": setupParameters lacks nTagger or TaggerPhotonEnergy"
                << std::endl;
      return 1;
    }
  setup->SetBranchAddress("nTagger", &nTaggerChannels);
  setup->SetBranchAddress("TaggerPhotonEnergy", setupTaggerEnergy);
  setup->GetEntry(0);
  if(nTaggerChannels <= 0 || nTaggerChannels > kMaxTaggerChannels)
    {
      std::cerr << fileName << ": invalid nTagger " << nTaggerChannels
                << std::endl;
      return 1;
    }

  TTreeReader trackReader(tracks);
  TTreeReaderValue<Int_t> nTracks(trackReader, "nTracks");
  TTreeReaderArray<Double_t> clusterEnergy(trackReader, "clusterEnergy");
  TTreeReaderArray<Double_t> theta(trackReader, "theta");
  TTreeReaderArray<Double_t> phi(trackReader, "phi");
  TTreeReaderArray<Double_t> trackTime(trackReader, "time");
  TTreeReaderArray<Int_t> detectors(trackReader, "detectors");

  TTreeReader taggerReader(tagger);
  TTreeReaderValue<Int_t> inputNTagged(taggerReader, "nTagged");
  TTreeReaderArray<Int_t> inputTaggedChannel(taggerReader, "taggedChannel");
  TTreeReaderArray<Double_t> inputTaggedTime(taggerReader, "taggedTime");
  const bool hasTaggedEnergy = tagger->GetBranch("taggedEnergy") != nullptr;
  TTreeReaderArray<Double_t>* inputTaggedEnergy = hasTaggedEnergy
    ? new TTreeReaderArray<Double_t>(taggerReader, "taggedEnergy") : nullptr;

  const Double_t randomWeight =
    -(config.promptMax - config.promptMin) /
    ((config.random1Max - config.random1Min) +
     (config.random2Max - config.random2Min));
  const TLorentzVector target(0.0, 0.0, 0.0, config.targetMass);
  const Long64_t entries = tracks->GetEntries();
  const auto start = std::chrono::steady_clock::now();

  for(Long64_t entry = 0; entry < entries; ++entry)
    {
      if(trackReader.SetEntry(entry) != TTreeReader::kEntryValid ||
         taggerReader.SetEntry(entry) != TTreeReader::kEntryValid)
        {
          std::cerr << fileName << ": cannot read entry " << entry << std::endl;
          delete inputTaggedEnergy;
          return 1;
        }

      out.Clear();
      out.inputFileIndex = fileIndex;
      out.sourceEntry = entry;
      std::vector<TLorentzVector> photons;

      if(*nTracks < 0 || static_cast<std::size_t>(*nTracks) > detectors.GetSize())
        {
          std::cerr << fileName << ": invalid nTracks in entry " << entry
                    << std::endl;
          delete inputTaggedEnergy;
          return 1;
        }
      for(Int_t track = 0; track < *nTracks; ++track)
        {
          const bool isCB = (detectors[track] & kEDetNaI) != 0;
          const bool hasChargedVeto =
            (detectors[track] & (kEDetPID | kEDetMWPC)) != 0;
          if(!isCB || hasChargedVeto || clusterEnergy[track] <= 0.0)
            continue;
          if(!std::isfinite(clusterEnergy[track]) || !std::isfinite(theta[track]) ||
             !std::isfinite(phi[track]) || !std::isfinite(trackTime[track]))
            continue;

          out.neutralTrack.push_back(track);
          out.neutralEnergy.push_back(clusterEnergy[track]);
          out.neutralTheta.push_back(theta[track]);
          out.neutralPhi.push_back(phi[track]);
          out.neutralTime.push_back(trackTime[track]);
          photons.push_back(PhotonP4(clusterEnergy[track], theta[track], phi[track]));
        }
      out.nNeutral = static_cast<Int_t>(photons.size());
      if(out.nNeutral != 2) continue;

      std::vector<TLorentzVector> pi0FourVectors;
      for(std::size_t i = 0; i < photons.size(); ++i)
        for(std::size_t j = i + 1; j < photons.size(); ++j)
          {
            const TLorentzVector candidate = photons[i] + photons[j];
            const Double_t mass = candidate.M();
            out.pairTrack1.push_back(out.neutralTrack[i]);
            out.pairTrack2.push_back(out.neutralTrack[j]);
            out.pairInvariantMass.push_back(mass);
            if(mass <= config.pi0MassMin || mass >= config.pi0MassMax)
              continue;

            out.pi0Track1.push_back(out.neutralTrack[i]);
            out.pi0Track2.push_back(out.neutralTrack[j]);
            out.pi0Mass.push_back(mass);
            out.pi0TotalEnergy.push_back(candidate.E());
            out.pi0KineticEnergy.push_back(candidate.E() - mass);
            out.pi0Theta.push_back(candidate.Theta() * TMath::RadToDeg());
            out.pi0Phi.push_back(candidate.Phi() * TMath::RadToDeg());
            out.pi0Time.push_back(0.5 * (out.neutralTime[i] + out.neutralTime[j]));
            out.pi0OpeningAngle.push_back(
              photons[i].Vect().Angle(photons[j].Vect()) * TMath::RadToDeg());
            out.pi0Px.push_back(candidate.Px());
            out.pi0Py.push_back(candidate.Py());
            out.pi0Pz.push_back(candidate.Pz());
            pi0FourVectors.push_back(candidate);
          }
      out.nGammaPairs = static_cast<Int_t>(out.pairInvariantMass.size());
      out.nPi0 = static_cast<Int_t>(pi0FourVectors.size());
      totalPi0 += out.nPi0;

      if(*inputNTagged < 0 ||
         static_cast<std::size_t>(*inputNTagged) > inputTaggedChannel.GetSize())
        {
          std::cerr << fileName << ": invalid nTagged in entry " << entry
                    << std::endl;
          delete inputTaggedEnergy;
          return 1;
        }
      for(Int_t hit = 0; hit < *inputNTagged; ++hit)
        {
          const Int_t channel = inputTaggedChannel[hit];
          if(channel < config.taggerChannelCutMin ||
             channel > config.taggerChannelCutMax ||
             channel >= nTaggerChannels)
            continue;
          const Double_t energy = inputTaggedEnergy
            ? (*inputTaggedEnergy)[hit] : setupTaggerEnergy[channel];
          if(!std::isfinite(energy) || energy <= 0.0 ||
             !std::isfinite(inputTaggedTime[hit]))
            continue;
          out.taggedChannel.push_back(channel);
          out.taggedEnergy.push_back(energy);
          out.taggedTime.push_back(inputTaggedTime[hit]);
        }
      out.nTagged = static_cast<Int_t>(out.taggedChannel.size());

      for(std::size_t pi0 = 0; pi0 < pi0FourVectors.size(); ++pi0)
        for(std::size_t tag = 0; tag < out.taggedChannel.size(); ++tag)
          {
            if(!InWindow(out.pi0Mass[pi0], config.pi0MassCutMin,
                         config.pi0MassCutMax))
              continue;

            const Double_t coincidence = out.taggedTime[tag] - out.pi0Time[pi0];
            Int_t classification = -1;
            Double_t weight = 0.0;
            if(InWindow(coincidence, config.promptMin, config.promptMax))
              {
                classification = 1;
                weight = 1.0;
              }
            else if(InWindow(coincidence, config.random1Min, config.random1Max) ||
                    InWindow(coincidence, config.random2Min, config.random2Max))
              {
                classification = 0;
                weight = randomWeight;
              }
//            else
//              continue;

            const Double_t beamEnergy = out.taggedEnergy[tag];
            const TLorentzVector beam(0.0, 0.0, beamEnergy, beamEnergy);
            const TLorentzVector missing = target + beam - pi0FourVectors[pi0];
            Double_t missingEnergy = 0.0;
            Double_t pi0ThetaCM = 0.0;
            if(!CalculateMissingEnergy(pi0FourVectors[pi0], beamEnergy,
                                       config.targetMass, config.pi0Mass,
                                       missingEnergy, pi0ThetaCM))
              continue;
            if(!InWindow(missingEnergy, config.missingEnergyCutMin,
                         config.missingEnergyCutMax))
              continue;
            const Double_t missingMass = missing.M() - config.targetMass;
            const Double_t pi0LabEnergy =
              CoherentPionLabEnergy(beamEnergy, pi0ThetaCM, config.pi0Mass,
                                    config.targetMass);
            const Double_t minimumOpeningAngle =
              PhiMinDeg(pi0LabEnergy, config.pi0Mass);
            const Double_t openingAngle =
              out.pi0OpeningAngle[pi0] - minimumOpeningAngle;
            if(!InWindow(openingAngle, config.openingAngleCutMin,
                         config.openingAngleCutMax))
              continue;
            out.combinationPi0.push_back(static_cast<Int_t>(pi0));
            out.combinationTagger.push_back(static_cast<Int_t>(tag));
            out.coincidenceTime.push_back(coincidence);
            out.timingClass.push_back(classification);
            out.timingWeight.push_back(weight);
            out.missingMass.push_back(missingMass);
            out.missingEnergy.push_back(missingEnergy);
            out.pi0ThetaCM.push_back(pi0ThetaCM);
            out.pi0LabEnergy.push_back(pi0LabEnergy);
            out.minimumOpeningAngle.push_back(minimumOpeningAngle);
            histograms.Fill(missingEnergy, openingAngle,
                            out.pi0Mass[pi0], missingMass, pi0ThetaCM,
                            out.taggedChannel[tag],
                            classification, weight);
          }
      out.nPi0Tagger = static_cast<Int_t>(out.combinationPi0.size());
      outputTree.Fill();
      ++totalEvents;

      if(config.progressInterval > 0 && (entry + 1) % config.progressInterval == 0)
        {
          const Double_t seconds = std::chrono::duration<Double_t>(
            std::chrono::steady_clock::now() - start).count();
          std::cout << "  " << (entry + 1) << '/' << entries << " events, "
                    << std::fixed << std::setprecision(1) << seconds << " s"
                    << std::endl;
        }
    }

  delete inputTaggedEnergy;
  std::cout << "  processed " << entries << " events" << std::endl;
  return 0;
}
}

int main(int argc, char** argv)
{
  if(argc != 2)
    {
      std::cerr << "Usage: " << argv[0] << " CONFIG_FILE" << std::endl;
      return 1;
    }

  try
    {
      const Config config = ReadConfig(argv[1]);
      const Int_t taggerChannels = ReadMaxTaggerChannels(config);
      TFile outputFile(config.outputFile.c_str(), "RECREATE");
      if(outputFile.IsZombie())
        throw std::runtime_error("cannot create output file " + config.outputFile);

      TTree outputTree("pi0", "Neutral pi0 candidates reconstructed from GoAT trees");
      Output output;
      MakeBranches(outputTree, output);
      Histograms histograms(config, taggerChannels);

      Long64_t totalEvents = 0;
      Long64_t totalPi0 = 0;
      for(std::size_t file = 0; file < config.inputFiles.size(); ++file)
        {
          std::cout << "Reading " << config.inputFiles[file] << std::endl;
          if(ProcessFile(config, static_cast<Int_t>(file), outputTree, output,
                         histograms,
                         totalEvents, totalPi0) != 0)
            return 1;
        }

      outputFile.cd();
      outputTree.Write();
      histograms.Write();
      outputFile.Close();
      std::cout << "Wrote " << totalEvents << " events and " << totalPi0
                << " pi0 candidates to " << config.outputFile << std::endl;
      return 0;
    }
  catch(const std::exception& error)
    {
      std::cerr << "Error: " << error.what() << std::endl;
      return 1;
    }
}

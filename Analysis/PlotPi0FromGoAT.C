#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TLegend.h"
#include "TStyle.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
std::string FileLabel(const char* path)
{
  const std::string name(path ? path : "");
  const std::string::size_type slash = name.find_last_of("/\\");
  return slash == std::string::npos ? name : name.substr(slash + 1);
}

TH1* ReadHistogram(TFile& file, const char* name, const char* suffix)
{
  TH1* source = dynamic_cast<TH1*>(file.Get(name));
  if(!source)
    {
      std::cerr << "Histogram '" << name << "' is missing in "
                << file.GetName() << std::endl;
      return nullptr;
    }

  TH1* histogram = dynamic_cast<TH1*>(source->Clone(
    (std::string(name) + suffix).c_str()));
  if(histogram)
    histogram->SetDirectory(nullptr);
  return histogram;
}
}

// Usage:
// root -l 'PlotPi0FromGoAT.C("first.root","second.root","comparison.pdf")'
void PlotPi0FromGoAT(const char* firstFileName,
                     const char* secondFileName,
                     const char* outputPdf = "Pi0FromGoAT_comparison.pdf")
{
  TFile firstFile(firstFileName, "READ");
  TFile secondFile(secondFileName, "READ");
  if(firstFile.IsZombie() || secondFile.IsZombie())
    {
      std::cerr << "Could not open both input ROOT files." << std::endl;
      return;
    }

  const std::vector<std::string> names = {
    "missingEnergy", "openingAngle", "pi0Mass", "missingMass", "pi0ThetaCM"
  };
  const std::string firstLabel = FileLabel(firstFileName);
  const std::string secondLabel = FileLabel(secondFileName);
  const bool writePdf = outputPdf && outputPdf[0] != '\0';
  const std::string pdf = writePdf ? outputPdf : "";

  gStyle->SetOptStat(0);
  std::vector<std::unique_ptr<TH1> > firstHistograms;
  std::vector<std::unique_ptr<TH1> > secondHistograms;
  std::vector<std::unique_ptr<TCanvas> > canvases;

  if(writePdf)
    {
      TCanvas pdfMarker("pdfMarker", "pdfMarker", 1, 1);
      pdfMarker.Print((pdf + "[").c_str());
    }

  for(std::size_t i = 0; i < names.size(); ++i)
    {
      firstHistograms.emplace_back(
        ReadHistogram(firstFile, names[i].c_str(), "_first"));
      secondHistograms.emplace_back(
        ReadHistogram(secondFile, names[i].c_str(), "_second"));
      TH1* first = firstHistograms.back().get();
      TH1* second = secondHistograms.back().get();
      if(!first || !second)
        continue;

      if(first->GetNbinsX() != second->GetNbinsX() ||
         first->GetXaxis()->GetXmin() != second->GetXaxis()->GetXmin() ||
         first->GetXaxis()->GetXmax() != second->GetXaxis()->GetXmax())
        std::cerr << "Warning: different binning for '" << names[i]
                  << "'." << std::endl;

      first->SetLineColor(kBlue + 1);
      first->SetMarkerColor(kBlue + 1);
      first->SetLineWidth(2);
      second->SetLineColor(kRed + 1);
      second->SetMarkerColor(kRed + 1);
      second->SetLineWidth(2);

      const double maximum = std::max(first->GetMaximum(), second->GetMaximum());
      const double minimum = std::min(first->GetMinimum(), second->GetMinimum());
      first->SetMaximum(maximum > 0.0 ? 1.12 * maximum : 0.0);
      first->SetMinimum(minimum < 0.0 ? 1.12 * minimum : 0.0);
      first->SetTitle((names[i] + " comparison").c_str());

      canvases.emplace_back(new TCanvas(
        ("canvas_" + names[i]).c_str(), names[i].c_str(), 1000, 700));
      first->Draw("HIST");
      second->Draw("HIST SAME");

      TLegend legend(0.58, 0.76, 0.89, 0.89);
      legend.SetBorderSize(0);
      legend.SetFillStyle(0);
      legend.AddEntry(first, firstLabel.c_str(), "l");
      legend.AddEntry(second, secondLabel.c_str(), "l");
      legend.Draw();
      canvases.back()->Modified();
      canvases.back()->Update();
      if(writePdf)
        canvases.back()->Print(pdf.c_str());
    }

  if(writePdf)
    {
      TCanvas pdfMarker("pdfMarkerClose", "pdfMarkerClose", 1, 1);
      pdfMarker.Print((pdf + "]").c_str());
      std::cout << "Wrote " << pdf << std::endl;
    }
}

#include "TCWriteARCalib.h"
#include "TString.h"
#include "TSystem.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <iostream>

namespace {
bool ParseRun(const char* text, Int_t& run)
{
    char* end = 0;
    errno = 0;
    const long value = std::strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' || value < 0 || value > INT_MAX)
        return false;
    run = static_cast<Int_t>(value);
    return true;
}

bool WriteDetector(CalibDetector_t detector, const TString& input,
                   const TString& output, const char* calibration, Int_t run)
{
    TCWriteARCalib writer(detector, input.Data());
    return writer.Write(output.Data(), calibration, run);
}
}

int main(int argc, char** argv)
{
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " CALIBRATION RUN DATA_DIRECTORY OUTPUT_DIRECTORY\n";
        return 2;
    }

    Int_t run = 0;
    if (!ParseRun(argv[2], run)) {
        std::cerr << "Invalid run number: " << argv[2] << '\n';
        return 2;
    }

    TString dataDir(argv[3]);
    TString outputDir(argv[4]);
    gSystem->ExpandPathName(dataDir);
    gSystem->ExpandPathName(outputDir);
    if (gSystem->AccessPathName(dataDir.Data(), kReadPermission)) {
        std::cerr << "Input directory does not exist: " << dataDir << '\n';
        return 1;
    }
    if (gSystem->mkdir(outputDir.Data(), kTRUE) != 0 &&
        gSystem->AccessPathName(outputDir.Data(), kWritePermission)) {
        std::cerr << "Cannot create output directory: " << outputDir << '\n';
        return 1;
    }

    std::cout << "Exporting calibration '" << argv[1] << "' for run " << run
              << " to " << outputDir << '\n';
    bool ok = true;
    ok = WriteDetector(kDETECTOR_TAGG, dataDir + "/FPD_855_new.dat",
                       outputDir + "/FPD_855_new_CaLib.dat", argv[1], run) && ok;
    ok = WriteDetector(kDETECTOR_CB, dataDir + "/Detector-NaI.dat",
                       outputDir + "/Detector-NaI_CaLib.dat", argv[1], run) && ok;
    ok = WriteDetector(kDETECTOR_PID, dataDir + "/Detector-PID.dat",
                       outputDir + "/Detector-PID_CaLib.dat", argv[1], run) && ok;

    if (!ok) {
        std::cerr << "Export failed; see the preceding CaLib errors.\n";
        return 1;
    }
    std::cout << "Export completed successfully.\n";
    return 0;
}

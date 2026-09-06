// Export CaLib parameters without parsing the legacy CaLib dictionary in Cling.
#include "TError.h"
#include "TString.h"
#include "TSystem.h"


//______________________________________________________________________________
void WriteARCalibration(const Char_t* calibration = "2025-Compton-4He",
                        Int_t run = 31840,
                        const Char_t* dataDir = "$CALIB/../acqu_user/data.2025",
                        const Char_t* outputDir = "$CALIB/export/2025-Compton-4He")
{
    TString program = "$CALIB/../build/bin/write_ar_calibration";
    gSystem->ExpandPathName(program);
    if (gSystem->AccessPathName(program.Data(), kExecutePermission))
    {
        Error("WriteARCalibration",
              "Exporter '%s' does not exist or is not executable. Rebuild AcquRoot first.",
              program.Data());
        return;
    }

    TString cal(calibration);
    TString input(dataDir);
    TString output(outputDir);
    cal.ReplaceAll("'", "'\\''");
    input.ReplaceAll("'", "'\\''");
    output.ReplaceAll("'", "'\\''");

    TString command = TString::Format("'%s' '%s' %d '%s' '%s'",
                                      program.Data(), cal.Data(), run,
                                      input.Data(), output.Data());
    const Int_t status = gSystem->Exec(command.Data());
    if (status != 0)
        Error("WriteARCalibration", "Export failed (exit status %d)", status);
}

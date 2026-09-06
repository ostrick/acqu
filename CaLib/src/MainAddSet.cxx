#include "TCMySQLManager.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <iostream>

namespace {
bool ParseInteger(const char* text, Int_t& value)
{
    char* end = 0;
    errno = 0;
    const long parsed = std::strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' || parsed < 0 || parsed > INT_MAX)
        return false;
    value = static_cast<Int_t>(parsed);
    return true;
}
}

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0]
                  << " SOURCE_SET FIRST_RUN LAST_RUN RAWFILE_PATH\n";
        return 2;
    }

    Int_t sourceSet, firstRun, lastRun;
    if (!ParseInteger(argv[1], sourceSet) || !ParseInteger(argv[2], firstRun) ||
        !ParseInteger(argv[3], lastRun) || firstRun > lastRun)
    {
        std::cerr << "Invalid source set or run range\n";
        return 2;
    }

    const Char_t calibration[] = "2025-Compton-4He";
    const Char_t description[] = "Calibration of 2025 4He Beamtime";
    const Char_t target[] = "LHe4";
    const Char_t* types[] = {
        "Type.Tagger.Time", "Type.CB.Time",
        "Type.CB.Energy", "Type.PID.Time"
    };

    TCMySQLManager* manager = TCMySQLManager::GetManager();
    if (!manager) return 1;

    manager->AddRunFiles(argv[4], target, firstRun, lastRun);
    if (!manager->ContainsRun(firstRun) || !manager->ContainsRun(lastRun))
    {
        std::cerr << "Boundary runs " << firstRun << " and " << lastRun
                  << " must exist in run_main; no runsets were copied.\n";
        return 1;
    }

    bool ok = true;
    for (UInt_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
        if (!manager->CloneSet(types[i], calibration, description,
                               sourceSet, firstRun, lastRun))
            ok = false;

    if (!ok)
    {
        std::cerr << "At least one runset could not be copied.\n";
        return 1;
    }

    std::cout << "All requested runsets were copied successfully.\n";
    return 0;
}

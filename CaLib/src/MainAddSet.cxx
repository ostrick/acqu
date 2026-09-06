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
    if (argc < 9)
    {
        std::cerr << "Usage: " << argv[0]
                  << " SOURCE_SET FIRST_RUN LAST_RUN RAWFILE_PATH CALIBRATION"
                     " DESCRIPTION TARGET TYPE [TYPE ...]\n";
        return 2;
    }

    Int_t sourceSet, firstRun, lastRun;
    if (!ParseInteger(argv[1], sourceSet) || !ParseInteger(argv[2], firstRun) ||
        !ParseInteger(argv[3], lastRun) || firstRun > lastRun)
    {
        std::cerr << "Invalid source set or run range\n";
        return 2;
    }

    TCMySQLManager* manager = TCMySQLManager::GetManager();
    if (!manager) return 1;

    manager->AddRunFiles(argv[4], argv[7], firstRun, lastRun);
    if (!manager->ContainsRun(firstRun) || !manager->ContainsRun(lastRun))
    {
        std::cerr << "Boundary runs " << firstRun << " and " << lastRun
                  << " must exist in run_main; no runsets were copied.\n";
        return 1;
    }

    bool ok = true;
    for (Int_t i = 8; i < argc; ++i)
        if (!manager->CloneSet(argv[i], argv[5], argv[6],
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

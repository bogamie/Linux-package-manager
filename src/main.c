#include "ui.h"
#include "utils.h"
#include "package_manager.h"
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-help") == 0) {
        printHelp();
        return 0;
    }
    int packageCount, currIndex = 0, startIndex = 0, prevCh = 0, exitFlag = 0;
    Package *packages = NULL;

    init();
    managePackage(NULL, 2);
    packageCount = loadPackages(&packages);

    while (!exitFlag) {
        if (currIndex < startIndex) {
            startIndex = currIndex;
        } else if (currIndex >= startIndex + ROWS - 3) {
            startIndex = currIndex - (ROWS - 3) + 1;
        }
        displayPackages(packages, packageCount, startIndex, currIndex);
        keyInput(&currIndex, &startIndex, &prevCh, &exitFlag, packageCount, packages);
    }

    deinit(packages, packageCount);
    return 0;
}


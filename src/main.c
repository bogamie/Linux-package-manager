#include "ui.h"
#include "utils.h"
#include "package_manager.h"
#include "constants.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-help") == 0) {
        printHelp();
        return EXIT_SUCCESS;
    }
    
    int packageCount, currIndex = 0, startIndex = 0, prevCh = 0, exitFlag = 0;
    Package *packages = NULL;

    init();
    
    // 시스템 업데이트 (선택 사항)
    managePackage(NULL, ACTION_UPDATE);
    
    packageCount = loadPackages(&packages);
    if (packageCount <= 0) {
        deinit(packages, packageCount);
        fprintf(stderr, "Failed to load packages\n");
        return EXIT_FAILURE;
    }

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
    return EXIT_SUCCESS;
}


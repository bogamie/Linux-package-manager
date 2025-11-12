#include "ui.h"
#include "utils.h"
#include "package_manager.h"
#include "constants.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    bool autoUpdate = false;

    // 명령행 인자 파싱
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
            printHelp();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "--auto-update") == 0 || strcmp(argv[i], "-U") == 0) {
            autoUpdate = true;
        }
    }
    
    int packageCount, currIndex = 0, startIndex = 0, prevCh = 0, exitFlag = 0;
    Package *packages = NULL;

    init();
    
    // 선택적 자동 시스템 업데이트 (사용자가 명령행 옵션으로 요청한 경우)
    if (autoUpdate) {
        managePackage(NULL, ACTION_UPDATE);
    }
    // 기본 동작: 시작 시 업데이트 수행 안 함. 'u' 키로 명시적 실행.
    
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include "ui.h"
#include "utils.h"
#include "package_manager.h"
#include "constants.h"

// 패키지 메모리 해제 함수
void freePackages(Package *p, int count) {
    if (!p) return;
    
    for (int i = 0; i < count; i++) {
        safeFree(&p[i].name);
        safeFree(&p[i].version);
        safeFree(&p[i].description);
    }
    free(p);
}

// dpkg 쿼리 실행 및 패키지 카운트
static int countPackages(void) {
    char buffer[BUFFER_SIZE];
    int packageCount = 0;

    FILE *fp = popen("dpkg-query -W -f='${Package}\t${Version}\t${binary:Summary}\n'", "r");
    if (!fp) {
        fprintf(stderr, "Failed to run dpkg-query command\n");
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        packageCount++;
    }
    pclose(fp);
    
    return packageCount;
}

// 패키지 정보 파싱
static bool parsePackageLine(char *buffer, Package *package) {
    char *name = strtok(buffer, "\t");
    char *version = strtok(NULL, "\t");
    char *description = strtok(NULL, "\n");

    if (!name) return false;

    package->name = safeStrdup(name ? name : "(Unknown)");
    package->version = safeStrdup(version ? version : "(Unknown)");
    package->description = safeStrdup(description ? description : "(No description)");
    
    return true;
}

int loadPackages(Package **p) {
    int packageCount = countPackages();
    if (packageCount <= 0) {
        fprintf(stderr, "No packages found or error occurred\n");
        return 0;
    }

    FILE *fp = popen("dpkg-query -W -f='${Package}\t${Version}\t${binary:Summary}\n'", "r");
    if (!fp) {
        fprintf(stderr, "Failed to re-run command\n");
        return 0;
    }

    *p = (Package *)calloc(packageCount, sizeof(Package));
    if (!*p) {
        fprintf(stderr, "Failed to allocate memory\n");
        pclose(fp);
        return 0;
    }

    char buffer[BUFFER_SIZE];
    int index = 0;
    while (fgets(buffer, sizeof(buffer), fp) && index < packageCount) {
        if (parsePackageLine(buffer, &(*p)[index])) {
            index++;
        }
    }
    pclose(fp);
    
    return index;
}

// 명령어 실행 결과 표시
static void displayCommandOutput(FILE *fp) {
    char buffer[BUFFER_SIZE];
    int row = 2;
    
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (row < ROWS - 1) {
            mvprintw(row++, 0, "%s", buffer);
        } else {
            mvprintw(ROWS - 1, 0, "Output exceeds screen height. Press any key to scroll.");
            refresh();
            getch();
            clear();
            row = 2;
        }
        refresh();
    }
}

// 작업 결과 메시지 표시
static void displayActionResult(int result, int action, const char *packageName, int row) {
    if (result == 0) {
        if (action == ACTION_UPDATE) {
            mvprintw(row, 0, "System updated and upgraded successfully.");
        } else {
            mvprintw(row, 0, "Package '%s' was %s successfully.", 
                     packageName, action == ACTION_INSTALL ? "installed" : "removed");
        }
    } else {
        if (action == ACTION_UPDATE) {
            mvprintw(row, 0, "Error: Failed to update and upgrade the system.");
        } else {
            mvprintw(row, 0, "Error: Failed to %s package '%s'.", 
                     action == ACTION_INSTALL ? "install" : "remove", packageName);
        }
    }
}

void managePackage(Package *package, int action) {
    char command[COMMAND_SIZE];
    clear();

    // 패키지 이름 검증
    if (action != ACTION_UPDATE && (!package || !isValidPackageName(package->name))) {
        mvprintw(0, 0, "Error: Invalid package name");
        mvprintw(ROWS - 1, 0, "Press any key to return.");
        refresh();
        getch();
        clear();
        return;
    }

    switch (action) {
        case ACTION_INSTALL:
            snprintf(command, sizeof(command), "sudo apt-get install -y %s 2>&1", package->name);
            mvprintw(0, 0, "Installing package: %s", package->name);
            break;
        case ACTION_REMOVE:
            snprintf(command, sizeof(command), "sudo apt-get remove -y %s 2>&1", package->name);
            mvprintw(0, 0, "Removing package: %s", package->name);
            break;
        case ACTION_UPDATE:
            snprintf(command, sizeof(command), "sudo apt-get update && sudo apt-get upgrade -y");
            mvprintw(0, 0, "Updating and upgrading the system...");
            break;
        default:
            mvprintw(0, 0, "Invalid action.");
            refresh();
            sleep(2);
            return;
    }

    refresh();

    FILE *fp = popen(command, "r");
    if (!fp) {
        mvprintw(2, 0, "Error: Failed to execute command.");
        mvprintw(ROWS - 1, 0, "Press any key to return.");
        refresh();
        getch();
        clear();
        return;
    }

    int currentRow = 2;
    displayCommandOutput(fp);
    int result = pclose(fp);

    // 마지막 출력 줄 찾기
    for (int i = ROWS - 2; i >= 2; i--) {
        move(i, 0);
        if (inch() != ' ' && inch() != '\0') {
            currentRow = i + 1;
            break;
        }
    }

    const char *pkgName = (action != ACTION_UPDATE && package) ? package->name : "";
    displayActionResult(result, action, pkgName, currentRow);

    refresh();
    if (action != ACTION_UPDATE) { 
        mvprintw(ROWS - 1, 0, "Press any key to return to the list.");
        getch();
        clear();
    } else {
        sleep(2);
        clear();
    }
}

// 패키지 상세 정보 가져오기
static bool fetchPackageDetails(const char *packageName, Package *package) {
    char command[COMMAND_SIZE];
    char buffer[BUFFER_SIZE];
    
    snprintf(command, sizeof(command), "apt show %s 2>/dev/null", packageName);
    FILE *fp = popen(command, "r");
    if (!fp) {
        return false;
    }

    char *name = NULL, *version = NULL, *description = NULL;
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strncmp(buffer, "Package:", 8) == 0) {
            name = safeStrdup(strchr(buffer, ':') + 2);
            name[strcspn(name, "\n")] = '\0';
        } else if (strncmp(buffer, "Version:", 8) == 0) {
            version = safeStrdup(strchr(buffer, ':') + 2);
            version[strcspn(version, "\n")] = '\0';
        } else if (strncmp(buffer, "Description:", 12) == 0) {
            description = safeStrdup(strchr(buffer, ':') + 2);
            description[strcspn(description, "\n")] = '\0';
        }
    }
    pclose(fp);

    if (name && version && description) {
        package->name = name;
        package->version = version;
        package->description = description;
        return true;
    }
    
    safeFree(&name);
    safeFree(&version);
    safeFree(&description);
    return false;
}

void searchPackage() {
    int count = 0;
    char query[PACKAGE_NAME_SIZE];
    char command[COMMAND_SIZE];
    char buffer[BUFFER_SIZE];
    char packages[MAX_PACKAGES][PACKAGE_NAME_SIZE]; 
    Package *p = NULL;
    int currIndex = 0, startIndex = 0, exitFlag = 0, prevCh = 0;

    mvprintw(ROWS - 1, 0, "/");
    clrtoeol();
    echo();

    getstr(query);
    noecho();
    
    // 입력 검증
    if (!isValidPackageName(query)) {
        clear();
        mvprintw(0, 0, "Error: Invalid search query. Only alphanumeric characters, '-', '.', '_', '+', ':' are allowed.");
        mvprintw(ROWS - 1, 0, "Press any key to return.");
        refresh();
        getch();
        clear();
        return;
    }
    
    clear();
    mvprintw(0, 0, "Search results for '%s':", query);
    mvhline(1, 0, '-', COLS);

    snprintf(command, sizeof(command), "apt search %s 2>/dev/null", query);

    FILE *fp = popen(command, "r");
    if (!fp) {
        mvprintw(2, 0, "Failed to run apt search command.");
        mvprintw(ROWS - 1, 0, "Press any key to return.");
        refresh();
        getch();
        return;
    }

    bool isPackageLine = true;
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, "Sorting") || strstr(buffer, "Full Text Search") || strlen(buffer) <= 1) {
            isPackageLine = true;
            continue;
        }

        if (isPackageLine) {
            char *name = strtok(buffer, " /");
            if (name && count < MAX_PACKAGES) {
                strncpy(packages[count], name, sizeof(packages[count]) - 1);
                packages[count][sizeof(packages[count]) - 1] = '\0';
                count++;
            }
            isPackageLine = false;
        } else {
            isPackageLine = false;
        }
    }
    pclose(fp);

    // 패키지 구조체 배열 메모리 할당
    p = (Package *)calloc(count, sizeof(Package));
    if (!p) {
        mvprintw(2, 0, "Memory allocation failed.");
        refresh();
        getch();
        return;
    }

    // 각 패키지 이름을 이용해 정보 추출
    int validCount = 0;
    for (int i = 0; i < count; i++) {
        // 패키지 이름 검증
        if (!isValidPackageName(packages[i])) {
            fprintf(stderr, "Skipping invalid package name: %s\n", packages[i]);
            continue;
        }
        
        if (fetchPackageDetails(packages[i], &p[validCount])) {
            validCount++;
        }
    }

    // 패키지 목록 출력 및 키 입력 처리
    while (!exitFlag) {
        if (currIndex < startIndex) {
            startIndex = currIndex;
        } else if (currIndex >= startIndex + ROWS - 3) {
            startIndex = currIndex - (ROWS - 3) + 1;
        }
        displayPackages(p, validCount, startIndex, currIndex);
        keyInput(&currIndex, &startIndex, &prevCh, &exitFlag, validCount, p);
    }

    // 메모리 해제
    freePackages(p, validCount);
}



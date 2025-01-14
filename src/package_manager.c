#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include "ui.h"
#include "utils.h"
#include "package_manager.h"

int loadPackages(Package **p) {
    char buffer[1024];
    int packageCount = 0;

    FILE *fp = popen("dpkg-query -W -f='${Package}\t${Version}\t${binary:Summary}\n'", "r");
    if (!fp) {
        fprintf(stderr, "Failed to run command\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        packageCount++;
    }
    pclose(fp);

    fp = popen("dpkg-query -W -f='${Package}\t${Version}\t${binary:Summary}\n'", "r");
    if (!fp) {
        fprintf(stderr, "Failed to re-run command\n");
        exit(EXIT_FAILURE);
    }

    *p = (Package *)calloc(packageCount, sizeof(Package));
    if (!*p) {
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    int index = 0;
    while (fgets(buffer, sizeof(buffer), fp) && index < packageCount) {
        char *name = strtok(buffer, "\t");
        char *version = strtok(NULL, "\t");
        char *description = strtok(NULL, "\n");

        (*p)[index].name = safeStrdup(name ? name : "(Unknown)");
        (*p)[index].version = safeStrdup(version ? version : "(Unknown)");
        (*p)[index].description = safeStrdup(description ? description : "(No description)");
        index++;
    }
    pclose(fp);
    return packageCount;
}

void managePackage(Package *package, int action) {
    char command[256];
    clear();

    switch (action) {
        case 0: // 패키지 설치
            snprintf(command, sizeof(command), "sudo apt-get install -y %s 2>&1", package->name);
            mvprintw(0, 0, "Installing package: %s", package->name);
            break;
        case 1: // 패키지 제거
            snprintf(command, sizeof(command), "sudo apt-get remove -y %s 2>&1", package->name);
            mvprintw(0, 0, "Removing package: %s", package->name);
            break;
        case 2: // 시스템 업데이트 및 업그레이드
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

    char buffer[1024];
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

    int result = pclose(fp);

    if (result == 0) {
        if (action == 2) {
            mvprintw(row, 0, "System updated and upgraded successfully.");
        } else {
            mvprintw(row, 0, "Package '%s' was %s successfully.", 
                     package->name, action == 0 ? "installed" : "removed");
        }
    } else {
        if (action == 2) {
            mvprintw(row, 0, "Error: Failed to update and upgrade the system.");
        } else {
            mvprintw(row, 0, "Error: Failed to %s package '%s'.", 
                     action == 0 ? "install" : "remove", package->name);
        }
    }

    refresh();
    if (action != 2) { 
        mvprintw(ROWS - 1, 0, "Press any key to return to the list.");
        getch();
        clear();
    } else {
        sleep(2);
        clear();
    }
}

void searchPackage() {
    int count = 0;
    char query[256];
    char command[512];
    char buffer[1024];
    char packages[1024][256]; 
    Package *p = NULL;
    int currIndex = 0, startIndex = 0, exitFlag = 0, prevCh = 0;

    mvprintw(ROWS - 1, 0, "/");
    clrtoeol();
    echo();

    getstr(query);
    noecho();
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
            if (name && count < 1024) {
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
    p = (Package *)malloc(count * sizeof(Package));
    if (!p) {
        mvprintw(2, 0, "Memory allocation failed.");
        refresh();
        getch();
        return;
    }

    // 각 패키지 이름을 이용해 정보 추출
    for (int i = 0; i < count; i++) {
        snprintf(command, sizeof(command), "apt show %s 2>/dev/null", packages[i]);
        FILE *fp = popen(command, "r");
        if (!fp) {
            fprintf(stderr, "Failed to fetch details for package: %s\n", packages[i]);
            continue;
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
            p[i].name = name;
            p[i].version = version;
            p[i].description = description;
        } else {
            safeFree(&name);
            safeFree(&version);
            safeFree(&description);
        }
    }

    // 패키지 목록 출력 및 키 입력 처리
    while (!exitFlag) {
        if (currIndex < startIndex) {
            startIndex = currIndex;
        } else if (currIndex >= startIndex + ROWS - 3) {
            startIndex = currIndex - (ROWS - 3) + 1;
        }
        displayPackages(p, count, startIndex, currIndex);
        keyInput(&currIndex, &startIndex, &prevCh, &exitFlag, count, p);
    }

    // 메모리 해제
    for (int i = 0; i < count; i++) {
        safeFree(&p[i].name);
        safeFree(&p[i].version);
        safeFree(&p[i].description);
    }
    free(p);
}



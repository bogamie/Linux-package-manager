#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include <sys/ioctl.h>

struct winsize w;

typedef struct Package {
    char *name;
    char *version;
    char *description;
} Package;

int ROWS, COLS;

void init();
void deinit(Package *p, int packageCount);
int loadPackages(Package **p);
void displayPackages(Package *p, int packageCount, int startIndex, int currentIndex);
void getMaxLen(Package *p, int packageCount, int *maxNameLen, int *maxVersionLen);
char *safeStrdup(const char *str);
void safeFree(char **ptr);
void printLine(const char *line, int row, bool highlight);
void keyInput(int *currIndex, int *startIndex, int *prevCh, int *exitFlag, int packageCount, Package *p);
void displayPackagesDetail(Package *p);
void searchPackage();
void displaySearchResults(const char *query);
void managePackage(Package *package, int action);
void printHelp();

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

void init() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    if (ioctl(0, TIOCGWINSZ, &w) < 0) {
        ROWS = 24;
        COLS = 80;
    } else {
        ROWS = w.ws_row;
        COLS = w.ws_col;
    }
    refresh();
}

void deinit(Package *p, int packageCount) {
    if (p) {
        for (int i = 0; i < packageCount; i++) {
            safeFree(&p[i].name);
            safeFree(&p[i].version);
            safeFree(&p[i].description);
        }
        free(p);
    }
    endwin();
}

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

void displayPackages(Package *p, int packageCount, int startIndex, int currentIndex) {
    erase();

    int maxNameLen, maxVersionLen;
    getMaxLen(p, packageCount, &maxNameLen, &maxVersionLen);
    int maxDescLen = COLS - maxNameLen - maxVersionLen - 2;

    char header[COLS + 1];
    snprintf(header, sizeof(header), "%-*s %-*s %-.*s", 
             maxNameLen, "Package", 
             maxVersionLen, "Version", 
             maxDescLen, "Description");
    printLine(header, 0, false);
    mvhline(1, 0, '-', COLS);

    int endIndex = startIndex + ROWS - 3;
    if (endIndex > packageCount) endIndex = packageCount;

    for (int i = startIndex; i < endIndex; i++) {
        char line[COLS + 1];
        snprintf(line, sizeof(line), "%-*s %-*s ", 
                 maxNameLen, p[i].name, 
                 maxVersionLen, p[i].version);
        strncat(line, p[i].description, maxDescLen);

        if (strlen(p[i].description) > maxDescLen) {
            line[COLS - 1] = '>';
        }
        printLine(line, 2 + (i - startIndex), i == currentIndex);
    }

    refresh();
}

void getMaxLen(Package *p, int packageCount, int *maxNameLen, int *maxVersionLen) {
    *maxNameLen = *maxVersionLen = 0;

    for (int i = 0; i < packageCount; i++) {
        int nameLen = strlen(p[i].name);
        int versionLen = strlen(p[i].version);

        if (nameLen > *maxNameLen) *maxNameLen = nameLen;
        if (versionLen > *maxVersionLen) *maxVersionLen = versionLen;
    }
}

char *safeStrdup(const char *str) {
    return str ? strdup(str) : strdup("");
}

void safeFree(char **ptr) {
    if (*ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

void printLine(const char *line, int row, bool highlight) {
    if (highlight) attron(COLOR_PAIR(1));
    mvprintw(row, 0, "%s", line);
    if (highlight) attroff(COLOR_PAIR(1));
}

// Update keyInput function to handle installation and removal
void keyInput(int *currIndex, int *startIndex, int *prevCh, int *exitFlag, int packageCount, Package *p) {
    int ch = getch();
    if (ch == 'g' && *prevCh == 'g') {
        *currIndex = 0;
        *prevCh = 0;
    } else {
        switch (ch) {
            case 'j':
                if (*currIndex < packageCount - 1) (*currIndex)++;
                break;
            case 'k':
                if (*currIndex > 0) (*currIndex)--;
                break;
            case 'G':
                *currIndex = packageCount - 1;
                break;
            case '\n':
                displayPackagesDetail(&p[*currIndex]);
                break;
            case '/':
                searchPackage();
                break;
            case 'i':
                managePackage(&p[*currIndex], true); // Install package
                break;
            case 'd':
                managePackage(&p[*currIndex], false); // Remove package
                break;
            case 'q':
                *exitFlag = 1;
                break;
            default:
                break;
        }
        *prevCh = ch;
    }
}

void displayPackagesDetail(Package *package) {
    clear();

    mvprintw(0, 0, "Package Details");
    mvhline(1, 0, '-', COLS);

    // apt-cache command to fetch detailed package info
    char command[256];
    snprintf(command, sizeof(command), "apt-cache show %s", package->name);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        mvprintw(4, 0, "Failed to retrieve package details.");
        mvprintw(6, 0, "Press 'q' to return to the package list.");
        refresh();
        return;
    }

    char buffer[1024];
    char lines[1000][1024];
    int lineCount = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL && lineCount < 1000) {
        size_t len = strlen(buffer);
        if (len > COLS) {
            int start = 0;
            while (start < len) {
                strncpy(lines[lineCount], &buffer[start], COLS - 1);
                lines[lineCount][COLS - 1] = '\0';
                lineCount++;
                start += COLS - 1;
                if (lineCount >= 1000) break;
            }
        } else {
            strncpy(lines[lineCount++], buffer, sizeof(buffer));
        }
    }

    pclose(fp);

    int offset = 0;
    int ch;

    while (true) {
        erase();

        mvprintw(0, 0, "Package Details");
        mvhline(1, 0, '-', COLS);

        int displayLines = ROWS - 3;
        for (int i = 0; i < displayLines && offset + i < lineCount; i++) {
            mvprintw(2 + i, 0, "%s", lines[offset + i]);
        }

        mvprintw(ROWS - 1, 0, "Press 'q' to return");
        refresh();

        ch = getch();
        if (ch == 'q') {
            clear();
            break;
        } else if (ch == 'j' && offset + displayLines < lineCount) {
            offset++;
        } else if (ch == 'k' && offset > 0) {
            offset--;
        }
    }
}

void searchPackage() {
    int count = 0;
    char query[256];
    char command[512];
    char buffer[1024];
    char packages[1024][256]; // 최대 1024개의 패키지 이름 저장 가능
    Package *p = NULL;
    int currIndex = 0, startIndex = 0, exitFlag = 0, prevCh = 0;

    mvprintw(ROWS - 1, 0, "/");
    clrtoeol();
    echo();

    getstr(query); // 사용자 입력 받기
    noecho();
    clear();
    mvprintw(0, 0, "Search results for '%s':", query);
    mvhline(1, 0, '-', COLS);

    // 명령어 생성
    snprintf(command, sizeof(command), "apt search %s 2>/dev/null", query);

    FILE *fp = popen(command, "r");
    if (!fp) {
        mvprintw(2, 0, "Failed to run apt search command.");
        mvprintw(ROWS - 1, 0, "Press any key to return.");
        refresh();
        getch();
        return;
    }

    // 검색 결과 처리
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
                packages[count][sizeof(packages[count]) - 1] = '\0'; // Null-terminate
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

// Install or remove the selected package
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

void printHelp() {
    printf("Package Manager Program\n");
    printf("========================\n");
    printf("Usage: ./main [options]\n");
    printf("\nOptions:\n");
    printf("  -help                Show this help message and exit.\n");
    printf("\nKeyboard Shortcuts:\n");
    printf("  j        Move down the package list.\n");
    printf("  k        Move up the package list.\n");
    printf("  G        Jump to the bottom of the list.\n");
    printf("  gg       Jump to the top of the list.\n");
    printf("  i        Install the selected package.\n");
    printf("  d        Remove the selected package.\n");
    printf("  u        Update and upgrade the system.\n");
    printf("  /        Search for a package.\n");
    printf("  Enter    Show details of the selected package.\n");
    printf("  q        Quit the program.\n");
    printf("\nExample:\n");
    printf("  ./main              Launch the program.\n");
    printf("  ./main -help        Show this help message.\n");
}

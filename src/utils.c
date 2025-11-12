#include <ncurses.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "ui.h"
#include "constants.h"

int ROWS, COLS;
static struct winsize w;

bool isValidPackageName(const char *name) {
    if (!name || strlen(name) == 0 || strlen(name) > MAX_PACKAGE_NAME_LENGTH) {
        return false;
    }
    
    for (int i = 0; name[i]; i++) {
        char c = name[i];
        // 패키지 이름에 허용되는 문자만: 영문, 숫자, 하이픈, 점, 언더스코어, 플러스, 콜론
        if (!isalnum(c) && c != '-' && c != '.' && c != '_' && c != '+' && c != ':') {
            return false;
        }
    }
    return true;
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
    
    // 최소 화면 크기 확인
    if (ROWS < MIN_ROWS || COLS < MIN_COLS) {
        endwin();
        fprintf(stderr, "Terminal size too small. Minimum: %dx%d\n", MIN_COLS, MIN_ROWS);
        exit(EXIT_FAILURE);
    }
    
    refresh();
}

void deinit(Package *p, int packageCount) {
    freePackages(p, packageCount);
    endwin();
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

void getMaxLen(Package *p, int packageCount, int *maxNameLen, int *maxVersionLen) {
    *maxNameLen = *maxVersionLen = 0;

    for (int i = 0; i < packageCount; i++) {
        int nameLen = strlen(p[i].name);
        int versionLen = strlen(p[i].version);

        if (nameLen > *maxNameLen) *maxNameLen = nameLen;
        if (versionLen > *maxVersionLen) *maxVersionLen = versionLen;
    }
}

void keyInput(int *currIndex, int *startIndex, int *prevCh, int *exitFlag, int packageCount, Package *p) {
    if (!currIndex || !startIndex || !prevCh || !exitFlag || !p) {
        return;
    }
    
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
                managePackage(&p[*currIndex], ACTION_INSTALL);
                break;
            case 'd':
                managePackage(&p[*currIndex], ACTION_REMOVE);
                break;
            case 'u':
                managePackage(NULL, ACTION_UPDATE);
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



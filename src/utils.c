#include <ncurses.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "ui.h"

int ROWS, COLS;
static struct winsize w;

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



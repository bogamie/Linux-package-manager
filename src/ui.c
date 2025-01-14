#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "ui.h"
#include "utils.h"

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

void displayPackagesDetail(Package *package) {
    clear();

    mvprintw(0, 0, "Package Details");
    mvhline(1, 0, '-', COLS);

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

void printLine(const char *line, int row, bool highlight) {
    if (highlight) attron(COLOR_PAIR(1));
    mvprintw(row, 0, "%s", line);
    if (highlight) attroff(COLOR_PAIR(1));
}


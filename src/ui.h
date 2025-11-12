#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "package_manager.h"

void displayPackages(Package *p, int packageCount, int startIndex, int currentIndex);
void displayPackagesDetail(Package *package);
void printHelp(void);
void printLine(const char *line, int row, bool highlight);

#endif // UI_H

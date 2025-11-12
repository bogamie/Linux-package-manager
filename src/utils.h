#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "package_manager.h"

extern int ROWS, COLS;

void init();
void deinit(Package *p, int packageCount);
char *safeStrdup(const char *str);
void safeFree(char **ptr);
void getMaxLen(Package *p, int packageCount, int *maxNameLen, int *maxVersionLen);
void keyInput(int *currIndex, int *startIndex, int *prevCh, int *exitFlag, int packageCount, Package *p);
bool isValidPackageName(const char *name);

#endif // UTILS_H

#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include "constants.h"

typedef struct Package {
    char *name;
    char *version;
    char *description;
} Package;

int loadPackages(Package **p);
void managePackage(Package *package, int action);
void searchPackage();
void freePackages(Package *p, int count);

#endif // PACKAGE_MANAGER_H

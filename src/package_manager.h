#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

typedef struct Package {
    char *name;
    char *version;
    char *description;
} Package;

int loadPackages(Package **p);
void managePackage(Package *package, int action);
void searchPackage();

#endif // PACKAGE_MANAGER_H

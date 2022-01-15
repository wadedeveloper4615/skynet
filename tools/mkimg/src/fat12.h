#ifndef FAT12_H_INCLUDED
#define FAT12_H_INCLUDED

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "file.h"
#include "fat.h"
#include "FATFileSystem.h"

class FAT12 : public FATFileSystem
{
private:
    FAT1216BootSectorPtr bootSector;
public:
    FAT12(File *file);
    virtual ~FAT12();

    void create();
};

#endif // FAT12_H_INCLUDED

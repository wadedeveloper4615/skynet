#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "fat12.h"

FAT12::FAT12(File* file)
{
    bootSector = new FAT1216BootSector;
    this->file=file;
    file->open(READ);
}

FAT12::~FAT12()
{
    delete bootSector;
    file->close();
}

#include "fat12.h"

FAT12::FAT12(File* file)
{
    bootSector = new FAT1216BootSector;
    this->file=file;
    file->open(WRITE);
    file->seek(SEEK_SET,0);
}

FAT12::~FAT12()
{
    delete bootSector;
    file->close();
}

void FAT12::create(){
}


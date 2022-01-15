#ifndef FAT12_H_INCLUDED
#define FAT12_H_INCLUDED

#include <string>
#include "file.h"
#include "fat.h"
#include "FATFileSystem.h"

class FAT12 : public FATFileSystem
{
private:
    FAT1216BootSectorPtr bootSector;
public:
    FAT12(char *filename);
    virtual ~FAT12();
};

#endif // FAT12_H_INCLUDED

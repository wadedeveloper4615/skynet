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
    int8u *fat;
    int32u FATSectorStart;
    int32u FATSectorSize;
    int32u RootSectorStart;
    int32u RootSectorSize;
    int32u DataSectorStart;
    int32u DataSectorSize;
    int32u clusterSize;
    DirEntryFatPtr rootDir;
    std::string *list;
    int32s numberOfEntries;
    int32s numberOfLFNEntries;
public:
    FAT12(char *filename);
    virtual ~FAT12();
    void parse();
    void print();
    void printRootDir();
protected:
    char *extractLongFileName(char *name,int32s size);
    char *extractShortFileName(char *name,int32s size);
    char *getAttrString(DirEntryFatPtr entry);
    char *getModifiedTime(DirEntryFatPtr entry);
    char *getModifiedDate(DirEntryFatPtr entry);
    int32u getNextCluster(int32u cluster);
    int32u getClusterSectorOffset(int32u cluster);
};

#endif // FAT12_H_INCLUDED

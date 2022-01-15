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
    BYTE *fat;
    DWORD FATSectorStart;
    DWORD RootSectorStart;
    DWORD DataSectorStart;
    DWORD clusterSize;
    DirEntryFatPtr rootDir;
    std::string *list;
    int numberOfEntries;
public:
    FAT12(char *filename);
    virtual ~FAT12();
    void parse();
    void print();
    void printRootDir();
protected:
    char *extractLongFileName(char *name,int size);
    char *extractShortFileName(char *name,int size);
    char *getAttrString(char *attrs, int size, DirEntryFatPtr entry);
    char *getModifiedTime(char *buffer, int size, DirEntryFatPtr entry);
    char *getModifiedDate(char *buffer, int size, DirEntryFatPtr entry);
    DWORD getNextCluster(DWORD cluster);
    DWORD getClusterSectorOffset(DWORD cluster);
};

#endif // FAT12_H_INCLUDED

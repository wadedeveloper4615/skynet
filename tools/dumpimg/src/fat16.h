#ifndef FAT16_H_INCLUDED
#define FAT16_H_INCLUDED

#include <string>
#include "file.h"
#include "fat.h"
#include "FATFileSystem.h"

class FAT16 : public FATFileSystem
{
private:
    MBR *mbr;
    FAT1216BootSectorPtr bootSector;
    WORD *fat;
    DWORD partitionStart;
    DWORD FATSectorStart;
    DWORD RootSectorStart;
    DWORD DataSectorStart;
    DWORD clusterSize;
    DirEntryFatPtr rootDir;
    std::string *list;
    int numberOfEntries;
    boolean knownMBR;
public:
    FAT16(char *filename, boolean knownMBR);
    virtual ~FAT16();
    void parse();
    void print();
    void printRootDir();
protected:
    void parseNoMBR();
    void parseMBR();
    char *extractLongFileName(char *name,int size);
    char *extractShortFileName(char *name,int size);
    char *getAttrString(char *attrs, int size, DirEntryFatPtr entry);
    char *getModifiedTime(char *buffer, int size, DirEntryFatPtr entry);
    char *getModifiedDate(char *buffer, int size, DirEntryFatPtr entry);
    DWORD getNextCluster(DWORD cluster);
    DWORD getClusterSectorOffset(DWORD cluster);
};

#endif // FAT16_H_INCLUDED

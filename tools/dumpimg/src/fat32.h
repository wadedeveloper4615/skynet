#ifndef FAT32_H_INCLUDED
#define FAT32_H_INCLUDED
#include <string>
#include "file.h"
#include "fat.h"
#include "FATFileSystem.h"

class FAT32 : public FATFileSystem
{
private:
    MBR *mbr;
    FAT32BootSectorPtr bootSector;
    FAT32FSInfoPtr fileSystemInfo;
    DWORD *fat;
    DWORD partitionStart;
    DWORD FATSectorStart;
    DWORD rootStart;
    DWORD clusterSize;
    DirEntryFatPtr rootDir;
    std::string *list;
    int numberOfEntries;
    boolean knownMBR;
public:
    FAT32(char *filename, boolean knownMBR);
    virtual ~FAT32();
    void parse();
    void print();
    void printRootDir();
protected:
    void parseMBR();
    void parseNoMBR();
    DWORD clusterToOffset(DWORD cluster);
    char *extractLongFileName(char *name,int size);
    char *extractShortFileName(char *name,int size);
    char *getAttrString(char *attrs, int size, DirEntryFatPtr entry);
    char *getModifiedTime(char *buffer, int size, DirEntryFatPtr entry);
    char *getModifiedDate(char *buffer, int size, DirEntryFatPtr entry);
    void dumpDirEntry(DirEntryFatPtr entry);
    DWORD getNextCluster(DWORD cluster);
    DWORD getClusterSectorOffset(DWORD cluster);
    DWORD getClusterByteOffset(DWORD cluster);
};

#endif // FAT32_H_INCLUDED

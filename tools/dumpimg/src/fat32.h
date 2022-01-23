#ifndef FAT32_H_INCLUDED
#define FAT32_H_INCLUDED
#include <string>
#include "file.h"
#include "fat.h"
#include "FATFileSystem.h"

#define FAT32_FAT_BAD    0x0FFFFFF7
#define FAT32_FAT_LAST   0x0FFFFFFF

class FAT32 : public FATFileSystem
{
private:
    MBR *mbr;
    FAT32BootSectorPtr bootSector;
    FAT32FSInfoPtr fileSystemInfo;
    DWORD *fat;
    DWORD FATSectorStart;
    DWORD FATSectorSize;
    DWORD RootDirSectorStart;
    DWORD RootDirSectorsSize;
    DWORD DataSectorStart;
    DWORD DataSectorSize;
    DWORD clusterSize;
    DirEntryFatPtr rootDir;
    std::string *list;
    int numberOfEntries;
    int numberOfLFNEntries;
    boolean knownMBR;
    DWORD partitionStart;
    DWORD partitionSize;
    DWORD root_size;
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
    char *getAttrString(DirEntryFatPtr entry);
    char *getModifiedTime(char *buffer, int size, DirEntryFatPtr entry);
    char *getModifiedDate(char *buffer, int size, DirEntryFatPtr entry);
    void dumpDirEntry(DirEntryFatPtr entry);
    DWORD getClusterSectorOffset(DWORD cluster);
    DWORD getClusterByteOffset(DWORD cluster);
    DWORD getNextCluster(DWORD cluster);
    DWORD getRootDirSizeinClusters();
};

#endif // FAT32_H_INCLUDED

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
    int32u *fat;
    int32u FATSectorStart;
    int32u FATSectorSize;
    int32u RootDirSectorStart;
    int32u RootDirSectorsSize;
    int32u DataSectorStart;
    int32u DataSectorSize;
    int32u clusterSize;
    DirEntryFatPtr rootDir;
    std::string *list;
    int32s numberOfEntries;
    int32s numberOfLFNEntries;
    boolean knownMBR;
    int32u partitionStart;
    int32u partitionSize;
    int32u root_size;
public:
    FAT32(char *filename, boolean knownMBR);
    virtual ~FAT32();
    void parse();
    void print();
    void printRootDir();
protected:
    void parseMBR();
    void parseNoMBR();
    int32u clusterToOffset(int32u cluster);
    char *extractLongFileName(char *name,int32s size);
    char *extractShortFileName(char *name,int32s size);
    char *getAttrString(DirEntryFatPtr entry);
    char *getModifiedTime(DirEntryFatPtr entry);
    char *getModifiedDate(DirEntryFatPtr entry);
    void dumpDirEntry(DirEntryFatPtr entry);
    int32u getClusterSectorOffset(int32u cluster);
    int32u getClusterByteOffset(int32u cluster);
    int32u getNextCluster(int32u cluster);
    int32u getRootDirSizeinClusters();
};

#endif // FAT32_H_INCLUDED

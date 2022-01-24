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
    int16u *fat;
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
    boolean knownMBR;
    int32u partitionStart;
public:
    FAT16(char *filename, boolean knownMBR);
    virtual ~FAT16();
    void parse();
    void print();
    void printRootDir();
protected:
    void parseNoMBR();
    void parseMBR();
    char *extractLongFileName(char *name,int32s size);
    char *extractShortFileName(char *name,int32s size);
    char *getAttrString(DirEntryFatPtr entry);
    char *getModifiedTime(DirEntryFatPtr entry);
    char *getModifiedDate(DirEntryFatPtr entry);
    int32u getNextCluster(int32u cluster);
    int32u getClusterSectorOffset(int32u cluster);
};

#endif // FAT16_H_INCLUDED

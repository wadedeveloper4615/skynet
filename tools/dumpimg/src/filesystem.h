#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "file.h"
#include "FATFileSystem.h"
#include "fat12.h"
#include "fat16.h"
#include "fat32.h"
#include "exfat.h"

#include "FatData.h"
#include "Fat.h"

#define MAX_SECT_SIZE   4096

#define DEFAULT_SECTOR_SIZE       512
#define BLOCK_SIZE         1024

typedef enum _FSType{
 FS_UNKNOWN = -1,
 FS_EXFAT   = 1,
 FS_FAT12   = 2,
 FS_FAT16   = 3,
 FS_FAT32   = 4
}FSType;

typedef enum _DeviceType
{
    TYPE_UNKNOWN,   /* type could not be determined */
    TYPE_BAD,       /* neither file nor block device */
    TYPE_FILE,      /* image file rather than device */
    TYPE_VIRTUAL,   /* block devices like LVM or RAID volumes */
    TYPE_REMOVABLE, /* removable disk device */
    TYPE_FIXED      /* fixed disk device */
}DeviceType;

typedef struct _DeviceInfo
{
    DeviceType type;
    int partition;
    int has_children;
    int geom_heads;
    int geom_sectors;
    long long geom_start;
    long long geom_size;
    int sector_size;
    long long size;
} DeviceInfo,*DeviceInfoPtr;

class FileSystem
{
private:
    char *filename;
    File *io;
    FATFileSystem *fs;
public:
    FileSystem(char *filename);
    virtual ~FileSystem();
    void parse();
public:
protected:
    BOOL power_of_two(DWORD val);
    FSType DetectFatSize(FAT1216BootSectorPtr bpb);
    FSType DetectFat(FAT1216BootSectorPtr bpb);
    int DetectMBR(MBRPtr mbr);
    FSType DetectExFat(EXFATBootSectorPtr vbr);
};

#endif // FILESYSTEM_H

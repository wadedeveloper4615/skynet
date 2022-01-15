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
    void createImage(DWORD blockSize, DWORD numberOfBlocks);
    void gatherDeviceInformation(FSType fileSystemType);
public:
    __int64 info();
    DeviceInfo devInfo;
    void establish_params();
    struct msdos_boot_sector bs;	/* Boot sector data */
    int size_fat;	/* Size in bits of FAT entries */
    int root_dir_entries;	/* Number of root directory entries */
    unsigned hidden_sectors;	/* Number of hidden sectors */
    int sectors_per_cluster;	/* Number of sectors per disk cluster */
    int nr_fats;		/* Default number of FATs to produce */
    unsigned long long cblocks;
    off_t part_sector; /* partition offset in sector */
    int orphaned_sectors;	/* Sectors that exist in the last block of filesystem */
protected:
    BOOL power_of_two(DWORD val);
    FSType DetectFatSize(FAT1216BootSectorPtr bpb);
    FSType DetectFat(FAT1216BootSectorPtr bpb);
    int DetectMBR(MBRPtr mbr);
    FSType DetectExFat(EXFATBootSectorPtr vbr);
    off_t fsize();
protected:
    long blkdev_valid_offset(int handle,off_t offset);
    int is_blkdev(int handle);

    void get_block_device_size(int handle,DeviceInfoPtr info);
    void get_block_geometry(DeviceInfoPtr info, dev_t rdev);
    void get_sector_size(DeviceInfoPtr info);

    off_t blkdev_find_size(int handle);
    int blkdev_get_size(int handle,unsigned long long *bytes);
    int blkdev_get_geometry(unsigned int *h, unsigned int *s);
    int blkdev_get_start(dev_t rdev, unsigned long long *s);
    int blkdev_get_sector_size(int *sector_size);
    void setup_tables(FSType fileSystemType);
    __int64 info(int handle);
    uint32_t generate_volume_id(void);
};

#endif // FILESYSTEM_H

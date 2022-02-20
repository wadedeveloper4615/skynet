#ifndef MKFS_H_INCLUDED
#define MKFS_H_INCLUDED

#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <wchar.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>
#include <iconv.h>
#include "version.h"

#define DEFAULT_DOS_CODEPAGE 850
#define TRUE 1			/* Boolean constants */
#define FALSE 0
#define TEST_BUFFER_BLOCKS 16
#define BLOCK_SIZE         1024
#define HARD_SECTOR_SIZE   512
#define SECTORS_PER_BLOCK ( BLOCK_SIZE / HARD_SECTOR_SIZE )
#define NO_NAME "NO NAME    "
#ifndef OFF_MAX
	#define OFF_MAX (off_t)((1ULL << (sizeof(off_t) * CHAR_BIT - 1)) - 1)
#endif
#define mark_sector_bad( sector ) mark_FAT_sector( sector, FAT_BAD )

/* FAT values */
#define FAT_EOF      (atari_format ? 0x0fffffff : 0x0ffffff8)
#define FAT_BAD      0x0ffffff7
#define MSDOS_EXT_SIGN 0x29	/* extended boot sector signature */
#define MSDOS_FAT12_SIGN "FAT12   "	/* FAT12 filesystem signature */
#define MSDOS_FAT16_SIGN "FAT16   "	/* FAT16 filesystem signature */
#define MSDOS_FAT32_SIGN "FAT32   "	/* FAT32 filesystem signature */
#define BOOT_SIGN 0xAA55	/* Boot sector magic number */
/* According to Microsoft FAT specification (fatgen103.doc) disk with
 * 4085 clusters (or more) is FAT16, but Microsoft Windows FAT driver
 * fastfat.sys detects disk with less then 4087 clusters as FAT12.
 * Linux FAT drivers msdos.ko and vfat.ko detect disk with at least
 * 4085 clusters as FAT16, therefore for compatibility reasons with
 * both systems disallow formatting disks to 4085 or 4086 clusters. */
#define MAX_CLUST_12	4084
#define MIN_CLUST_16	4087
/* According to Microsoft FAT specification (fatgen103.doc) disk with
 * 65525 clusters (or more) is FAT32, but Microsoft Windows FAT driver
 * fastfat.sys, Linux FAT drivers msdos.ko and vfat.ko detect disk as
 * FAT32 when Sectors Per FAT (fat_length) is set to zero. And not by
 * number of clusters. Still there is cluster upper limit for FAT16. */
#define MAX_CLUST_16	65524
#define MIN_CLUST_32	65525
/* M$ says the high 4 bits of a FAT32 FAT entry are reserved and don't belong
 * to the cluster number. So the max. cluster# is based on 2^28 */
#define MAX_CLUST_32	268435446
#define OLDGEMDOS_MAX_SECTORS	32765
#define GEMDOS_MAX_SECTORS	65531
#define GEMDOS_MAX_SECTOR_SIZE	(16*1024)
#define BOOTCODE_SIZE		448
#define BOOTCODE_FAT32_SIZE	420
#define MSG_OFFSET_OFFSET 3
#define MESSAGE_OFFSET 29	/* Offset of message in above code */
#define SECTOR_SIZE 512		/* sector size (bytes) */
#define MSDOS_DPS (SECTOR_SIZE / sizeof(struct msdos_dir_entry))
#define MSDOS_DPS_BITS 4	/* log2(MSDOS_DPS) */
#define MSDOS_DIR_BITS 5	/* log2(sizeof(struct msdos_dir_entry)) */

#define ATTR_NONE 0	    /* no attribute bits */
#define ATTR_RO 1	    /* read-only */
#define ATTR_HIDDEN 2	/* hidden */
#define ATTR_SYS 4	    /* system */
#define ATTR_VOLUME 8	/* volume label */
#define ATTR_DIR 16	    /* directory */
#define ATTR_ARCH 32	/* archived */

/* attribute bits that are copied "as is" */
#define ATTR_UNUSED (ATTR_VOLUME | ATTR_ARCH | ATTR_SYS | ATTR_HIDDEN)

#define DELETED_FLAG 0xe5	/* marks file as deleted when in name[0] */
#define IS_FREE(n) (!*(n) || *(n) == DELETED_FLAG)
#define MSDOS_NAME 11			/* maximum name length */
#define MSDOS_DOT ".          "		/* ".", padded to MSDOS_NAME chars */
#define MSDOS_DOTDOT "..         "	/* "..", padded to MSDOS_NAME chars */

typedef struct _ProgramArguments
{
    bool create;
    off_t part_sector;                   /* partition offset in sector */
    int size_fat;	                     /* Size in bits of FAT entries */
    int size_fat_by_user;	             /* 1 if FAT size user selected */
    unsigned sector_size;	             /* Size of a logical sector */
    int sector_size_set;	             /* User selected sector size */
    int sectors_per_cluster;	         /* Number of sectors per disk cluster */
    char * device_name;
    int blocks_specified;
    unsigned long long blocks;           /* Number of blocks in filesystem */
    unsigned hidden_sectors;	         /* Number of hidden sectors */
    int hidden_sectors_by_user;	         /* -h option invoked */
    int root_dir_entries;	             /* Number of root directory entries */
    int root_dir_entries_set;	         /* User selected root directory size */
    int fat_media_byte;	                 /* media byte in header and starting FAT */
    int drive_number_by_user;	         /* drive number option invoked */
    int drive_number_option;	         /* drive number */
    long volume_id;                      /* Volume ID number */
    int reserved_sectors;	             /* Number of reserved sectors */
} ProgramArguments, *ProgramArgumentsPtr;

#pragma pack(push, 1)
struct msdos_volume_info
{
    uint8_t drive_number;	    /* BIOS drive number */
    uint8_t boot_flags;		    /* bit 0: dirty, bit 1: need surface test */
    uint8_t ext_boot_sign;	    /* 0x29 if fields below exist (DOS 3.3+) */
    uint8_t volume_id[4];	    /* Volume ID number */
    uint8_t volume_label[11];	/* Volume label */
    uint8_t fs_type[8];		    /* Typically FAT12 or FAT16 */
};

struct msdos_boot_sector
{
    uint8_t boot_jump[3];	/* Boot strap short or near jump */
    uint8_t system_id[8];	/* Name - can be used to special case  partition manager volumes */
    uint8_t sector_size[2];	/* bytes per logical sector */
    uint8_t cluster_size;	/* sectors/cluster */
    uint16_t reserved;		/* reserved sectors */
    uint8_t fats;		    /* number of FATs */
    uint8_t dir_entries[2];	/* root directory entries */
    uint8_t sectors[2];		/* number of sectors */
    uint8_t media;		    /* media code (unused) */
    uint16_t fat_length;	/* sectors/FAT */
    uint16_t secs_track;	/* sectors per track */
    uint16_t heads;		/* number of heads */
    uint32_t hidden;		/* hidden sectors (unused) */
    uint32_t total_sect;	/* number of sectors (if sectors == 0) */
    union
    {
        struct
        {
            struct msdos_volume_info vi;
            uint8_t boot_code[BOOTCODE_SIZE];
        } _oldfat;
        struct
        {
            uint32_t fat32_length;	/* sectors/FAT */
            uint16_t flags;		    /* bit 8: fat mirroring, low 4: active fat */
            uint8_t version[2];		/* major, minor filesystem version */
            uint32_t root_cluster;	/* first cluster in root directory */
            uint16_t info_sector;	/* filesystem info sector */
            uint16_t backup_boot;	/* backup boot sector */
            uint16_t reserved2[6];	/* Unused */
            struct msdos_volume_info vi;
            uint8_t boot_code[BOOTCODE_FAT32_SIZE];
        } _fat32;
    }  fstype;
    uint16_t boot_sign;
};
#define fat32	fstype._fat32
#define oldfat	fstype._oldfat
struct fat32_fsinfo
{
    uint32_t reserved1;		/* Nothing as far as I can tell */
    uint32_t signature;		/* 0x61417272L */
    uint32_t free_clusters;	/* Free cluster count.  -1 if unknown */
    uint32_t next_cluster;	/* Most recently allocated cluster. Unused under Linux. */
    uint32_t reserved2[4];
};

enum device_type {
    TYPE_UNKNOWN,   /* type could not be determined */
    TYPE_BAD,       /* neither file nor block device */
    TYPE_FILE,      /* image file rather than device */
    TYPE_VIRTUAL,   /* block devices like LVM or RAID volumes */
    TYPE_REMOVABLE, /* removable disk device */
    TYPE_FIXED      /* fixed disk device */
};

struct device_info {
    enum device_type type;
    /*
     * partition number if detected
     * 0  = whole disk device (including unpartitioned image file)
     * -1 = could not be determined
     */
    int partition;

    /*
     * whether partitions or device mapper devices or any other kind of
     * children use this device
     *  1 = yes
     *  0 = no
     * -1 = could not be determined
     */
    int has_children;

    /*
     * detected geometry, or -1 if unknown
     */
    int geom_heads;
    int geom_sectors;
    long long geom_start;
    long long geom_size;

    /*
     * detected sector size or -1 if unknown
     */
    int sector_size;

    /*
     * size in bytes, or -1 if unknown
     */
    long long size;
};

struct msdos_dir_entry {
    uint8_t name[MSDOS_NAME];	/* name including extension */
    uint8_t attr;		        /* attribute bits */
    uint8_t lcase;		        /* Case for base and extension */
    uint8_t ctime_cs;		    /* Creation time, centiseconds (0-199) */
    uint16_t ctime;		        /* Creation time */
    uint16_t cdate;		        /* Creation date */
    uint16_t adate;		        /* Last access date */
    uint16_t starthi;		    /* High 16 bits of cluster in FAT32 */
    uint16_t time, date, start;	/* time, date and first cluster */
    uint32_t size;		        /* file size (in bytes) */
};
#pragma pack(pop)

void die(const char *msg, ...);
#endif // MKFS_H_INCLUDED

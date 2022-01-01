#ifndef FAT_H_INCLUDED
#define FAT_H_INCLUDED

#include "types.h"

#define BOOTCODE_SIZE		          448
#define BOOTCODE_FAT32_SIZE	          420
#define NAME_LEN_MAX  512
#define POWERofTWO(x) (((x)!=0) && ((x) & ((x) - 1))==0)
#define SECT_SIZE  512
#define SECT_RES32  32   // sectors reserved

typedef enum _AttributeFat
{
    ATTR_READ_ONLY      = 0x01, //ATTR_READ_ONLY (Read-only)
    ATTR_HIDDEN         = 0x02, //ATTR_HIDDEN (Hidden)
    ATTR_SYSTEM         = 0x04, //ATTR_SYSTEM (System)
    ATTR_VOLUME_ID      = 0x08, //ATTR_VOLUME_ID (Volume label)
    ATTR_DIRECTORY      = 0x10, //ATTR_DIRECTORY (Directory)
    ATTR_ARCHIVE        = 0x20, //ATTR_ARCHIVE (Archive)
    ATTR_LONG_FILE_NAME = 0x0F  //ATTR_LONG_FILE_NAME (LFN entry)
} Attribute;

#pragma pack(push, 1)
struct msdos_volume_info
{
    int8u drive_number;	    /* BIOS drive number */
    int8u boot_flags;		/* bit 0: dirty, bit 1: need surface test */
    int8u ext_boot_sign;	/* 0x29 if fields below exist (DOS 3.3+) */
    int8u volume_id[4];	    /* Volume ID number */
    int8u volume_label[11];	/* Volume label */
    int8u fs_type[8];		/* Typically FAT12 or FAT16 */
};

struct msdos_boot_sector
{
    int8u boot_jump[3];	    /* Boot strap short or near jump */
    int8u system_id[8];	    /* Name - can be used to special case partition manager volumes */
    int8u sector_size[2];	/* bytes per logical sector */
    int8u cluster_size;	    /* sectors/cluster */
    int16u reserved;		/* reserved sectors */
    int8u fats;		        /* number of FATs */
    int8u dir_entries[2];	/* root directory entries */
    int8u sectors[2];		/* number of sectors */
    int8u media;		    /* media code (unused) */
    int16u fat_length;	    /* sectors/FAT */
    int16u secs_track;	    /* sectors per track */
    int16u heads;		    /* number of heads */
    int32u hidden;		    /* hidden sectors (unused) */
    int32u total_sect;	    /* number of sectors (if sectors == 0) */
    union
    {
        struct
        {
            struct msdos_volume_info vi;
            int8u boot_code[BOOTCODE_SIZE];
        } _oldfat;
        struct
        {
            int32u fat32_length;	/* sectors/FAT */
            int16u flags;		    /* bit 8: fat mirroring, low 4: active fat */
            int8u version[2];		/* major, minor filesystem version */
            int32u root_cluster;	/* first cluster in root directory */
            int16u info_sector;	    /* filesystem info sector */
            int16u backup_boot;	    /* backup boot sector */
            int16u reserved2[6];	/* Unused */
            struct msdos_volume_info vi;
            int8u boot_code[BOOTCODE_FAT32_SIZE];
        } _fat32;
    } fstype;
    int16u boot_sign;
};

struct fat32_fsinfo
{
    int32u reserved1;		/* Nothing as far as I can tell */
    int32u signature;		/* 0x61417272L */
    int32u free_clusters;	/* Free cluster count.  -1 if unknown */
    int32u next_cluster;	/* Most recently allocated cluster. Unused under Linux. */
    int32u reserved2[4];
} ;

typedef struct _Partition
{
    int8u status;           //0x80 for bootable, 0x00 for not bootable, anything else for invalid
    int8u StartAddrHead;    //head address of start of partition
    int16u StartAddrCylSec;  //(AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
    int8u type;
    int8u EndAddrHead;      //head address of start of partition
    int16u EndAddrCylSec;    //(AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
    int32u StartLBA;        //linear address of first sector in partition. Multiply by sector size (usually 512) for real offset
    int32u EndLBA;          //linear address of last sector in partition. Multiply by sector size (usually 512) for real offset
} Partition, *PartitionPtr;

typedef struct _MBR
{
    int8u Code[440];
    int32u DiskSig;
    int16u Reserved;
    Partition partition[4];
    int16u BootSignature;
} MBR,*MBRPtr;

// 36 byte structure
typedef struct _BioParameterBlock
{
    //                             offset size
    int8u   BS_jmpBoot[3];      // 0      3
    int8u   BS_OEMName[8];      // 3      8
    int16u  BPB_BytsPerSec;     //11      2
    int8u   BPB_SecPerClus;     //13      1
    int16u  BPB_RsvdSecCnt;     //14      2
    int8u   BPB_NumFATs;        //16      1
    int16u  BPB_RootEntCnt;     //17      2
    int16u  BPB_TotSec16;       //19      2
    int8u   BPB_Media;          //21      1
    int16u  BPB_FATSz16;        //22      2
    int16u  BPB_SecPerTrk;      //24      2
    int16u  BPB_NumHeads;       //26      2
    int32u  BPB_HiddSec;        //28      4
    int32u  BPB_TotSec32;       //32      4
} BioParameterBlock,*BioParameterBlockPtr;

//512 byte structure
typedef struct _FAT1216BootSector
{
    //                            offset size
    BioParameterBlock bpb;     //  0      36
    int8u   BS_DrvNum;         // 36       1
    int8u   BS_Reserved1;      // 37       1
    int8u   BS_BootSig;        // 38       1
    int32u  BS_VolID;          // 39       4
    int8u   BS_VolLab[11];     // 43      11
    int8u   BS_FilSysType[8];  // 54       8
    int8u   bootCode[448];     // 63     448
    int16u  bootSignature;     //510       2
} FAT1216BootSector, *FAT1216BootSectorPtr;

//512 byte structure
typedef struct _FAT32BootSector
{
    //                            offset size
    BioParameterBlock bpb;     //  0      36
    int32u  BPB_FATSz32;       // 36       4
    int16u  BPB_ExtFlags;      // 40       2
    int16u  BPB_FSVer;         // 42       2
    int32u  BPB_RootClus;      // 44       4
    int16u  BPB_FSInfo;        // 48       2
    int16u  BPB_BkBootSec;     // 50       2
    int8u   BPB_Reserved[12];  // 52      12
    int8u   BS_DrvNum;         // 64       1
    int8u   BS_Reserved1;      // 65       1
    int8u   BS_BootSig;        // 66       1
    int32u  BS_VolID;          // 67       4
    int8u   BS_VolLab[11];     // 71      11
    int8u   BS_FilSysType[8];  // 82       8
    int8u   bootCode[420];     // 90     420
    int16u  bootSignature;     //510       2
} FAT32BootSector, *FAT32BootSectorPtr;

typedef struct _FAT32FSInfo
{
    int32u  FSI_LeadSig;        // 0x41615252 ("RRaA")
    int8u   FSI_Reserved1[480]; // reserved
    int32u  FSI_StrucSig;       // 0x61417272 ("rrAa")
    int32u  FSI_Free_Count;     // -1 when the count is unknown
    int32u  FSI_Nxt_Free;       // most recent allocated cluster
    int8u   FSI_Reserved2[12];  // reserved
    int32u  FSI_TrailSig;       // 0xAA550000  (00 00 55 AA)
} FAT32FSInfo, *FAT32FSInfoPtr;

typedef struct _DSKSZTOSECPERCLUS
{
    int32u DiskSize;
    int8u SecPerClusVal;
}DSKSZTOSECPERCLUS,*DSKSZTOSECPERCLUSPtr;

extern DSKSZTOSECPERCLUS DskTableFAT16 [];
extern DSKSZTOSECPERCLUS DskTableFAT32 [];

typedef struct _DirEntry
{
    union
    {
        int8u  name[8];
        struct
        {
            int8u allocationStatus;
            int8u dummy[7];
        } ;
    } statusName;
    int8u   ext[3];
    int8u   attrb;
    union
    {
        int8u   resv[10];
        struct
        {
            int8u   nt_resv;
            int8u   crt_time_tenth;
            int16u   crt_time;
            int16u   crt_date;
            int16u   crt_last;
            int16u   strtclst32;
        } fat32;
    } type;
    int16u   time;
    int16u   date;
    int16u   strtclst;
    int32u  filesize;
} DirEntryFat,*DirEntryFatPtr;

typedef struct _LongDirEntry
{
    int8u   sequ_flags;
    int8u   name0[10];
    int8u   attrb;
    int8u   resv;
    int8u   sfn_crc;
    int8u   name1[12];
    int16u   clust_zero;
    int8u   name2[4];
} LongDirEntryFat,*LongDirEntryFatPtr;

typedef struct _ResourceData
{
    char mbr_filename[NAME_LEN_MAX];
    char boot_filename[NAME_LEN_MAX];
    char targ_filename[NAME_LEN_MAX];
    int32u file_cnt;
    int64u base_lba;
    int64u tot_sectors;
    int    fatType;
    int    spc;
    int    heads;
    int    spt;
} ResourceData, *ResourceDataPtr;
#pragma pack(pop)

#endif // FAT_H_INCLUDED

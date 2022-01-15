#ifndef FAT1216_H_INCLUDED
#define FAT1216_H_INCLUDED

#include "FatData.h"

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
    uint8_t drive_number;	/* BIOS drive number */
    uint8_t boot_flags;		/* bit 0: dirty, bit 1: need surface test */
    uint8_t ext_boot_sign;	/* 0x29 if fields below exist (DOS 3.3+) */
    uint8_t volume_id[4];	/* Volume ID number */
    uint8_t volume_label[11];	/* Volume label */
    uint8_t fs_type[8];		/* Typically FAT12 or FAT16 */
};

struct msdos_boot_sector
{
    uint8_t boot_jump[3];	/* Boot strap short or near jump */
    uint8_t system_id[8];	/* Name - can be used to special case partition manager volumes */
    uint8_t sector_size[2];	/* bytes per logical sector */
    uint8_t cluster_size;	/* sectors/cluster */
    uint16_t reserved;		/* reserved sectors */
    uint8_t fats;		/* number of FATs */
    uint8_t dir_entries[2];	/* root directory entries */
    uint8_t sectors[2];		/* number of sectors */
    uint8_t media;		/* media code (unused) */
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
            uint16_t flags;		/* bit 8: fat mirroring, low 4: active fat */
            uint8_t version[2];		/* major, minor filesystem version */
            uint32_t root_cluster;	/* first cluster in root directory */
            uint16_t info_sector;	/* filesystem info sector */
            uint16_t backup_boot;	/* backup boot sector */
            uint16_t reserved2[6];	/* Unused */
            struct msdos_volume_info vi;
            uint8_t boot_code[BOOTCODE_FAT32_SIZE];
        } _fat32;
    } fstype;
    uint16_t boot_sign;
};

struct fat32_fsinfo
{
    uint32_t reserved1;		/* Nothing as far as I can tell */
    uint32_t signature;		/* 0x61417272L */
    uint32_t free_clusters;	/* Free cluster count.  -1 if unknown */
    uint32_t next_cluster;	/* Most recently allocated cluster. Unused under Linux. */
    uint32_t reserved2[4];
} ;

typedef struct _Partition
{
    BYTE status;           //0x80 for bootable, 0x00 for not bootable, anything else for invalid
    BYTE StartAddrHead;    //head address of start of partition
    WORD StartAddrCylSec;  //(AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
    BYTE type;
    BYTE EndAddrHead;      //head address of start of partition
    WORD EndAddrCylSec;    //(AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
    DWORD StartLBA;        //linear address of first sector in partition. Multiply by sector size (usually 512) for real offset
    DWORD EndLBA;          //linear address of last sector in partition. Multiply by sector size (usually 512) for real offset
} Partition, *PartitionPtr;

typedef struct _MBR
{
    BYTE Code[440];
    DWORD DiskSig;
    WORD Reserved;
    Partition partition[4];
    WORD BootSignature;
} MBR,*MBRPtr;

typedef struct _BioParameterBlock
{
    BYTE   BS_jmpBoot[3];
    char   BS_OEMName[8];
    WORD   BPB_BytsPerSec;
    BYTE   BPB_SecPerClus;
    WORD   BPB_RsvdSecCnt;
    BYTE   BPB_NumFATs;
    WORD   BPB_RootEntCnt;
    WORD   BPB_TotSec16;
    BYTE   BPB_Media;
    WORD   BPB_FATSz16;
    WORD   BPB_SecPerTrk;
    WORD   BPB_NumHeads;
    DWORD  BPB_HiddSec;
    DWORD  BPB_TotSec32;
} BioParameterBlock,*BioParameterBlockPtr;

typedef struct _FAT1216BootSector
{
    BioParameterBlock bpb;
    BYTE   BS_DrvNum;
    BYTE   BS_Reserved1;
    BYTE   BS_BootSig;
    DWORD  BS_VolID;
    char   BS_VolLab[11];
    char   BS_FilSysType[8];
    BYTE   bootCode[448];
    WORD   bootSignature;
} FAT1216BootSector, *FAT1216BootSectorPtr;

typedef struct _FAT32BootSector
{
    BioParameterBlock bpb;
    DWORD  BPB_FATSz32;
    WORD   BPB_ExtFlags;
    WORD   BPB_FSVer;
    DWORD  BPB_RootClus;
    WORD   BPB_FSInfo;
    WORD   BPB_BkBootSec;
    BYTE   BPB_Reserved[12];
    BYTE   BS_DrvNum;
    BYTE   BS_Reserved1;
    BYTE   BS_BootSig;
    DWORD  BS_VolID;
    char   BS_VolLab[11];
    char   BS_FilSysType[8];
    BYTE   bootCode[420];
    WORD   bootSignature;
} FAT32BootSector, *FAT32BootSectorPtr;

typedef struct _FAT32FSInfo
{
    DWORD  FSI_LeadSig;        // 0x41615252 ("RRaA")
    BYTE   FSI_Reserved1[480]; // reserved
    DWORD  FSI_StrucSig;       // 0x61417272 ("rrAa")
    DWORD  FSI_Free_Count;     // -1 when the count is unknown
    DWORD  FSI_Nxt_Free;       // most recent allocated cluster
    BYTE   FSI_Reserved2[12];  // reserved
    DWORD  FSI_TrailSig;       // 0xAA550000  (00 00 55 AA)
} FAT32FSInfo, *FAT32FSInfoPtr;

typedef struct _DirEntry
{
    union
    {
        BYTE  name[8];
        struct
        {
            BYTE allocationStatus;
            BYTE dummy[7];
        } ;
    } statusName;
    BYTE   ext[3];
    BYTE   attrb;
    union
    {
        BYTE   resv[10];
        struct
        {
            BYTE   nt_resv;
            BYTE   crt_time_tenth;
            WORD   crt_time;
            WORD   crt_date;
            WORD   crt_last;
            WORD   strtclst32;
        } fat32;
    } type;
    WORD   time;
    WORD   date;
    WORD   strtclst;
    DWORD  filesize;
} DirEntryFat,*DirEntryFatPtr;

typedef struct _LongDirEntry
{
    BYTE   sequ_flags;
    BYTE   name0[10];
    BYTE   attrb;
    BYTE   resv;
    BYTE   sfn_crc;
    BYTE   name1[12];
    WORD   clust_zero;
    BYTE   name2[4];
} LongDirEntryFat,*LongDirEntryFatPtr;
#pragma pack(pop)



#endif // FAT1216_H_INCLUDED

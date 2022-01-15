#ifndef EXFAT_H
#define EXFAT_H

#include "file.h"
#include "FATFileSystem.h"

typedef unsigned long long QWORD;

#pragma pack(push, 1)
#define EXFAT_FAT_BAD    0xFFFFFFF7
#define EXFAT_FAT_LAST   0xFFFFFFFF

#define EXFAT_ATTR_ARCHIVE   0x20
#define EXFAT_ATTR_SUB_DIR   0x10
#define EXFAT_ATTR_SYSTEM    0x04
#define EXFAT_ATTR_HIDDEN    0x02
#define EXFAT_ATTR_READ_ONLY 0x01
#define EXFAT_ATTR_ALL       0x37


#define EXFAT_DIR_EOD        0x00   // end of directory
#define EXFAT_DIR_LABEL_UN   0x03   // Volume Name Record. Master Entry (Unnamed Volume)
#define EXFAT_DIR_DELETED    0x05   // Deleted File Name Record
#define EXFAT_DIR_DELETED_S  0x40   // Deleted File Name Record, Stream Extension
#define EXFAT_DIR_DELETED_F  0x41   // Deleted File Name Record, Filename Extension
#define EXFAT_DIR_BITMAP     0x81   // Bitmap Logical Location and Size
#define EXFAT_DIR_UCASE      0x82   // Up-Case Table Logical Location and Size
#define EXFAT_DIR_LABEL      0x83   // Volume Name Record, Master Entry (Named Volume)
#define EXFAT_DIR_ENTRY      0x85   // Directory Entry Record
#define EXFAT_DIR_STRM_EXT   0xC0   // Directory Entry Record, Stream Extension
#define EXFAT_DIR_NAME_EXT   0xC1   // Directory Entry Record, Filename Extension
#define EXFAT_DIR_WINCE_ACC  0xE2   //
#define EXFAT_DIR_GUID       0xA0   //
#define EXFAT_DIR_TEXFAT     0xA1   //

#define EXFAT_FLAGS_ALL_POS  (1<<0)
#define EXFAT_FLAGS_NO_FAT   (1<<1)

typedef struct _EXFATBootSector
{
    BYTE jmp[3];				/* 0x00 jmp and nop instructions */
    BYTE fileSystemName[8];		/* 0x03 "EXFAT   " */
    BYTE  unused1[53];  		/* 0x0B always 0 */
    QWORD PartitionOffset;		/* 0x40 partition first sector */
    QWORD VolumeLength;			/* 0x48 partition sectors count */
    DWORD FATOffset;    		/* 0x50 FAT first sector */
    DWORD FATLength;     		/* 0x54 FAT sectors count */
    DWORD ClusterHeapOffset;	/* 0x58 first cluster sector */
    DWORD ClusterCount;		    /* 0x5C total clusters count */
    DWORD FirstClusterOfRootDirectory; /* 0x60 first cluster of the root dir */
    DWORD volume_serial;		/* 0x64 volume serial number */
    struct
    {
        /* 0x68 FS version */;
        BYTE minor;
        BYTE major;
    } version;
    WORD volume_state;			/* 0x6A volume state flags */
    BYTE sector_bits;			/* 0x6C sector size as (1 << n) */
    BYTE spc_bits;				/* 0x6D sectors per cluster as (1 << n) */
    BYTE fat_count;				/* 0x6E always 1 */
    BYTE drive_no;				/* 0x6F always 0x80 */
    BYTE allocated_percent;		/* 0x70 percentage of allocated space */
    BYTE unused2[397];   		/* 0x71 always 0 */
    WORD boot_signature;		/* the value of 0xAA55 */
} EXFATBootSector,*EXFATBootSectorPtr;

struct S_GUID
{
    DWORD  data1;
    WORD   data2;
    WORD   data3;
    WORD   data4;
    BYTE   data5[6];
};

typedef struct _EXFATRoot
{
    BYTE  entry_type;
    union
    {
        struct
        {
            BYTE    sec_count;
            WORD    crc;
            WORD    attributes;
            WORD    resv1;
            DWORD   created;
            DWORD   last_mod;
            DWORD   last_acc;
            BYTE    created_ms;
            BYTE    last_mod_ms;
            BYTE    created_tz_offset;
            BYTE    last_mod_tz_offset;
            BYTE    last_access_tz_offset;
            BYTE    resv2[7];
        } dir_entry;
        struct
        {
            BYTE    flags;
            BYTE    resv1;
            BYTE    name_len;
            WORD    name_hash;
            WORD    resv2;
            DWORD64 valid_data_len;
            DWORD   resv3;
            DWORD   first_cluster;
            DWORD64 data_len;
        } stream_ext;
        struct
        {
            BYTE    flags;
            WORD    name[15];
        } file_name_ext;
        struct
        {
            BYTE    len;
            WORD    name[11];
            BYTE    resv1[8];
        } label;
        struct
        {
            BYTE    flags;
            BYTE    resv1[18];
            DWORD   cluster_strt;
            DWORD64 data_len;
        } bitmap;
        struct
        {
            BYTE    resv1[3];
            DWORD   crc;
            BYTE    resv2[12];
            DWORD   cluster_strt;
            DWORD64 data_len;
        } up_case_table;
        struct
        {
            BYTE    sec_count;  // always 0
            WORD    crc;
            WORD    flags;
            struct S_GUID guid;
            BYTE    resv1[10];
        } guid;
    } type;
} EXFATRoot, *EXFATRootPtr;

#pragma pack(pop)

class ExFat : public FATFileSystem
{
private:
    EXFATBootSectorPtr bootSector;
    DWORD fatAlignmentStart;
    DWORD fatAlignmentSize;
    DWORD fatStart;
    DWORD fatSize;
    DWORD *fat;
    DWORD root_size;
    DWORD bytesPerSector;
    DWORD sectorPerCluster;
    EXFATRootPtr root;
    DWORD rootSizeInBytes;
public:
    ExFat(char *filename);
    virtual ~ExFat();
    void parse();
    void print();
    void printRootDir();
protected:
    DWORD getNextCluster(DWORD cluster);
    DWORD getClusterSectorOffset(DWORD cluster);
    DWORD getRootDirSizeinClusters();
    char *extractShortFileName(char *name,int size);
private:
};

#endif // EXFAT_H

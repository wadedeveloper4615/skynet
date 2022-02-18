#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "filesystem.h"
#include "file.h"

#define BOOT_SIGN 0xAA55	/* Boot sector magic number */
#define MSDOS_NAME 11			/* maximum name length */
#define NO_NAME "NO NAME    "
unsigned char dummy_boot_jump[3] = { 0xeb, 0x3c, 0x90 };
#define MSG_OFFSET_OFFSET 3
char dummy_boot_code[BOOTCODE_SIZE] = "\x0e"	/* push cs */
                                      "\x1f"			/* pop ds */
                                      "\xbe\x5b\x7c"		/* mov si, offset message_txt */
                                      /* write_msg: */
                                      "\xac"			/* lodsb */
                                      "\x22\xc0"			/* and al, al */
                                      "\x74\x0b"			/* jz key_press */
                                      "\x56"			/* push si */
                                      "\xb4\x0e"			/* mov ah, 0eh */
                                      "\xbb\x07\x00"		/* mov bx, 0007h */
                                      "\xcd\x10"			/* int 10h */
                                      "\x5e"			/* pop si */
                                      "\xeb\xf0"			/* jmp write_msg */
                                      /* key_press: */
                                      "\x32\xe4"			/* xor ah, ah */
                                      "\xcd\x16"			/* int 16h */
                                      "\xcd\x19"			/* int 19h */
                                      "\xeb\xfe"			/* foo: jmp foo */
                                      /* message_txt: */
                                      "This is not a bootable disk.  Please insert a bootable floppy and\r\n"
                                      "press any key to try again ... \r\n";

#define MESSAGE_OFFSET 29	/* Offset of message in above code */


FileSystem::FileSystem(char *filename)
{
    this->filename = filename;
    this->io = new File();
    this->io->open(filename,READWRITE);
}

FileSystem::~FileSystem()
{
    delete io;
}

boolean FileSystem::power_of_two(int32u val)
{
    return ((val > 1) && ((val & (val - 1)) == 0));
}

const int8u media[11] = { 0x00, 0x01, 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

FSType FileSystem::DetectFatSize(FAT1216BootSectorPtr bpb)
{
    unsigned fat12 = 0, fat16 = 0, fat32 = 0;

    const unsigned root_dir_sects = (((bpb->bpb.BPB_RootEntCnt * 32) + (bpb->bpb.BPB_BytsPerSec - 1)) / bpb->bpb.BPB_BytsPerSec);
    const unsigned fat_size = (bpb->bpb.BPB_FATSz16 != 0) ? bpb->bpb.BPB_FATSz16 : ((FAT32BootSectorPtr) bpb)->BPB_FATSz32;
    const unsigned total_secs = (bpb->bpb.BPB_TotSec16 != 0) ? bpb->bpb.BPB_TotSec16 : bpb->bpb.BPB_TotSec32;
    const unsigned data_sec = total_secs - (bpb->bpb.BPB_RsvdSecCnt + (bpb->bpb.BPB_NumFATs * fat_size) + root_dir_sects);
    const unsigned clusters = (data_sec / bpb->bpb.BPB_SecPerClus);
    if (clusters < 4085)
    {
        fat12++;
    }
    else if (clusters < 65525)
    {
        fat16++;
    }
    else
    {
        fat32++;
    }
    if (memcmp((void *) bpb->BS_FilSysType, "FAT12   ", 8) == 0)
        fat12++;
    else if (memcmp((void *) bpb->BS_FilSysType, "FAT16   ", 8) == 0)
        fat16++;
    else if (memcmp(((FAT32BootSectorPtr) bpb)->BS_FilSysType, "FAT32   ", 8) == 0)
        fat32++;
    if ((fat12 > 0) && (memcmp((void *) bpb->BS_FilSysType, "FAT12   ", 8) != 0))
        fat12--;
    if ((fat16 > 0) && (memcmp((void *) bpb->BS_FilSysType, "FAT16   ", 8) != 0))
        fat16--;
    if ((fat32 > 0) && (memcmp(((FAT32BootSectorPtr) bpb)->BS_FilSysType, "FAT32   ", 8) != 0))
        fat32--;
    if (fat12 && !fat16 && !fat32)
        return FS_FAT12;
    if (!fat12 && fat16 && !fat32)
        return FS_FAT16;
    if (!fat12 && !fat16 && fat32)
        return FS_FAT32;
    return FS_UNKNOWN;
}

FSType FileSystem::DetectFat(FAT1216BootSectorPtr bpb)
{
    if (!(((bpb->bpb.BS_jmpBoot[0] == 0xEB) && (bpb->bpb.BS_jmpBoot[2] == 0x90)) ||
            ((bpb->bpb.BS_jmpBoot[0] == 0xE9) && ((*(int16u *) &bpb->bpb.BS_jmpBoot[1]) < 0x1FE))))
        return FS_UNKNOWN;
    if ((bpb->bpb.BPB_BytsPerSec < 128) || (bpb->bpb.BPB_BytsPerSec > 4096) || !power_of_two(bpb->bpb.BPB_BytsPerSec))
        return FS_UNKNOWN;
    if ((bpb->bpb.BPB_SecPerClus != 1) && (
                (bpb->bpb.BPB_SecPerClus < 1) || (bpb->bpb.BPB_SecPerClus > 128) || !power_of_two(bpb->bpb.BPB_SecPerClus))
       )
        return FS_UNKNOWN;
    if (!bpb->bpb.BPB_RsvdSecCnt)
        return FS_UNKNOWN;
    if (!bpb->bpb.BPB_NumFATs)
        return FS_UNKNOWN;
    if (((bpb->bpb.BPB_TotSec16 && bpb->bpb.BPB_TotSec32) && (bpb->bpb.BPB_TotSec16 != bpb->bpb.BPB_TotSec32)) || (!bpb->bpb.BPB_TotSec16 && !bpb->bpb.BPB_TotSec32))
        return FS_UNKNOWN;
    if (memchr(media, bpb->bpb.BPB_Media, 11) == NULL)
        return FS_UNKNOWN;
    return DetectFatSize(bpb);
}

int FileSystem::DetectMBR(MBRPtr mbr)
{
    //printf("%02X\n",mbr->partition[0].type);
    return mbr->partition[0].type==0x04 || mbr->partition[0].type==0x0B;
}

void FileSystem::parse()
{
    int8u *buffer = new int8u[65*MAX_SECT_SIZE];
    io->read(buffer,0,65*MAX_SECT_SIZE);
    boolean mbrExist = DetectMBR((MBRPtr)buffer);
    FSType result;
    if (mbrExist)
    {
        MBRPtr mbr = (MBRPtr)buffer;
        result = DetectFat((FAT1216BootSectorPtr)(buffer+mbr->partition[0].StartLBA*512));
    }
    else
    {
        result = DetectFat((FAT1216BootSectorPtr)buffer);
    }
    switch(result)
    {
    case FS_UNKNOWN:
        fs = NULL;
        if (mbrExist)
            printf("Unknown file system with MBR\n");
        else
            printf("Unknown file system with no MBR\n");
        break;
    case FS_FAT12:
        fs = new FAT12(filename);
        if (mbrExist)
            printf("FAT12 file system with MBR\n");
        else
            printf("FAT12 file system with no MBR\n");
        break;
    case FS_FAT16:
        if (mbrExist)
        {
            fs = new FAT16(filename,true);
            printf("FAT16 file system with MBR\n");
        }
        else
        {
            fs = new FAT16(filename,false);
            printf("FAT16 file system with no MBR\n");
        }
        break;
    case FS_FAT32:
        if (mbrExist)
        {
            fs = new FAT32(filename,true);
            printf("FAT32 file system with MBR\n");
        }
        else
        {
            fs = new FAT32(filename,false);
            printf("FAT32 file system with no MBR\n");
        }
        break;
    }
    if (fs!=NULL)
    {
        fs->parse();
        fs->print();
        fs->printRootDir();
    }
    delete[] buffer;
}

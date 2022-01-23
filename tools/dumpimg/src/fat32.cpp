#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "fat32.h"

#define LOBYTE(w)              ((BYTE)((DWORD_PTR)(w) & 0xFF))
#define HIBYTE(w)              ((BYTE)((DWORD_PTR)(w) >> 8))

#define LOWORD(l)              ((WORD)((DWORD_PTR)(l) & 0xFFFF))
#define HIWORD(l)              ((WORD)((DWORD_PTR)(l) >> 16))

#define MAKEWORD(low,high)     ((WORD)(((BYTE)((DWORD_PTR)(low) & 0xFF)) | ((WORD)((BYTE)((DWORD_PTR)(high) & 0xFF))) << 8))
#define MAKEDWORD(low,high)    ((LONG)(((WORD)((DWORD_PTR)(low) & 0xFFFF)) | ((DWORD)((WORD)((DWORD_PTR)(high) & 0xFFFF))) << 16))

FAT32::FAT32(char *filename, boolean knownMBR)
{
    mbr = new MBR();
    bootSector = new FAT32BootSector;
    fileSystemInfo = new FAT32FSInfo;
    io = new File();
    io->open(filename,READ);
    list=NULL;
    numberOfEntries=0;
    this->knownMBR=knownMBR;
}

FAT32::~FAT32()
{
    delete mbr;
    delete bootSector;
    delete fileSystemInfo;
    delete io;
}

DWORD FAT32::getClusterSectorOffset(DWORD cluster)
{
    int16_t BPB_BytesPerSec = bootSector->bpb.BPB_BytsPerSec;
    int16_t BPB_RsvdSecCnt  = bootSector->bpb.BPB_RsvdSecCnt;
    int8_t BPB_NumFATs      = bootSector->bpb.BPB_NumFATs;
    int32_t BPB_FATSz32     = bootSector->BPB_FATSz32;

    if (cluster == 0)
        cluster = 2;
    return (cluster - 2)  + (BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32);
}

DWORD FAT32::getClusterByteOffset(DWORD cluster)
{
    return getClusterSectorOffset(cluster)*bootSector->bpb.BPB_BytsPerSec;
}

DWORD FAT32::getNextCluster(DWORD cluster)
{
    DWORD newclust = fat[cluster];
    if (newclust >= FAT32_FAT_BAD) newclust = FAT32_FAT_LAST;
    return newclust;
}

DWORD FAT32::getRootDirSizeinClusters()
{
    DWORD root_size = 0;
    DWORD clust = bootSector->BPB_RootClus;
    do
    {
        root_size++;
        clust = getNextCluster(clust);
    }
    while(clust<FAT32_FAT_LAST);
    return root_size;
}

void FAT32::parse()
{
    if (knownMBR)
    {
        parseMBR();
    }
    else
    {
        parseNoMBR();
    }
}

void FAT32::parseMBR()
{
    io->seek(0,SEEK_SET);
    io->read(mbr,sizeof(MBR));

    partitionStart = mbr->partition[0].StartLBA*512;
    partitionSize =  mbr->partition[0].EndLBA*512;

    io->seek(partitionStart,SEEK_SET);
    io->read(bootSector,sizeof(FAT32BootSector));
    io->read(fileSystemInfo,sizeof(FAT32FSInfo));

    FATSectorStart = bootSector->bpb.BPB_RsvdSecCnt;
    FATSectorSize = bootSector->BPB_FATSz32 * bootSector->bpb.BPB_NumFATs;

    RootDirSectorStart = FATSectorStart + FATSectorSize;
    RootDirSectorsSize = (32 * bootSector->bpb.BPB_RootEntCnt + bootSector->bpb.BPB_BytsPerSec - 1) / bootSector->bpb.BPB_BytsPerSec;

    DataSectorStart = RootDirSectorStart + RootDirSectorsSize;
    DataSectorSize = bootSector->bpb.BPB_TotSec32 - DataSectorStart;

    DWORD fatSizeInBytes = bootSector->BPB_FATSz32 * bootSector->bpb.BPB_BytsPerSec;
    fat = new DWORD[fatSizeInBytes/sizeof(DWORD)];

    io->seek(partitionStart + FATSectorStart * bootSector->bpb.BPB_BytsPerSec,SEEK_SET);
    io->read(fat,fatSizeInBytes);

    root_size = getRootDirSizeinClusters();

    DWORD numberOfRootEntries = root_size*bootSector->bpb.BPB_SecPerClus*bootSector->bpb.BPB_BytsPerSec/sizeof(DirEntryFat);

    rootDir = new DirEntryFat[numberOfRootEntries];
    memset(rootDir,0,sizeof(DirEntryFat)* numberOfRootEntries);

    DWORD rootStart = partitionStart + RootDirSectorStart*bootSector->bpb.BPB_BytsPerSec;

    int i=0;
    while(true)
    {

        io->seek(rootStart + i*32,SEEK_SET);
        io->read(&rootDir[i], sizeof(rootDir[i]));
        if (rootDir[i].statusName.allocationStatus==0)
        {
            break;
        }
        i++;
        if (i==numberOfRootEntries)
        {
            break;
        }
    }
    numberOfEntries=i;
}

void FAT32::parseNoMBR()
{
    io->seek(0,SEEK_SET);
    io->read(bootSector,sizeof(FAT32BootSector));
    io->read(fileSystemInfo,sizeof(FAT32FSInfo));

    partitionStart = 0;
    partitionSize = 0;

    FATSectorStart = bootSector->bpb.BPB_RsvdSecCnt;
    FATSectorSize = bootSector->BPB_FATSz32 * bootSector->bpb.BPB_NumFATs;

    RootDirSectorStart = FATSectorStart + FATSectorSize;
    RootDirSectorsSize = (32 * bootSector->bpb.BPB_RootEntCnt + bootSector->bpb.BPB_BytsPerSec - 1) / bootSector->bpb.BPB_BytsPerSec;

    DataSectorStart = RootDirSectorStart + RootDirSectorsSize;
    DataSectorSize = bootSector->bpb.BPB_TotSec32 - DataSectorStart;

    DWORD fatSizeInBytes = bootSector->BPB_FATSz32 * bootSector->bpb.BPB_BytsPerSec;
    fat = new DWORD[fatSizeInBytes/sizeof(DWORD)];

    io->seek(partitionStart + FATSectorStart * bootSector->bpb.BPB_BytsPerSec,SEEK_SET);
    io->read(fat,fatSizeInBytes);

    root_size = getRootDirSizeinClusters();

    DWORD numberOfRootEntries = root_size*bootSector->bpb.BPB_SecPerClus*bootSector->bpb.BPB_BytsPerSec/sizeof(DirEntryFat);

    rootDir = new DirEntryFat[numberOfRootEntries];
    memset(rootDir,0,sizeof(DirEntryFat)* numberOfRootEntries);

    DWORD rootStart = partitionStart + RootDirSectorStart*bootSector->bpb.BPB_BytsPerSec;

    int i=0;
    while(true)
    {

        io->seek(rootStart + i*32,SEEK_SET);
        io->read(&rootDir[i], sizeof(rootDir[i]));
        if (rootDir[i].statusName.allocationStatus==0)
        {
            break;
        }
        i++;
        if (i==numberOfRootEntries)
        {
            break;
        }
    }
    numberOfEntries=i;
}

void FAT32::print()
{
    if (mbr!=NULL)
    {
        printf("Disk Signature:             0x%08X\n",mbr->DiskSig);
        printf("Reserved:                   %u\n",mbr->Reserved);
        printf("Boot Signature:             0x%04X\n",mbr->BootSignature);
        for(int i=0; i<=3; i++)
        {
            if (mbr->partition[i].status!=0 && mbr->partition[i].type!=0)
            {
                printf("partition %d Status:         0x%02X\n",i,mbr->partition[i].status);               //0x80 for bootable, 0x00 for not bootable, anything else for invalid
                printf("partition %d Start Cylinder: %d\n",i,mbr->partition[i].StartAddrCylSec & 0x3FF);  //(AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
                printf("partition %d Start Head:     %d\n",i,mbr->partition[i].StartAddrHead);            //head address of start of partition
                printf("partition %d Start Sector:   %d\n",i,mbr->partition[i].StartAddrCylSec & 0x3F);   //(AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
                printf("partition %d Type:           0x%02X\n",i,mbr->partition[i].type);
                printf("partition %d End Cylinder:   %d\n",i,mbr->partition[i].EndAddrCylSec & 0x3FF);    //(AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
                printf("partition %d End Head:       %d\n",i,mbr->partition[i].EndAddrHead);              //head address of start of partition
                printf("partition %d End Sector:     %d\n",i,mbr->partition[i].EndAddrCylSec & 0x3F);     //(AddrCylSec & 0x3F) for sector,  (AddrCylSec & 0x3FF) for cylendar
                printf("partition %d Start LBA:      %ld\n",i,mbr->partition[i].StartLBA);                //linear address of first sector in partition. Multiply by sector size (usually 512) for real offset
                printf("partition %d End LBA:        %ld\n",i,mbr->partition[i].EndLBA);                  //linear address of last sector in partition. Multiply by sector size (usually 512) for real offset
                printf("partition %d Start Offset:   %ld\n",i,mbr->partition[i].StartLBA*512);            //linear address of first sector in partition. Multiply by sector size (usually 512) for real offset
                printf("partition %d End Offset:     %ld\n",i,mbr->partition[i].EndLBA*512);              //linear address of last sector in partition. Multiply by sector size (usually 512) for real offset
                printf("\n");
            }
        }
    }
    printf("OEM Name:                    '%.*s'\n",8, bootSector->bpb.BS_OEMName);
    printf("Bytes Per Sector:            %d\n",bootSector->bpb.BPB_BytsPerSec);
    printf("Sectors Per Cluster:         %d\n",bootSector->bpb.BPB_SecPerClus);
    printf("Bytes Per Cluster:           %d\n",bootSector->bpb.BPB_SecPerClus*bootSector->bpb.BPB_BytsPerSec);
    printf("Reserved Sectors:            %d\n",bootSector->bpb.BPB_RsvdSecCnt);
    printf("Number of FATS               %d\n",bootSector->bpb.BPB_NumFATs);
    printf("Number Of Root Entries(N/A): %d\n",bootSector->bpb.BPB_RootEntCnt);
    printf("Sectors (N/A):               %d\n",bootSector->bpb.BPB_TotSec16);
    printf("Descriptor:                  0x%02X\n",bootSector->bpb.BPB_Media);
    printf("Sectors Per FAT (N/A):       %d\n",bootSector->bpb.BPB_FATSz16);
    printf("Sectors Per track:           %d\n",bootSector->bpb.BPB_SecPerTrk);
    printf("Heads                        %d\n",bootSector->bpb.BPB_NumHeads);
    printf("Hidden Sectors               %ld\n",bootSector->bpb.BPB_HiddSec);
    printf("Extended Sectors             %lu\n",bootSector->bpb.BPB_TotSec32);
    printf("Size of FAT                  %ld\n",bootSector->BPB_FATSz32);
    printf("Ext Flags                    0x%04X\n",bootSector->BPB_ExtFlags);
    printf("FS Version                   %d\n",bootSector->BPB_FSVer);
    printf("RootCluster                  %ld\n",bootSector->BPB_RootClus);
    printf("FSInfo Sector                %d\n",bootSector->BPB_FSInfo);
    printf("Backup Boot Sector           %d\n",bootSector->BPB_BkBootSec);
    printf("Drive Number                 %d\n",bootSector->BS_DrvNum);
    printf("Signature                    0x%02X\n",bootSector->BS_BootSig);
    printf("Serial Number                0x%08X\n",bootSector->BS_VolID);
    printf("Label                        '%.*s'\n",11, bootSector->BS_VolLab);
    printf("System ID                    '%.*s'\n",8,bootSector->BS_FilSysType);
    printf("Boot Signature               0x%04X\n\n",bootSector->bootSignature);
    printf("Signature 1                  0x%08X\n",fileSystemInfo->FSI_LeadSig);
    printf("Signature 2                  0x%08X\n",fileSystemInfo->FSI_StrucSig);
    printf("Free Cluster Count           %ld\n",fileSystemInfo->FSI_Free_Count);
    printf("Free Space                   %ld\n",fileSystemInfo->FSI_Free_Count*bootSector->bpb.BPB_SecPerClus*bootSector->bpb.BPB_BytsPerSec);
    printf("Next Free Cluster Available  %ld\n",fileSystemInfo->FSI_Nxt_Free);
    printf("Boot Signature               0x%08X\n\n",fileSystemInfo->FSI_TrailSig);

    printf("Root Size in bytes           %ld\n\n",root_size*bootSector->bpb.BPB_SecPerClus*bootSector->bpb.BPB_BytsPerSec);

    printf("Sector Type          Start\t Size\n");
    printf("======================================================\n");
    printf("Partition Start   :  %8d\t%8d\n", partitionStart,partitionSize);
    printf("Boot Sector       :  %8d\t%8d\n", 0,1);
    printf("Reserved Sectors  :  %8d\t%8d\n", 1,bootSector->bpb.BPB_RsvdSecCnt);
    printf("FAT Sector        :  %8ld\t%8ld\n", FATSectorStart,FATSectorSize);
    printf("Root Dir Sector   :  %8ld\t%8ld\n", RootDirSectorStart,RootDirSectorsSize);
    printf("Data Sector       :  %8ld\t%8ld\n\n", DataSectorStart,DataSectorSize);
}

char *FAT32::extractLongFileName(char *name,int size)
{
    char *buffer=new char[50];
    memset(buffer,0,50);
    int n=0;
    for(int i=0; i<size; i++)
    {
        if (name[i]!='\0')
        {
            buffer[n++]=name[i];
        }
    }
    return buffer;
}

char *FAT32::extractShortFileName(char *name,int size)
{
    char *buffer=new char[50];
    memset(buffer,0,50);
    int n=0;
    for(int i=0; i<size; i++)
    {
        if (name[i]!=' ' && name[i]!='\0')
        {
            buffer[n++]=name[i];
        }
    }
    return buffer;
}

char *FAT32::getAttrString(DirEntryFatPtr entry)
{
    char *attrs = new char[7];
    memset(attrs,0,7);
    attrs[0]='-';
    attrs[1]='-';
    attrs[2]='-';
    attrs[3]='-';
    attrs[4]='-';
    attrs[5]='-';
    if (((entry->attrb&0x01)>>0)==1) attrs[0]='R';
    if (((entry->attrb&0x02)>>1)==1) attrs[1]='H';
    if (((entry->attrb&0x04)>>2)==1) attrs[2]='S';
    if (((entry->attrb&0x08)>>3)==1) attrs[3]='L';
    if (((entry->attrb&0x10)>>4)==1) attrs[4]='D';
    if (((entry->attrb&0x20)>>5)==1) attrs[5]='A';
    return attrs;
}

char *FAT32::getModifiedTime(char *buffer, int size, DirEntryFatPtr entry)
{
    int t = entry->time;
    sprintf(buffer,"%02d:%02d:%02d", t >> 11, (t & 0x07E0) >> 5, (t & 0x001F) >> 0);
    return buffer;
}

char *FAT32::getModifiedDate(char *buffer, int size, DirEntryFatPtr entry)
{
    int d = entry->date;
    sprintf(buffer,"%02d/%02d/%02d", (d & 0x01E0) >> 5, (d & 0x001F) >> 0,(d >> 9) + 1980);
    return buffer;
}


void FAT32::dumpDirEntry(DirEntryFatPtr entry)
{
    if (entry->attrb!=0x0F)
    {
        printf("Name                         '%.*s'\n",8, extractShortFileName((char *)entry->statusName.name,8));
        printf("Allocation Status            0x%02X\n",entry->statusName.allocationStatus);
        printf("Ext                          '%.*s'\n",3, entry->ext);
        printf("Attribute                    0x%02X\n",entry->attrb);
        printf("Creation Time                0x%04X\n",entry->type.fat32.crt_time);
        printf("Creation Date                0x%04X\n",entry->type.fat32.crt_date);
        printf("Access Date                  0x%04X\n",entry->type.fat32.crt_last);
        printf("Starting Cluster HI          0x%04X\n",entry->type.fat32.strtclst32);
        printf("Modified Time                0x%04X\n",entry->time);
        printf("Modified Date                0x%04X\n",entry->date);
        printf("Starting Cluster LO          0x%04X\n",entry->strtclst);
        printf("File Size                    %d\n",entry->filesize);
    }
    else
    {
        LongDirEntryFatPtr entry2 = (LongDirEntryFatPtr)entry;
        printf("Ordinal                      0x%02X\n",entry2->sequ_flags);
        printf("Name 1                       '%.*s'\n",10, extractLongFileName((char *)entry2->name0,10));
        printf("Attribute                    0x%02X\n",entry2->attrb);
        printf("Type                         0x%02X\n",entry2->resv);
        printf("Checksum                     0x%02X\n",entry2->sfn_crc);
        printf("Name 2                       '%.*s'\n",12, extractLongFileName((char *)entry2->name1,12));
        printf("Fat Cluster Lo               %d\n",entry2->clust_zero);
        printf("Name 3                       '%.*s'\n",4, extractLongFileName((char *)entry2->name2,4));
    }
    printf("\n");
}

void FAT32::printRootDir()
{
    printf("\nDate       Time     Attr    Size     Cluster SFN         LFN\n");
    for(int i=0; i<numberOfEntries; i++)
    {
        if (rootDir[i].attrb==ATTR_LONG_FILE_NAME)
        {
            LongDirEntryFatPtr lfn = (LongDirEntryFatPtr)&rootDir[i];
            char *name0 = extractLongFileName((char *)lfn->name0,10);
            char *name1 = extractLongFileName((char *)lfn->name1,12);
            char *name2 = extractLongFileName((char *)lfn->name2,4);
            char longname[30];
            memset(longname,0,sizeof(longname));
            sprintf(longname,"%s%s%s",name0,name1,name2);
            if (lfn->sequ_flags>0x40)
            {
                numberOfLFNEntries = lfn->sequ_flags-0x40;
                list = new std::string[numberOfLFNEntries];
                list[numberOfLFNEntries-1]=longname;
            }
            else
            {
                list[lfn->sequ_flags-1]=longname;
            }
        }
        else
        {
            char shortname[12];
            char buffer1[12];
            char buffer2[12];

            memset(shortname,0,sizeof(shortname));
            memset(buffer1,0,sizeof(buffer1));
            memset(buffer2,0,sizeof(buffer2));

            char *name = extractShortFileName((char *)rootDir[i].statusName.name,8);
            char *ext = extractShortFileName((char *)rootDir[i].ext,3);
            char *attr = getAttrString(&rootDir[i]);
            char *mtime = getModifiedTime((char *)buffer1,sizeof(buffer1),&rootDir[i]);
            char *mdate = getModifiedDate((char *)buffer2,sizeof(buffer2),&rootDir[i]);
            sprintf(shortname,"%s.%s",name,ext);
            std::string lname;
            if (list!=NULL)
            {
                lname=list[0];
                for (int i=1; i<numberOfLFNEntries; i++)
                {
                    lname.append(list[i]);
                }
            }
            printf("%.*s %.*s %.*s % -8d % -8d % -11s %.*s\n", 10, mdate, 8, mtime, 6, attr, rootDir[i].filesize, rootDir[i].strtclst, shortname,256,lname.c_str());
            if (list!=NULL)
            {
                delete[] list;
                list = NULL;
                numberOfLFNEntries=0;
            }
        }
    }
}


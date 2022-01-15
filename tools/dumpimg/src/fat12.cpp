#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "fat12.h"

FAT12::FAT12(char *filename)
{
    bootSector = new FAT1216BootSector;
    io = new File();
    io->open(filename,READ);
    list = NULL;
    numberOfEntries=0;
}

FAT12::~FAT12()
{
    delete bootSector;
    delete io;
    delete[] fat;
}

DWORD FAT12::getNextCluster(DWORD cluster)
{
    unsigned short offset = cluster / 2 * 3;
    unsigned short nextClus;
    if (cluster % 2 == 0)
        nextClus = ((fat[offset + 1] & 0x0f) << 8) | fat[offset];
    else
        nextClus = (fat[offset + 2] << 4) | ((fat[offset + 1] >> 4) & 0x0f);
    return nextClus;

}

DWORD FAT12::getClusterSectorOffset(DWORD cluster)
{
    unsigned int ret = DataSectorStart + ((cluster - 2) * bootSector->bpb.BPB_SecPerClus);
    return ret;
}

void FAT12::parse()
{
    io->seek(0,SEEK_SET);
    io->read(bootSector,sizeof(FAT1216BootSector));

    clusterSize = bootSector->bpb.BPB_SecPerClus*bootSector->bpb.BPB_BytsPerSec;

    DWORD FATSz = bootSector->bpb.BPB_FATSz16;
    DWORD RootDirSectors = ((bootSector->bpb.BPB_RootEntCnt*32)+(bootSector->bpb.BPB_BytsPerSec-1))/bootSector->bpb.BPB_BytsPerSec;

    FATSectorStart  = bootSector->bpb.BPB_RsvdSecCnt;
    RootSectorStart = FATSectorStart + bootSector->bpb.BPB_NumFATs * FATSz;

    DataSectorStart = RootSectorStart + RootDirSectors;

    DWORD fatSize = FATSz * bootSector->bpb.BPB_BytsPerSec;
    fat = new BYTE[fatSize];
    io->seek(FATSectorStart * bootSector->bpb.BPB_BytsPerSec,SEEK_SET);
    io->read(fat, fatSize);

    rootDir = new DirEntryFat[bootSector->bpb.BPB_RootEntCnt];
    memset(rootDir,0,sizeof(DirEntryFat)* bootSector->bpb.BPB_RootEntCnt);

    int i=0;
    while(true)
    {
        io->seek((RootSectorStart * bootSector->bpb.BPB_BytsPerSec) + i*32,SEEK_SET);
        io->read(&rootDir[i], sizeof(rootDir[i]));
        if (rootDir[i].statusName.allocationStatus==0)
        {
            break;
        }
        i++;
    }
}

void FAT12::print()
{
    printf("OEM Name:               '%.*s'\n",8, bootSector->bpb.BS_OEMName);
    printf("Bytes Per Sector:       %d\n",bootSector->bpb.BPB_BytsPerSec);
    printf("Sectors Per Cluster:    %d\n",bootSector->bpb.BPB_SecPerClus);
    printf("Bytes Per Cluster:      %d\n",clusterSize);
    printf("Reserved Sectors:       %d\n",bootSector->bpb.BPB_RsvdSecCnt);
    printf("Number of FATS          %d\n",bootSector->bpb.BPB_NumFATs);
    printf("Number Of Root Entries: %d\n",bootSector->bpb.BPB_RootEntCnt);
    printf("Sectors:                %d\n",bootSector->bpb.BPB_TotSec16);
    printf("Descriptor:             0x%02X\n",bootSector->bpb.BPB_Media);
    printf("Sectors Per FAT:        %d\n",bootSector->bpb.BPB_FATSz16);
    printf("Sectors Per track:      %d\n",bootSector->bpb.BPB_SecPerTrk);
    printf("Heads                   %d\n",bootSector->bpb.BPB_NumHeads);
    printf("Hidden Sectors          %ld\n",bootSector->bpb.BPB_HiddSec);
    printf("Extended Sectors        %lu\n",bootSector->bpb.BPB_TotSec32);
    printf("drive Number            %d\n",bootSector->BS_DrvNum);
    printf("Reserved                %d\n",bootSector->BS_Reserved1);
    printf("Signature               0x%02X\n",bootSector->BS_BootSig);
    printf("Serial Number           0x%08X\n",bootSector->BS_VolID);
    printf("Label                   '%.*s'\n",11, bootSector->BS_VolLab);
    printf("System ID               '%.*s'\n",8,bootSector->BS_FilSysType);
    printf("Boot Signature          0x%04X\n\n",bootSector->bootSignature);
    printf("BS: %d FAT addr: %d Root addr: %d Data addr: %d\n", 0, FATSectorStart, RootSectorStart, DataSectorStart);
    printf("FAT entry %d ==> FAT entry %d\n",3,this->getNextCluster(3));
    DWORD offset = getClusterSectorOffset(3)*bootSector->bpb.BPB_BytsPerSec;
    BYTE buffer[50];
    memset(buffer,0,sizeof(buffer));
    io->seek(offset,SEEK_SET);
    io->read(buffer,sizeof(buffer)-1);
    printf("Buffer:               '%.*s'\n",49, buffer);
}

char *FAT12::extractLongFileName(char *name,int size)
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

char *FAT12::extractShortFileName(char *name,int size)
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

char *FAT12::getAttrString(char *attrs, int size, DirEntryFatPtr entry)
{
    memset(attrs,0,size);
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
}

char *FAT12::getModifiedTime(char *buffer, int size, DirEntryFatPtr entry)
{
    int t = entry->time;
    sprintf(buffer,"%02d:%02d:%02d", t >> 11, (t & 0x07E0) >> 5, (t & 0x001F) >> 0);
    return buffer;
}

char *FAT12::getModifiedDate(char *buffer, int size, DirEntryFatPtr entry)
{
    int d = entry->date;
    sprintf(buffer,"%02d/%02d/%02d", (d & 0x01E0) >> 5, (d & 0x001F) >> 0,(d >> 9) + 1980);
    return buffer;
}

void FAT12::printRootDir()
{
    printf("Date       Time     Attr   Size      Cluster SFN         LFN\n");
    for(int i=0; i<bootSector->bpb.BPB_RootEntCnt; i++)
    {
        if (rootDir[i].statusName.allocationStatus==0)
        {
            break;
        }
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
                numberOfEntries = lfn->sequ_flags-0x40;
                list=new std::string[numberOfEntries];
                list[numberOfEntries-1]=longname;
            }
            else
            {
                list[lfn->sequ_flags-1]=longname;
            }
        }
        else
        {
            char shortname[12];
            char attrs[7];
            char buffer1[10];
            char buffer2[10];

            memset(shortname,0,sizeof(shortname));

            char *name = extractShortFileName((char *)rootDir[i].statusName.name,8);
            char *ext = extractShortFileName((char *)rootDir[i].ext,3);
            char *attr = getAttrString((char *)attrs, sizeof(attrs), &rootDir[i]);
            char *mtime = getModifiedTime((char *)buffer1,sizeof(buffer1),&rootDir[i]);
            char *mdate = getModifiedDate((char *)buffer2,sizeof(buffer2),&rootDir[i]);

            sprintf(shortname,"%s.%s",name,ext);
            std::string lname;
            if (list!=NULL)
            {
                lname=list[0];
                for (int i=1; i<numberOfEntries; i++)
                {
                    lname.append(list[i]);
                }
            }
            printf("%s %s %s % 8d % 8d % -11s %.*s\n", mdate, mtime, attrs,rootDir[i].filesize, rootDir[i].strtclst, shortname,256,lname.c_str());
            if (list!=NULL)
            {
                delete[] list;
                list = NULL;
                numberOfEntries=0;
            }
        }
    }
}

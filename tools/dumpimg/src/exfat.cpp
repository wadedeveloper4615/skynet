#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "exfat.h"

ExFat::ExFat(char *filename)
{
    bootSector = new EXFATBootSector;
    io = new File();
    io->open(filename,READ);
}

ExFat::~ExFat()
{
    delete bootSector;
    delete io;
}


DWORD ExFat::getClusterSectorOffset(DWORD cluster)
{
    DWORD ret = ((cluster-2) * sectorPerCluster) + bootSector->ClusterHeapOffset;
    return ret;
}

DWORD ExFat::getNextCluster(DWORD cluster)
{
    DWORD newclust = fat[cluster];
    if (newclust >= EXFAT_FAT_BAD) newclust = EXFAT_FAT_LAST;
    return newclust;
}

DWORD ExFat::getRootDirSizeinClusters()
{
    DWORD root_size = 0;  // in clusters
    DWORD clust = bootSector->FirstClusterOfRootDirectory;
    do
    {
        clust = getNextCluster(clust);
        root_size++;
    }
    while (clust < EXFAT_FAT_BAD);
}

void ExFat::parse()
{
    io->seek(0,SEEK_SET);
    io->read(bootSector,sizeof(EXFATBootSector));

    fatAlignmentStart = 24;
    fatAlignmentSize = bootSector->FATOffset-24;
    fatStart = bootSector->FATOffset;
    fatSize = bootSector->FATLength;

    bytesPerSector = (1 << bootSector->sector_bits);
    sectorPerCluster = (1 << bootSector->spc_bits);

    DWORD size = fatSize * bytesPerSector;
    DWORD offset = fatStart * bytesPerSector;
    fat = new DWORD[size/4];
    io->seek(offset,SEEK_SET);
    io->read(fat,size);

    DWORD clust = bootSector->FirstClusterOfRootDirectory;
    root_size = 0;
    do
    {
        clust = getNextCluster(clust);
        root_size++;
    }
    while (clust < EXFAT_FAT_BAD);

    DWORD clusterSizeInBytes = sectorPerCluster * bytesPerSector;
    BYTE *buffer = new BYTE[root_size * clusterSizeInBytes];
    BYTE *ptr = (BYTE *) buffer;
    clust = bootSector->FirstClusterOfRootDirectory;
    do
    {
        io->read(ptr, getClusterSectorOffset(clust)*bytesPerSector, clusterSizeInBytes);
        ptr += clusterSizeInBytes;
        clust = getNextCluster(clust);
    }
    while (clust < EXFAT_FAT_BAD);

    root = (EXFATRootPtr)(buffer);
    rootSizeInBytes = root_size * clusterSizeInBytes;
}

char *ExFat::extractShortFileName(char *name,int size)
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

void ExFat::print()
{
    printf("OEM Name:               '%.*s'\n",8, bootSector->fileSystemName);
    printf("Partition Offset        %lld\n",bootSector->PartitionOffset);
    printf("Volume Length           %lld\n",bootSector->VolumeLength);
    printf("FAT Offset              %ld\n",bootSector->FATOffset);
    printf("FAT Length              %ld\n",bootSector->FATLength);
    printf("Cluster Heap Offset     %ld\n",bootSector->ClusterHeapOffset);
    printf("Cluster Count           %ld\n",bootSector->ClusterCount);
    printf("Root dir cluster        %ld\n",bootSector->FirstClusterOfRootDirectory);
    printf("Volume Serial Number    0x%08LX\n",bootSector->volume_serial);
    printf("File System Version     %d.%d\n",bootSector->version.major,bootSector->version.minor);
    printf("Volume State Flags      0x%04LX\n",bootSector->volume_state);
    printf("Sector Bits             %d\n",bootSector->sector_bits);
    printf("Sector Per Cluster bits %d\n",bootSector->spc_bits);
    printf("FAT Count               %d\n",bootSector->fat_count);
    printf("Drive Number            %d\n",bootSector->drive_no);
    printf("Allocated Percent       %d\n",bootSector->allocated_percent);
    printf("Boot Signature          0x%04X\n",bootSector->boot_signature);
    printf("FAT Alignment Start     %d\n",fatAlignmentStart);
    printf("FAT Alignment Size      %d\n",fatAlignmentSize);
    printf("FAT Start               %d\n",fatStart);
    printf("FAT Size                %d\n",fatSize);
    printf("root dir is %ld cluster(s) long\n",root_size);
}

void ExFat::printRootDir()
{
    for(int i=0; i< rootSizeInBytes/sizeof(EXFATRoot); i++)
    {
        if (root[i].entry_type==EXFAT_DIR_EOD)
        {
            break;
        }
        printf("Type: %02X\n",root[i].entry_type);
        if (root[i].entry_type==EXFAT_DIR_LABEL)
        {
            printf("Label:               '%s'\n",extractShortFileName((char *)root[i].type.label.name,22));
        }
    }
}

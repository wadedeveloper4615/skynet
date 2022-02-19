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

typedef struct _ProgramArguments
{
    bool create;
    int size_fat;	                     /* Size in bits of FAT entries */
    int size_fat_by_user;	             /* 1 if FAT size user selected */
    unsigned sector_size;	             /* Size of a logical sector */
    int sector_size_set;	             /* User selected sector size */
    int sectors_per_cluster;	         /* Number of sectors per disk cluster */
    char * device_name;
    int blocks_specified;
    unsigned long long blocks;           /* Number of blocks in filesystem */
} ProgramArguments, *ProgramArgumentsPtr;

#endif // MKFS_H_INCLUDED

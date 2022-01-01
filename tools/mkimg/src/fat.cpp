#include "fat.h"

/*
*This is the table for FAT16 drives. NOTE that this table includes
* entries for disk sizes larger than 512 MB even though typically
* only the entries for disks < 512 MB in size are used.
* The way this table is accessed is to look for the first entry
* in the table for which the disk size is less than or equal
* to the DiskSize field in that table entry. For this table to
* work properly BPB_RsvdSecCnt must be 1, BPB_NumFATs
* must be 2, and BPB_RootEntCnt must be 512. Any of these values
* being different may require the first table entries DiskSize value
* to be changed otherwise the cluster count may be to low for FAT16.
*/
DSKSZTOSECPERCLUS DskTableFAT16 [] =
{
    {8400,        0}, /* disks up to 4.1 MB, the 0 value for SecPerClusVal trips an error */
    {32680,       2}, /* disks up to 16 MB, 1k cluster */
    {262144,      4}, /* disks up to 128 MB, 2k cluster */
    {524288,      8}, /* disks up to 256 MB, 4k cluster */
    {1048576,    16}, /* disks up to 512 MB, 8k cluster */
    /* The entries after this point are not used unless FAT16 is forced */
    {2097152,    32}, /* disks up to 1 GB, 16k cluster */
    {4194304,    64}, /* disks up to 2 GB, 32k cluster */
    {0xFFFFFFFF,  0} /*any disk greater than 2GB, 0 value for SecPerClusVal trips an error */
};

/*
* This is the table for FAT32 drives. NOTE that this table includes
* entries for disk sizes smaller than 512 MB even though typically
* only the entries for disks >= 512 MB in size are used.
* The way this table is accessed is to look for the first entry
* in the table for which the disk size is less than or equal
* to the DiskSize field in that table entry. For this table to
* work properly BPB_RsvdSecCnt must be 32, and BPB_NumFATs
* must be 2. Any of these values being different may require the first
* table entries DiskSize value to be changed otherwise the cluster count
* may be to low for FAT32.
*/
DSKSZTOSECPERCLUS DskTableFAT32 [] = {
 {66600,       0}, /* disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error */
 {532480,      1}, /* disks up to 260 MB, .5k cluster */
 {16777216,    8}, /* disks up to 8 GB, 4k cluster */
 {33554432,   16}, /* disks up to 16 GB, 8k cluster */
 {67108864,   32}, /* disks up to 32 GB, 16k cluster */
 {0xFFFFFFFF, 64}/* disks greater than 32GB, 32k cluster */
 };

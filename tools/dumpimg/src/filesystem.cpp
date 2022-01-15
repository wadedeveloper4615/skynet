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

BOOL FileSystem::power_of_two(DWORD val)
{
    return ((val > 1) && ((val & (val - 1)) == 0));
}

const BYTE media[11] = { 0x00, 0x01, 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

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
            ((bpb->bpb.BS_jmpBoot[0] == 0xE9) && ((*(WORD *) &bpb->bpb.BS_jmpBoot[1]) < 0x1FE))))
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

FSType FileSystem::DetectExFat(EXFATBootSectorPtr vbr)
{
    if ((vbr->jmp[0] != 0xEB) || (vbr->jmp[1] != 0x76) || (vbr->jmp[2] != 0x90))
        return FS_UNKNOWN;
    if (!(vbr->version.major == 0x01 && vbr->version.minor == 0x00))
        return FS_UNKNOWN;
    if ((vbr->sector_bits > 12) || ((vbr->sector_bits + vbr->spc_bits) > 25))
        return FS_UNKNOWN;
    if ((vbr->fat_count < 1) || (vbr->fat_count > 2))
        return FS_UNKNOWN;
    if (vbr->allocated_percent > 100)
        return FS_UNKNOWN;
    if (vbr->volume_state & (1<<3))
        return FS_UNKNOWN;
    return FS_EXFAT;
}

void FileSystem::parse()
{
    BYTE *buffer = new BYTE[65*MAX_SECT_SIZE];
    io->read(buffer,0,65*MAX_SECT_SIZE);
    boolean mbrExist = DetectMBR((MBRPtr)buffer);
    FSType result;
    if (mbrExist)
    {
        MBRPtr mbr = (MBRPtr)buffer;
        result = DetectFat((FAT1216BootSectorPtr)(buffer+mbr->partition[0].StartLBA*512));
        if (result==FS_UNKNOWN)
        {
            result = DetectExFat((EXFATBootSectorPtr)(buffer+mbr->partition[0].StartLBA*512));
        }
    }
    else
    {
        result = DetectFat((FAT1216BootSectorPtr)buffer);
        if (result==FS_UNKNOWN)
        {
            result = DetectExFat((EXFATBootSectorPtr)buffer);
        }
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
    case FS_EXFAT:
        fs = new ExFat(filename);
        if (mbrExist)
            printf("ExFat file system with MBR\n");
        else
            printf("EXFat file system with no MBR\n");
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

off_t FileSystem::fsize()
{
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    fprintf(stderr, "Cannot determine size of %s: %s\n",
            filename, strerror(errno));

    return -1;
}

void FileSystem::createImage(DWORD blockSize, DWORD numberOfBlocks)
{
    io = new File();
    io->open(filename,WRITE);
    io->seek(0,SEEK_SET);
    BYTE *buffer = new BYTE[blockSize];
    memset(buffer,0,blockSize);
    for(int i=0; i<numberOfBlocks; i++)
    {
        io->write(buffer,blockSize);
    }
    delete[] buffer;
    io->close();

    DWORD fileSize = fsize();
}

long FileSystem::blkdev_valid_offset (int handle,off_t offset)
{
    char ch;

    if (lseek (handle, offset, 0) < 0)
        return 0;
    if (_read (handle, &ch, 1) < 1)
        return 0;
    return 1;
}

int FileSystem::is_blkdev(int handle)
{
    struct stat st;
    return (fstat(handle, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t FileSystem::blkdev_find_size (int handle)
{
    uintmax_t high, low = 0;

    for (high = 1024; blkdev_valid_offset (handle,high); )
    {
        if (high == UINTMAX_MAX)
            return -1;

        low = high;

        if (high >= UINTMAX_MAX/2)
            high = UINTMAX_MAX;
        else
            high *= 2;
    }

    while (low < high - 1)
    {
        uintmax_t mid = (low + high) / 2;

        if (blkdev_valid_offset (handle,mid))
            low = mid;
        else
            high = mid;
    }
    blkdev_valid_offset (handle,0);
    return (low + 1);
}

int FileSystem::blkdev_get_size(int handle,unsigned long long *bytes)
{
    struct stat st;

    if (fstat(handle, &st) == 0 && S_ISREG(st.st_mode))
    {
        *bytes = st.st_size;
        return 0;
    }
    if (!S_ISBLK(st.st_mode))
        return -1;

    *bytes = blkdev_find_size(handle);
    return 0;
}

void FileSystem::get_block_device_size(int handle,DeviceInfoPtr info)
{
    unsigned long long bytes;

    if (!blkdev_get_size(handle, &bytes) && bytes != 0)
        info->size = bytes;
}

int FileSystem::blkdev_get_geometry(unsigned int *h, unsigned int *s)
{
    *h = 0;
    *s = 0;
    return -1;
}

int FileSystem::blkdev_get_start(dev_t rdev, unsigned long long *s)
{
    *s = 0;
    return -1;
}

int FileSystem::blkdev_get_sector_size(int *sector_size)
{
    *sector_size = DEFAULT_SECTOR_SIZE;
    return 0;
}

void FileSystem::get_block_geometry(DeviceInfoPtr info, dev_t rdev)
{
    unsigned int heads, sectors;
    unsigned long long start;

    if (!blkdev_get_geometry(&heads, &sectors)
            && heads && sectors)
    {
        info->geom_heads = heads;
        info->geom_sectors = sectors;
    }

    if (!blkdev_get_start(rdev, &start))
        info->geom_start = start;
}


void FileSystem::get_sector_size(DeviceInfoPtr info)
{
    int size;

    if (!blkdev_get_sector_size(&size))
        info->sector_size = size;
}

__int64 FileSystem::info(int handle)
{
    memset(&devInfo,0,sizeof(devInfo));

    struct stat stat;
    int ret = fstat(handle, &stat);
    if (ret < 0)
    {
        perror("fstat on target failed");
        return -1;
    }
    if (S_ISREG(stat.st_mode))
    {
        /* there is nothing more to discover for an image file */
        devInfo.type = TYPE_FILE;
        devInfo.partition = 0;
        devInfo.size = stat.st_size;
        devInfo.sector_size = DEFAULT_SECTOR_SIZE;
        return 0;
    }

    if (!S_ISBLK(stat.st_mode))
    {
        /* neither regular file nor block device? not usable */
        devInfo.type = TYPE_BAD;
        return 0;
    }

    get_block_device_size(handle,&devInfo);
    get_block_geometry(&devInfo,stat.st_rdev);
    get_sector_size(&devInfo);
    return 0;
}

void FileSystem::establish_params()
{
    unsigned int sector_size=512;
    unsigned int sec_per_track;
    unsigned int heads;
    unsigned int media = 0xf8;
    unsigned int cluster_size = 4;  /* starting point for FAT12 and FAT16 */
    int def_root_dir_entries = 512;

    if (devInfo.geom_heads > 0)
    {
        heads = devInfo.geom_heads;
        sec_per_track = devInfo.geom_sectors;
    }
    else
    {
        unsigned long long int total_sectors;

        if (devInfo.geom_size > 0)
            total_sectors = devInfo.geom_size;
        else if (devInfo.sector_size > 0)
            total_sectors = devInfo.size / devInfo.sector_size;
        else
            total_sectors = devInfo.size / sector_size;

        if (total_sectors <= 524288)
        {
            /* For capacity below the 256MB (with 512b sectors) use CHS Recommendation from SD Card Part 2 File System Specification */
            heads = total_sectors <=  32768 ? 2 :
                    total_sectors <=  65536 ? 4 :
                    total_sectors <= 262144 ? 8 : 16;
            sec_per_track = total_sectors <= 4096 ? 16 : 32;
        }
        else
        {
            /* Use LBA-Assist Translation for calculating CHS when disk geometry is not available */
            heads = total_sectors <=  16*63*1024 ? 16 :
                    total_sectors <=  32*63*1024 ? 32 :
                    total_sectors <=  64*63*1024 ? 64 :
                    total_sectors <= 128*63*1024 ? 128 : 255;
            sec_per_track = 63;
        }
    }
    if (devInfo.type != TYPE_FIXED)
    {
        /* enter default parameters for floppy disks if the size matches */
        switch (devInfo.size / 1024)
        {
        case 360:
            sec_per_track = 9;
            heads = 2;
            media = 0xfd;
            cluster_size = 2;
            def_root_dir_entries = 112;
            break;

        case 720:
            sec_per_track = 9;
            heads = 2;
            media = 0xf9;
            cluster_size = 2;
            def_root_dir_entries = 112;
            break;

        case 1200:
            sec_per_track = 15;
            heads = 2;
            media = 0xf9;
            cluster_size = 1;
            def_root_dir_entries = 224;
            break;

        case 1440:
            sec_per_track = 18;
            heads = 2;
            media = 0xf0;
            cluster_size = 1;
            def_root_dir_entries = 224;
            break;

        case 2880:
            sec_per_track = 36;
            heads = 2;
            media = 0xf0;
            cluster_size = 2;
            def_root_dir_entries = 224;
            break;
        }
    }
    hidden_sectors = 1;
	root_dir_entries = def_root_dir_entries;
    bs.secs_track = sec_per_track;
    bs.heads = heads;
    bs.media = media;
    bs.cluster_size = cluster_size;

    part_sector=0;
    cblocks = (devInfo.size - part_sector * sector_size) / BLOCK_SIZE;
    orphaned_sectors = ((devInfo.size - part_sector * sector_size) % BLOCK_SIZE) / sector_size;

}

#define fat32	fstype._fat32
#define oldfat	fstype._oldfat

uint32_t FileSystem::generate_volume_id(void)
{
    struct timeval now;

    if (gettimeofday(&now, NULL) != 0 || now.tv_sec == (time_t)-1 || now.tv_sec < 0)
    {
        srand(time(NULL));
        /* rand() returns int from [0,RAND_MAX], therefore only 31 bits */
        return (((uint32_t)(rand() & 0xFFFF)) << 16) | ((uint32_t)(rand() & 0xFFFF));
    }

    /* volume ID = current time, fudged for more uniqueness */
    return ((uint32_t)now.tv_sec << 20) | (uint32_t)now.tv_usec;
}

void FileSystem::setup_tables(FSType fileSystemType)
{
    unsigned cluster_count = 0, fat_length;
    struct tm *ctime;
    struct msdos_volume_info *vi = (fileSystemType == FS_FAT32 ? &bs.fat32.vi : &bs.oldfat.vi);
    char label[12] = { 0 };
    size_t len;
    int ret;
    int i;

    memcpy((char *)bs.system_id, "mkfs.fat", strlen("mkfs.fat"));
    if (bs.media == 0xf8)
        vi->drive_number=0x80;
    else
        vi->drive_number=0x00;

    if (fileSystemType == FS_FAT32) {
        root_dir_entries = 0;
    }

    memcpy(label, NO_NAME, MSDOS_NAME);

    long volume_id = this->generate_volume_id();		/* Volume ID number */
    vi->volume_id[0] = (unsigned char)(volume_id & 0x000000ff);
    vi->volume_id[1] = (unsigned char)((volume_id & 0x0000ff00) >> 8);
    vi->volume_id[2] = (unsigned char)((volume_id & 0x00ff0000) >> 16);
    vi->volume_id[3] = (unsigned char)(volume_id >> 24);
    memcpy(vi->volume_label, label, 11);

    memcpy(bs.boot_jump, dummy_boot_jump, 3);
    /* Patch in the correct offset to the boot code */
    bs.boot_jump[1] = ((size_fat == 32 ?
                        (char *)&bs.fat32.boot_code :
                        (char *)&bs.oldfat.boot_code) - (char *)&bs) - 2;

    if (fileSystemType == FS_FAT32)
    {
        int offset = (char *)&bs.fat32.boot_code - (char *)&bs + MESSAGE_OFFSET + 0x7c00;
        if (dummy_boot_code[BOOTCODE_FAT32_SIZE - 1])
            printf("Warning: message too long; truncated\n");
        dummy_boot_code[BOOTCODE_FAT32_SIZE - 1] = 0;
        memcpy(bs.fat32.boot_code, dummy_boot_code, BOOTCODE_FAT32_SIZE);
        bs.fat32.boot_code[MSG_OFFSET_OFFSET] = offset & 0xff;
        bs.fat32.boot_code[MSG_OFFSET_OFFSET + 1] = offset >> 8;
    }
    else
    {
        memcpy(bs.oldfat.boot_code, dummy_boot_code, BOOTCODE_SIZE);
    }
    bs.boot_sign = BOOT_SIGN;
    bs.reserved = (fileSystemType == FS_FAT32) ? 32 : 1;
    nr_fats = 2;
    bs.fats = (char)nr_fats;
    hidden_sectors = 0;
    bs.hidden = hidden_sectors;
}

void FileSystem::gatherDeviceInformation(FSType fileSystemType){
    io = new File();
    io->open(filename,READWRITE);

    int handle = io->getHandle();
    info(handle);

    printf("devinfo type:        %d\n",devInfo.type);
    printf("devinfo parition:    %d\n",devInfo.partition);
    printf("devinfo children:    %d\n",devInfo.has_children);
    printf("devinfo heads:       %d\n",devInfo.geom_heads);
    printf("devinfo sectors:     %d\n",devInfo.geom_sectors);
    printf("devinfo start:       %lld\n",devInfo.geom_start);
    printf("devinfo size:        %lld\n",devInfo.geom_size);
    printf("devinfo sector size: %d\n",devInfo.sector_size);
    printf("devinfo size:        %lld\n",devInfo.size);


    //establish_params();
    //setup_tables(fileSystemType);

    io->close();
}

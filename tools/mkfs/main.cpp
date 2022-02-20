#include "mkfs.h"
#include "file.h"
#include "endian.h"

using namespace std;

enum {OPT_HELP=1000};
const struct option long_options[] =
{
    {"help",      no_argument,       NULL, OPT_HELP},
    {0,}
};

const struct device_info device_info_clueless =
{
    .type         = TYPE_UNKNOWN,
    .partition    = -1,
    .has_children = -1,
    .geom_heads   = -1,
    .geom_sectors = -1,
    .geom_start   = -1,
    .geom_size    = -1,
    .sector_size  = -1,
    .size         = -1,
};
char initial_volume_name[] = NO_NAME;    /* Initial volume name, make sure that is writable */
char *volume_name = initial_volume_name; /* Volume name */
struct msdos_boot_sector bs;	         /* Boot sector data */


void die(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

void setup_tables(ProgramArgumentsPtr arguments){
    unsigned cluster_count = 0, fat_length;
    struct tm *ctime;
    struct msdos_volume_info *vi = (arguments->size_fat == 32 ? &bs.fat32.vi : &bs.oldfat.vi);
    char label[12] = { 0 };
    size_t len;
    int ret;
    int i;

    memcpy((char *)bs.system_id, "mkfs.fat", strlen("mkfs.fat"));
}

void establish_params(ProgramArgumentsPtr arguments, struct device_info *info)
{
    unsigned int sec_per_track;
    unsigned int heads;
    unsigned int media = 0xf8;
    unsigned int cluster_size = 4;  /* starting point for FAT12 and FAT16 */
    int def_root_dir_entries = 512;

    if (info->geom_heads > 0)
    {
        heads = info->geom_heads;
        sec_per_track = info->geom_sectors;
    }
    else
    {
        unsigned long long int total_sectors;

        if (info->geom_size > 0)
            total_sectors = info->geom_size;
        else if (info->sector_size > 0)
            total_sectors = info->size / info->sector_size;
        else
            total_sectors = info->size / arguments->sector_size;

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

    if (info->type != TYPE_FIXED)
    {
        /* enter default parameters for floppy disks if the size matches */
        switch (info->size / 1024)
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

    if (!arguments->size_fat && info->size >= 512 * 1024 * 1024)
    {
        printf("Auto-selecting FAT32 for large filesystem\n");
        arguments->size_fat = 32;
    }
    if (arguments->size_fat == 32)
    {
        /*
         * For FAT32, try to do the same as M$'s format command
         * (see http://www.win.tue.nl/~aeb/linux/fs/fat/fatgen103.pdf p. 20):
         * fs size <= 260M: 0.5k clusters
         * fs size <=   8G:   4k clusters
         * fs size <=  16G:   8k clusters
         * fs size <=  32G:  16k clusters
         * fs size >   32G:  32k clusters
         */
        unsigned long long int sectors = info->size / arguments->sector_size;
        cluster_size = sectors > 32*1024*1024*2 ? 64 :
                       sectors > 16*1024*1024*2 ? 32 :
                       sectors >  8*1024*1024*2 ? 16 :
                       sectors >     260*1024*2 ? 8 : 1;
    }

    if (!arguments->hidden_sectors_by_user && info->geom_start >= 0 && info->geom_start + arguments->part_sector <= UINT32_MAX)
        arguments->hidden_sectors = info->geom_start + arguments->part_sector;

    if (!arguments->root_dir_entries)
        arguments->root_dir_entries = def_root_dir_entries;

    if (!bs.secs_track)
        bs.secs_track = htole16(sec_per_track);
    if (!bs.heads)
        bs.heads = htole16(heads);
    bs.media = media;
    bs.cluster_size = cluster_size;
}

int get_device_info(int fd, struct device_info *info)
{
    struct stat stat;
    int ret;

    *info = device_info_clueless;

    ret = fstat(fd, &stat);
    if (ret < 0)
    {
        die("fstat on target failed");
        return -1;
    }

    if (S_ISREG(stat.st_mode))
    {
        info->type = TYPE_FILE;
        info->partition = 0;
        info->size = stat.st_size;
        return 0;
    }

    if (!S_ISBLK(stat.st_mode))
    {
        info->type = TYPE_BAD;
        return 0;
    }
    return 0;
}

void usage(const char *name, int exitval)
{
    fprintf(stderr, "Usage: %s [OPTIONS] TARGET [BLOCKS]\n", name);
    fprintf(stderr, "Create FAT filesystem in TARGET, which can be a block device or file. Use only\n");
    fprintf(stderr, "up to BLOCKS 1024 byte blocks if specified. With the -C option, file TARGET will be\n");
    fprintf(stderr, "created with a size of 1024 bytes times BLOCKS, which must be specified.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -C              Create file TARGET then create filesystem in it\n");
    fprintf(stderr, "  -F SIZE         Select FAT size SIZE (12, 16 or 32)\n");
    fprintf(stderr, "  -s COUNT        Set number of sectors per cluster to COUNT\n");
    fprintf(stderr, "  -S SIZE         Select a sector size of SIZE (a power of two, at least 512)\n");
    fprintf(stderr, "  --help          Show this help message and exit\n");
    exit(exitval);
}

ProgramArgumentsPtr getArguments(int argc, char **argv)
{
    int c;
    char *tmp;
    int conversion;

    ProgramArgumentsPtr arguments = new ProgramArguments;
    memset(arguments,0,sizeof(*arguments));
    arguments->sector_size=512;
    while ((c = getopt_long(argc, argv, "CF:s:S:h", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'C':		/* C : Create a new file */
            arguments->create = true;
            break;
        case 'F':		/* F : Choose FAT size */
            conversion = strtol(optarg, &tmp, 0);
            if (!*optarg || isspace(*optarg) || *tmp || errno || (conversion != 12 && conversion != 16 && conversion != 32))
            {
                printf("Bad FAT type : %s\n", optarg);
                usage(argv[0], 1);
            }
            arguments->size_fat = conversion;
            arguments->size_fat_by_user = 1;
            break;
        case 's':		/* s : Sectors per cluster */
            conversion = strtol(optarg, &tmp, 0);
            if (!*optarg || isspace(*optarg) || *tmp || errno || (conversion != 1 && conversion != 2
                    && conversion != 4 && conversion != 8 && conversion != 16
                    && conversion != 32 && conversion != 64 && conversion != 128))
            {
                printf("Bad number of sectors per cluster : %s\n", optarg);
                usage(argv[0], 1);
            }
            arguments->sectors_per_cluster = conversion;
            break;
        case 'S':		/* S : Sector size */
            conversion = strtol(optarg, &tmp, 0);
            if (!*optarg || isspace(*optarg) || *tmp || errno || (conversion != 512 && conversion != 1024 &&
                    conversion != 2048 && conversion != 4096 && conversion != 8192 &&
                    conversion != 16384 && conversion != 32768))
            {
                printf("Bad logical sector size : %s\n", optarg);
                usage(argv[0], 1);
            }
            arguments->sector_size = conversion;
            arguments->sector_size_set = 1;
            break;
        case OPT_HELP:
            usage(argv[0], 0);
            break;
        case '?':
            usage(argv[0], 1);
            exit(1);
        default:
            fprintf(stderr,"Internal error: getopt_long() returned unexpected value %d\n", c);
            exit(2);
        }
    }
    if (optind == argc || !argv[optind])
    {
        printf("No device specified.\n");
        usage(argv[0], 1);
    }
    arguments->device_name = argv[optind++];
    if (optind != argc)
    {
        arguments->blocks_specified = 1;
        conversion = strtoll(argv[optind], &tmp, 0);
        if (!*argv[optind] || isspace(*argv[optind]) || *tmp || errno || conversion < 0)
        {
            printf("Bad block count : %s\n", argv[optind]);
            usage(argv[0], 1);
        }
        arguments->blocks = conversion;
        optind++;
    }
    if (optind != argc)
    {
        fprintf(stderr, "Excess arguments on command line\n");
        usage(argv[0], 1);
    }
    return arguments;
}

void printArguments(ProgramArgumentsPtr arguments)
{
    printf("Create File         = %s\n",arguments->create? "true" : "false");
    printf("Size of FAT         = %d\n",arguments->size_fat);
    printf("Size of FAT BY User = %d\n",arguments->size_fat_by_user);
    printf("Sector Size         = %d\n",arguments->sector_size);
    printf("Sector Size Set     = %d\n",arguments->sector_size_set);
    printf("Sector per cluster  = %d\n",arguments->sectors_per_cluster);
    printf("Device name         = %s\n",arguments->device_name);
    printf("Blocks specified    = %d\n",arguments->blocks_specified);
    printf("Blocks              = %lld\n",arguments->blocks);
}

uint32_t generate_volume_id(void)
{
    struct timeval now;

    if (gettimeofday(&now, NULL) != 0 || now.tv_sec == (time_t)-1 || now.tv_sec < 0)
    {
        srand(getpid());
        /* rand() returns int from [0,RAND_MAX], therefore only 31 bits */
        return (((uint32_t)(rand() & 0xFFFF)) << 16) | ((uint32_t)(rand() & 0xFFFF));
    }

    /* volume ID = current time, fudged for more uniqueness */
    return ((uint32_t)now.tv_sec << 20) | (uint32_t)now.tv_usec;
}

int main (int argc, char **argv)
{
    printf("mkfs " VERSION " (" VERSION_DATE ")\n");
    ProgramArgumentsPtr arguments = getArguments(argc,argv);
    arguments->volume_id = generate_volume_id();
    printArguments(arguments);
    File file;
    if (arguments->create)
    {
        file.open(arguments->device_name,READWRITE,CREATE);
        int blocks = arguments->blocks;
        int part_sector = arguments->part_sector;
        int sector_size = arguments->sector_size;
        if (file.truncate(part_sector * sector_size + blocks * BLOCK_SIZE))
        {
            die("unable to resize %s", arguments->device_name);
        }
    }
    else
    {
        file.open(arguments->device_name,READWRITE,NOCREATE);
    }
    struct device_info devinfo;
    if (get_device_info(file.getHandle(), &devinfo) < 0)
        die("error collecting information about %s", arguments->device_name);
    if (devinfo.size <= 0)
        die("unable to discover size of %s", arguments->device_name);
    if (devinfo.sector_size > 0)
    {
        if (arguments->sector_size_set)
        {
            if (arguments->sector_size < devinfo.sector_size)
            {
                arguments->sector_size = devinfo.sector_size;
                fprintf(stderr, "Warning: sector size was set to %d (minimal for this device)\n", arguments->sector_size);
            }
        }
        else
        {
            arguments->sector_size = devinfo.sector_size;
            arguments->sector_size_set = 1;
        }

        if (devinfo.size <= arguments->part_sector * arguments->sector_size)
            die("The device %s size %llu is less then the offset %llu", arguments->device_name, devinfo.size, (unsigned long long) arguments->part_sector * arguments->sector_size);
    }
    if (arguments->sector_size > 4096)
        fprintf(stderr,"Warning: sector size %d > 4096 is non-standard, filesystem may not be usable\n", arguments->sector_size);

    long long cblocks = (devinfo.size - arguments->part_sector * arguments->sector_size) / BLOCK_SIZE;
    long long orphaned_sectors = ((devinfo.size - arguments->part_sector * arguments->sector_size) % BLOCK_SIZE) / arguments->sector_size;

    if (arguments->blocks_specified)
    {
        if (arguments->blocks != cblocks)
        {
            fprintf(stderr, "Warning: block count mismatch: ");
            fprintf(stderr, "found %llu but assuming %llu.\n", cblocks, arguments->blocks);
        }
    }
    else
    {
        arguments->blocks = cblocks;
    }
    establish_params(arguments,&devinfo);
    setup_tables(arguments);

    file.close();
    return 0;
}

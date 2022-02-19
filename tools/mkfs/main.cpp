#include "mkfs.h"

using namespace std;

enum {OPT_HELP=1000};
const struct option long_options[] =
{
    {"help",      no_argument,       NULL, OPT_HELP},
    {0,}
};

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

ProgramArgumentsPtr getArguments(int argc, char **argv){
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

void printArguments(ProgramArgumentsPtr arguments){
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

int main (int argc, char **argv)
{
    printf("mkfs " VERSION " (" VERSION_DATE ")\n");
    ProgramArgumentsPtr arguments = getArguments(argc,argv);
    printArguments(arguments);
    return 0;
}

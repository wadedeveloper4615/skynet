#include "mkfs.h"

extern unsigned char *fat;	                     /* File allocation table */
extern unsigned char *info_sector_buffer;	     /* FAT32 info sector */
extern struct msdos_dir_entry *root_dir;	     /* Root directory */
extern int dev;     		                     /* FS block device file handle */
extern off_t part_sector;                        /* partition offset in sector */
extern unsigned sector_size;     	             /* Size of a logical sector */
extern const char *program_name;

void error(const char *msg, ...)
{
    free(fat);
    free(info_sector_buffer);
    free(root_dir);
    va_list args;
    if (program_name)
    {
        fprintf(stderr, "%s: ", program_name);
    }
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

void seekto(off_t pos,char *errstr)
{
    off_t __pos = (pos);
    if (lseek(dev, part_sector * sector_size + __pos, SEEK_SET) != part_sector * sector_size + __pos)				\
        error("seek to %s failed whilst writing tables",errstr);
}

void writebuf(void *buf,int size,char *errstr)
{
    int __size = (size);
    if (write(dev, buf, __size) != __size)
        error("failed whilst writing %s",errstr);
}

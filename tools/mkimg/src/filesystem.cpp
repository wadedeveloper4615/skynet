#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "filesystem.h"
#include "file.h"
#include "fat.h"

FileSystem::FileSystem(string filename)
{
    this->filename = filename;
}

FileSystem::~FileSystem()
{
    delete io;
}

int32s FileSystem::fsize()
{
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;
    fprintf(stderr, "Cannot determine size of %s: %s\n", filename, strerror(errno));
    return -1;
}

int32u FileSystem::createImage(int32u blockSize, int32u numberOfBlocks)
{
    io = new File();
    io->open(filename,"wb");
    io->seek(0,SEEK_SET);
    BYTE *buffer = new BYTE[blockSize];
    memset(buffer,0,blockSize);
    for(int i=0; i<numberOfBlocks; i++)
    {
        io->write(buffer,blockSize);
    }
    delete[] buffer;
    io->close();

    return fsize();
}

int8u FileSystem::createFat12(ResourceDataPtr data)
{
    unsigned cylinders = (unsigned) (data->tot_sectors + ((16*63)-1)) / (16*63);    // cylinders used
    unsigned add = (unsigned) (((int64u) cylinders * (16*63)) - data->tot_sectors); // sectors to add to boundary on cylinder
    int64u tot_sects = data->tot_sectors;

    // check a few things
    if ((data->fatType != 12) && (data->fatType != 16) && (data->fatType != 32))
    {
        printf("FAT size must be 12, 16, or 32.\n");
        return -1;
    }

    if (!POWERofTWO(data->spc) || data->spc > 64)
    {
        printf("Sectors Per Cluster must be a power of 2 and <= 64\n");
        return -1;
    }
    io->open(filename,"wb");
    io->seek(0,SEEK_SET);
    BYTE *buffer = new BYTE[SECT_RES32 * SECT_SIZE];
    memset(buffer,0,SECT_RES32 * SECT_SIZE);
    return 1;
}


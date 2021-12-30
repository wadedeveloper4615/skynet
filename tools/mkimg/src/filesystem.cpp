#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "filesystem.h"
#include "file.h"

FileSystem::FileSystem(char *filename)
{
    this->filename = filename;
}

FileSystem::~FileSystem()
{
    delete io;
}

_off_t FileSystem::fsize()
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


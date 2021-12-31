#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "file.h"

class FileSystem
{
private:
    char *filename;
    File *io;
public:
    FileSystem(char *filename);
    virtual ~FileSystem();
    DWORD createImage(DWORD blockSize, DWORD numberOfBlocks);
protected:
    _off_t fsize();
};

#endif // FILESYSTEM_H

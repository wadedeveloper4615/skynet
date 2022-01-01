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
#include "types.h"
#include "file.h"
#include "fat.h"

class FileSystem
{
private:
    string filename;
    File *io;
public:
    FileSystem(string filename);
    virtual ~FileSystem();
    int32u createImage(int32u blockSize, int32u numberOfBlocks);
    int8u createFat12(ResourceDataPtr data);
protected:
    int32s fsize();
};

#endif // FILESYSTEM_H

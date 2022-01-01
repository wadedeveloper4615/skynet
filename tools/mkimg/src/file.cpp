#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <strsafe.h>
#include <errno.h>
#include <cstdint>
#include "file.h"

File::File()
{
}

void File::open(LPCTSTR filename,const char *mode)
{
    handle = fopen(filename,mode);
}

File::~File()
{
    close();
}

void File::close()
{
    fclose(handle);
}

int32u File::read(void *buffer,int32u Size)
{
    return fread(buffer, Size, 1, handle);
}

int32u File::write(void *buffer,int32u Size)
{
    return fwrite(buffer, Size, 1, handle);
}

int64s File::seek(int64s offset,int32u origin)
{
    return _fseeki64(handle,offset,origin);
}

int64s File::tell()
{
    return _ftelli64(handle);
}

int32u File::read(void *buffer, int64s offset, int32u size)
{
    seek(offset,SEEK_SET);
    return read(buffer,size);
}


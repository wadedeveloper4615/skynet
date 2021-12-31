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

DWORD File::read(void *buffer,DWORD Size)
{
    return fread(buffer, Size,1,handle);
}

DWORD File::write(void *buffer,DWORD Size)
{
    return fwrite(buffer, Size, 1, handle);
}

__int64 File::seek(__int64 offset,DWORD origin)
{
    return _fseeki64(handle,offset,origin);
}

__int64 File::tell()
{
    return _ftelli64(handle);
}

DWORD File::read(void *buffer, __int64 offset, DWORD size)
{
    seek(offset,SEEK_SET);
    return read(buffer,size);
}


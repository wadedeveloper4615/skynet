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

void File::open(LPCTSTR filename,FileState state){
    handle = -1;
    if (state==READ)
    {
        handle = _open(filename,_O_RDONLY|_O_BINARY,_S_IREAD);
    }
    if (state==WRITE)
    {
        handle = _open(filename,_O_WRONLY|_O_BINARY,_S_IWRITE);
    }
    if (state==READWRITE)
    {
        handle = _open(filename,_O_RDWR|_O_BINARY,_S_IREAD | _S_IWRITE);
    }
    if (handle == -1)
    {
        perror( "Open failed on input file" );
        exit(1);
    }
}

File::~File()
{
    close();
}

void File::close()
{
    _close(handle);
}

DWORD File::read(void *buffer,DWORD Size)
{
    DWORD dwBytesRead = _read(handle, buffer, Size);
    if (dwBytesRead == -1)
    {
        perror( "Problem reading file" );
        close();
        exit(1);
    }
    return dwBytesRead;
}

DWORD File::write(void *buffer,DWORD Size)
{
    DWORD dwBytesWritten = _write(handle, buffer, Size);
    if (dwBytesWritten == -1)
    {
        switch(errno)
        {
        case EBADF:
            perror("Bad file descriptor!");
            break;
        case ENOSPC:
            perror("No space left on device!");
            break;
        case EINVAL:
            perror("Invalid parameter: buffer was NULL!");
            break;
        default:
            perror("Unexpected error!");
        }
        close();
        exit(1);
    }
    return dwBytesWritten;
}

__int64 File::seek(__int64 offset,DWORD origin)
{
    return _lseeki64(handle,offset,origin);
}

__int64 File::tell()
{
    return _telli64(handle);
}

DWORD File::read(void *buffer, __int64 offset, DWORD size)
{
    seek(offset,SEEK_SET);
    return read(buffer,size);
}


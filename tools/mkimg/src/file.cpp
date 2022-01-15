#include <tchar.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <strsafe.h>
#include <errno.h>
#include "file.h"

File::File(const char *filename)
{
    this->filename = filename;
}

void File::open(FileState state){
    handle = -1;
    if (state==READ)
    {
        handle = _open(filename,_O_RDONLY|_O_BINARY,_S_IREAD);
    }
    if (state==WRITE)
    {
        handle = _open(filename,_O_WRONLY|_O_BINARY|_O_CREAT,_S_IWRITE);
    }
    if (state==READWRITE)
    {
        handle = _open(filename,_O_RDWR|_O_BINARY|_O_CREAT,_S_IREAD|_S_IWRITE);
    }
    if (handle == -1)
    {
        perror( "Open failed on input file" );
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

int32u File::read(void *buffer,int32u Size)
{
    int32u dwBytesRead = _read(handle, buffer, Size);
    if (dwBytesRead == -1)
    {
        perror( "Problem reading file" );
        close();
    }
    return dwBytesRead;
}

int32u File::write(void *buffer,int32u Size)
{
    int32u dwBytesWritten = _write(handle, buffer, Size);
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
    }
    return dwBytesWritten;
}

int64s File::seek(int64s offset,int32u origin)
{
    return _lseeki64(handle,offset,origin);
}

int64s File::tell()
{
    return _telli64(handle);
}

int32u File::read(void *buffer, int64s offset, int32u size)
{
    seek(offset,SEEK_SET);
    return read(buffer,size);
}


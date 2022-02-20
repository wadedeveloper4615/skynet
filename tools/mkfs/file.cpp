#include "mkfs.h"
#include "file.h"

File::File()
{
}

void File::open(char *filename,FileState state, CreationFlag flag)
{
    handle = -1;
    int oflag=0;
    int pmode=0;
    if (state==READ)
    {
        oflag = _O_RDONLY|_O_BINARY;
        pmode = _S_IREAD;
    }
    if (state==WRITE)
    {
        oflag = _O_WRONLY|_O_BINARY;
        pmode = _S_IWRITE;
    }
    if (state==READWRITE)
    {
        oflag = _O_RDWR|_O_BINARY;
        pmode = _S_IREAD|_S_IWRITE;
    }
    if (flag==CREATE){
        oflag |= _O_CREAT;
    }
    handle = _open(filename,oflag,pmode);
    if (handle == -1)
    {
        die( "Open failed on input file '%s'",filename);
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

int File::truncate(off64_t offset){
    return ftruncate64(handle, offset);
}

__int64 File::seek(__int64 offset,DWORD origin)
{
    return _lseeki64(handle,offset,origin);
}

__int64 File::tell()
{
    return _telli64(handle);
}


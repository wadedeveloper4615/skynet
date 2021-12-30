#ifndef FILE_H
#define FILE_H

#include <windows.h>

typedef enum _FileState
{
    READ,
    WRITE,
    READWRITE
} FileState;

class File
{
public:
    File();
    virtual ~File();

    void open(LPCTSTR filename,FileState state);
    DWORD read(void *buffer,DWORD Size);
    DWORD write(void *buffer,DWORD Size);
    __int64 seek(__int64 Offset,DWORD Origin);
    __int64 tell();
    DWORD read(void *Buffer, LONGLONG offset, DWORD size);
    void close();
    inline int getHandle(){return handle;};
protected:
private:
    int handle;
};

#endif // FILE_H

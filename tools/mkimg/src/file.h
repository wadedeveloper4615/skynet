#ifndef FILE_H
#define FILE_H

#include <windows.h>

class File
{
public:
    File();
    virtual ~File();

    void open(LPCTSTR filename,const char *mode);
    DWORD read(void *buffer,DWORD Size);
    DWORD write(void *buffer,DWORD Size);
    __int64 seek(__int64 Offset,DWORD Origin);
    __int64 tell();
    DWORD read(void *Buffer, LONGLONG offset, DWORD size);
    void close();
    inline FILE *getHandle(){return handle;};
protected:
private:
    FILE *handle;
};

#endif // FILE_H

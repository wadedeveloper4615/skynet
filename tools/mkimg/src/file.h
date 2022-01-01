#ifndef FILE_H
#define FILE_H

#include <windows.h>
#include "types.h"

class File
{
public:
    File();
    virtual ~File();

    void open(LPCTSTR filename,const char *mode);
    int32u read(void *buffer,int32u Size);
    int32u write(void *buffer,int32u Size);
    int64s seek(int64s Offset,int32u Origin);
    int64s tell();
    int32u read(void *Buffer, int64s offset, int32u size);
    void close();
    inline FILE *getHandle(){return handle;};
protected:
private:
    FILE *handle;
};

#endif // FILE_H

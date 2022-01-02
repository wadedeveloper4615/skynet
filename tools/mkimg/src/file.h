#ifndef FILE_H
#define FILE_H

#include "types.h"

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

    void open(const char *filename,FileState state);
    int32u read(void *buffer,int32u Size);
    int32u write(void *buffer,int32u Size);
    int64s seek(int64s Offset,int32u Origin);
    int64s tell();
    int32u read(void *Buffer, int64s offset, int32u size);
    void close();
    inline int getHandle(){return handle;};
protected:
private:
    int handle;
};

#endif // FILE_H

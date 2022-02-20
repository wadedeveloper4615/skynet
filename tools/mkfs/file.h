#ifndef FILE_H
#define FILE_H

#include "types.h"

typedef enum _FileState
{
    READ,
    WRITE,
    READWRITE
} FileState;

typedef enum _CreationFlag
{
    CREATE,
    NOCREATE
} CreationFlag;

class File
{
public:
    File();
    virtual ~File();

    void open(char *filename,FileState state, CreationFlag flag);
    int32u read(void *buffer,int32u Size);
    int32u write(void *buffer,int32u Size);
    int64s seek(int64s Offset,int32u Origin);
    int truncate(off64_t offset);
    int64s tell();
    void close();
    inline int getHandle(){return handle;};
protected:
private:
    int handle;
};

#endif // FILE_H

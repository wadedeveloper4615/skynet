#ifndef FATFILESYSTEM_H_INCLUDED
#define FATFILESYSTEM_H_INCLUDED

class FATFileSystem
{
protected:
    File *io;
public:
    virtual void parse()=0;
    virtual void print()=0;
    virtual void printRootDir()=0;
};


#endif // FATFILESYSTEM_H_INCLUDED

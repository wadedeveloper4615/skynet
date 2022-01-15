#include "fat12.h"

void createEmptyFile(File* file, int64u sectors, int32u sectorSize)
{
    file->open(WRITE);
    file->seek(SEEK_SET,0);
    char buffer[sectorSize];
    for (int i=0; i<sectors; i++)
    {
        memset(buffer,0,sizeof(buffer));
        file->write(buffer,sizeof(buffer));
    }
    file->close();
}

void createFat12FileSystem(File *file){
    FAT12 *fat=new FAT12(file);
    fat->create();
}

int main(int argc, char *argv[])
{
    File file("C:\\projects\\skynet\\tools\\mkimg\\images\\fat12.img");
    createEmptyFile(&file,2880,512);
    createFat12FileSystem(&file);
    return 0;
}

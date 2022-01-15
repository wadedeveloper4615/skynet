#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "fat12.h"

int32u generate_volume_id(void)
{
    struct timeval now;

    if (gettimeofday(&now, NULL) != 0 || now.tv_sec == (time_t)-1 || now.tv_sec < 0)
    {
        srand(time(NULL));
        /* rand() returns int from [0,RAND_MAX], therefore only 31 bits */
        return (((int32u)(rand() & 0xFFFF)) << 16) | ((int32u)(rand() & 0xFFFF));
    }

    /* volume ID = current time, fudged for more uniqueness */
    return ((int32u)now.tv_sec << 20) | (int32u)now.tv_usec;
}

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

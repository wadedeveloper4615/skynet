#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "file.h"

void createEmptyFile(File& file, int64u sectors, int32u sectorSize)
{
    file.open("C:\\projects\\skynet\\tools\\mkimg\\images\\fat12.img",WRITE);
    file.seek(SEEK_SET,0);
    char buffer[sectorSize];
    for (int i=0; i<sectors; i++)
    {
        memset(buffer,0,sizeof(buffer));
        file.write(buffer,sizeof(buffer));
    }
    file.close();
}

int main(int argc, char *argv[])
{
    File file;
    createEmptyFile(file,2880,512);
    return 0;
}

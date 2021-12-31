#include "filesystem.h"

int main()
{
    char *filename="C:\\projects\\skynet\\output\\intermediate\\images\\boot_a.img";
    FileSystem fs(filename);
    DWORD size = fs.createImage(512,2880);
    printf("Created image file %s of size %ld bytes\n",filename,size);
    return 0;
}

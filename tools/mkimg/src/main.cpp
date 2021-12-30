#include "filesystem.h"
//#include "FatData.h"

int main()
{
    FileSystem fs("C:\\projects\\linux\\tools\\DiskImage\\images\\new.img");
    fs.createImage(512,2880);
    printf("================================\n");
    return 0;
}

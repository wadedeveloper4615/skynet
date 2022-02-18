#include "filesystem.h"
#include "FatData.h"

void parseFileSystem(char *filename){
    FileSystem fs(filename);
    fs.parse();
}

int main()
{
    //parseFileSystem("C:\\projects\\skynet\\tools\\dumpimg\\images\\floppy.img");
    //parseFileSystem("C:\\projects\\skynet\\tools\\dumpimg\\images\\a.img");
    parseFileSystem("C:\\projects\\imgtest\\fat12_2.img");
    //parseFileSystem("C:\\projects\\skynet\\tools\\dumpimg\\images\\fat16NoMbr.img");
    //parseFileSystem("C:\\projects\\skynet\\tools\\dumpimg\\images\\fat16Mbr.img");
    //parseFileSystem("C:\\projects\\skynet\\tools\\dumpimg\\images\\fat32NoMbr.img");
    //parseFileSystem("C:\\projects\\skynet\\tools\\dumpimg\\images\\fat32Mbr.img");
    printf("Execution Complete\n");
    return 0;
}

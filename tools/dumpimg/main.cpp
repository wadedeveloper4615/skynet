#include "filesystem.h"
#include "FatData.h"

void parseFileSystem(char *filename){
    FileSystem fs(filename);
    fs.parse();
}

int main(int argc, char **argv)
{
    parseFileSystem(argv[1]);
    printf("Execution Complete\n");
    return 0;
}

#include "filesystem.h"

int main()
{
    string filename="C:\\projects\\skynet\\output\\intermediate\\images\\boot_a.img";
    FileSystem fs(filename);
    DWORD size = fs.createImage(512,2880);
    printf("Created image file %s of size %ld bytes\n",filename,size);
    ResourceDataPtr data = new ResourceData;
    memset(data, 0, sizeof(*data));
    data->base_lba = 63;        // Base LBA of partition (LBA of 1st sector of boot code)
    data->tot_sectors = 2880;   // Total number of sectors in partition
    data->fatType = 12;         // Create FAT 12 image
    data->spc = 1;              // Sectors Per Cluster
    data->heads = 2;           // heads per cylinder
    data->spt = 18;             // sectors per track
    fs.createFat12(data);
    printf("Created FAT structures in '%s'\n",filename,size);
    return 0;
}

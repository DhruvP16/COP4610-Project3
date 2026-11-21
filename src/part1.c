#include "part1.h"
#include <inttypes.h>
#include <string.h>

// Open the FAT32 image and read its boot sector (Return 0 on success, -1 failure)
int open_image(FAT32 *fat32, const char *filename) {
    // Empty FAT32 structure.
    memset(fat32, 0, sizeof(*fat32));

    // Open the image, read only binary mode
    fat32->image = fopen(filename, "rb");

    if (fat32->image == NULL) {
        perror("Error opening image");
        return -1;
    }

    // Read the BPB from the beginning of the image
    if (fread(&fat32->bpb, sizeof(BPB), 1, fat32->image) != 1) {
        fprintf(stderr, "Error: could not read boot sector\n");
        close_image(fat32);
        return -1;
    }

    // Pointer to access the BPB fields more easily
    BPB *bpb = &fat32->bpb;

    // Calculate how many sectors are reserved or used by FATs
    uint64_t used_sectors = (uint64_t)bpb->BPB_RsvdSecCnt + (uint64_t)bpb->BPB_NumFATs * bpb->BPB_FATSz32;

    // Check for invalid values in important FAT32 fields
    if (bpb->BPB_BytsPerSec == 0 || bpb->BPB_SecPerClus == 0 || bpb->BPB_RsvdSecCnt == 0 ||
        bpb->BPB_NumFATs == 0 || bpb->BPB_FATSz16 != 0 || bpb->BPB_FATSz32 == 0 ||
        bpb->BPB_RootClus < 2 || bpb->BPB_TotSec32 <= used_sectors) {
        fprintf(stderr, "Error: invalid FAT32 boot sector\n");
        close_image(fat32);
        return -1;
    }

    // Move to the end of the image to determine its size
    if (fseek(fat32->image, 0, SEEK_END) != 0) {
        perror("Error seeking image");
        close_image(fat32);
        return -1;
    }

    // ftell returns the current byte position
    long length = ftell(fat32->image);

    if (length < 0) {
        perror("Error getting image size");
        close_image(fat32);
        return -1;
    }

    // Save the image size in bytes
    fat32->image_size = (uint64_t)length;

    // Check that the image contains its declared volume
    if (fat32->image_size < (uint64_t)bpb->BPB_TotSec32 * bpb->BPB_BytsPerSec) {
        fprintf(stderr, "Error: image is smaller than its declared FAT32 volume\n");
        close_image(fat32);
        return -1;
    }

    // Initially, the current directory is the root directory
    fat32->current_cluster = bpb->BPB_RootClus;
    return 0;
}


// Print the six FAT32 properties required by Part 1
void command_info(const FAT32 *fat32) {
    const BPB *bpb = &fat32->bpb;

    // Find how many sectors are used before the data region
    uint64_t used_sectors = (uint64_t)bpb->BPB_RsvdSecCnt + (uint64_t)bpb->BPB_NumFATs * bpb->BPB_FATSz32;

    // Remaining sectors belong to the data region.
    uint64_t data_sectors = bpb->BPB_TotSec32 - used_sectors;

    // Convert data sectors into clusters
    uint64_t total_clusters = data_sectors / bpb->BPB_SecPerClus;

    // Each FAT32 entry is four bytes
    uint64_t fat_entries = (uint64_t)bpb->BPB_FATSz32 * bpb->BPB_BytsPerSec / 4;

    printf("Root Cluster: %" PRIu32 "\n", bpb->BPB_RootClus);
    printf("Bytes Per Sector: %" PRIu16 "\n", bpb->BPB_BytsPerSec);
    printf("Sectors Per Cluster: %" PRIu8 "\n", bpb->BPB_SecPerClus);
    printf("Total Clusters in Data Region: %" PRIu64 "\n", total_clusters);
    printf("Entries in One FAT: %" PRIu64 "\n", fat_entries);
    printf("Size of Image (bytes): %" PRIu64 "\n", fat32->image_size);
}

// Close the image and clear its file pointer
void close_image(FAT32 *fat32) {
    if (fat32->image != NULL) {
        fclose(fat32->image);
        fat32->image = NULL;
    }
}

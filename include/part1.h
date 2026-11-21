#ifndef PART1_H
#define PART1_H

#include <stdio.h>
#include <stdint.h>

// FAT32 BIOS Parameter Block (BPB)
// Store info about the layout of FAT32 Image
/*
    uint8_t --> 1 byte
    uint16_t --> 2 byte
    uint32_t --> 4 byte
*/
typedef struct __attribute__((packed)) {
    uint8_t  BS_jmpBoot[3];     // Jump instruction for booting
    uint8_t  BS_OEMName[8];     // Name of the system that formatted the volume
    uint16_t BPB_BytsPerSec;    // Number of bytes per sector
    uint8_t  BPB_SecPerClus;    // Number of sectors per cluster
    uint16_t BPB_RsvdSecCnt;    // Number of reserved sectors
    uint8_t  BPB_NumFATs;       // Number of FAT tables
    uint16_t BPB_RootEntCnt;    // Root directory entries (0 for FAT32)
    uint16_t BPB_TotSec16;      // Total sectors for FAT12/16 (0 for FAT32)
    uint8_t  BPB_Media;         // Type of storage media
    uint16_t BPB_FATSz16;       // FAT size for FAT12/16 (0 for FAT32)
    uint16_t BPB_SecPerTrk;     // Number of sectors per track
    uint16_t BPB_NumHeads;      // Number of disk heads
    uint32_t BPB_HiddSec;       // Number of hidden sectors
    uint32_t BPB_TotSec32;      // Total number of sectors in the volume
    uint32_t BPB_FATSz32;       // Number of sectors in one FAT
    uint16_t BPB_ExtFlags;      // FAT mirroring and active FAT settings
    uint16_t BPB_FSVer;         // FAT32 version number
    uint32_t BPB_RootClus;      // Starting cluster of the root directory
} BPB;


// Stores info about currently opened FAT32 image
typedef struct {
    FILE *image;                // Pointer to the opened image file
    BPB bpb;                    // Boot sector info
    uint32_t current_cluster;   // Current directory cluster
    uint64_t image_size;        // Total image size in bytes
} FAT32;


// Part 1 functions
int open_image(FAT32 *fat32, const char *filename);
void command_info(const FAT32 *fat32);
void close_image(FAT32 *fat32);

#endif 

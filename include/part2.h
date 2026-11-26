#ifndef PART2_H
#define PART2_H

#include <stdbool.h>
#include "part1.h"

typedef struct {
    char name[13];  // 8-character base, period, 3-character extension, terminator
    uint32_t cluster;
    bool is_directory;
} DirectoryEntry;

typedef struct {
    DirectoryEntry *items;
    size_t count;
    size_t capacity;
} EntryList;

// The path holds each directory below root, starting with the outermost.
void print_prompt(const char *image_name, const EntryList *path);
void command_ls(const FAT32 *fat32);
void command_cd(FAT32 *fat32, EntryList *path, const char *dirname);

#endif

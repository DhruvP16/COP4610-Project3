#include "part2.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

enum {
    ENTRY_BYTES = 32,
    ATTR_VOLUME_LABEL = 0x08,
    ATTR_DIRECTORY = 0x10,
    FAT32_MASK = 0x0fffffff,
    FAT32_RESERVED = 0x0ffffff0,
    FAT32_END = 0x0ffffff8
};

static uint16_t read_le16(const uint8_t *bytes) {
    return bytes[0] | ((uint16_t)bytes[1] << 8);
}

static uint32_t read_le32(const uint8_t *bytes) {
    return read_le16(bytes) | ((uint32_t)read_le16(bytes + 2) << 16);
}

static bool append_entry(EntryList *list, DirectoryEntry entry) {
    if (list->count == list->capacity) {
        if (list->capacity > SIZE_MAX / sizeof(entry) / 2) {
            return false;
        }
        size_t capacity = list->capacity ? list->capacity * 2 : 8;
        DirectoryEntry *items = realloc(list->items, capacity * sizeof(entry));
        if (items == NULL) {
            return false;
        }
        list->items = items;
        list->capacity = capacity;
    }
    list->items[list->count++] = entry;
    return true;
}

// Return the length copied after removing space padding from an 8.3 name field.
static size_t copy_name_field(char *out, const uint8_t *field, size_t size, bool lower) {
    while (size > 0 && field[size - 1] == ' ') {
        size--;
    }
    for (size_t i = 0; i < size; i++) {
        out[i] = lower ? tolower(field[i]) : field[i];
    }
    return size;
}

static DirectoryEntry decode_entry(const uint8_t record[ENTRY_BYTES]) {
    DirectoryEntry entry = {0};
    size_t length = copy_name_field(entry.name, record, 8, record[12] & 0x08);
    if (record[0] == 0x05) {
        entry.name[0] = (char)0xe5;  // Escaped first byte in a live short name.
    }
    if (record[8] != ' ') {
        entry.name[length++] = '.';
        length += copy_name_field(entry.name + length, record + 8, 3, record[12] & 0x10);
    }
    entry.name[length] = '\0';
    entry.is_directory = (record[11] & ATTR_DIRECTORY) != 0;
    entry.cluster = (((uint32_t)read_le16(record + 20) << 16) | read_le16(record + 26)) & FAT32_MASK;
    return entry;
}

static uint64_t first_data_sector(const BPB *bpb) {
    return bpb->BPB_RsvdSecCnt + (uint64_t)bpb->BPB_NumFATs * bpb->BPB_FATSz32;
}

static uint32_t data_cluster_count(const BPB *bpb) {
    return (bpb->BPB_TotSec32 - first_data_sector(bpb)) / bpb->BPB_SecPerClus;
}

static bool valid_cluster(uint32_t cluster, uint32_t count) {
    return cluster >= 2 && cluster < FAT32_RESERVED && cluster - 2 < count;
}

static const char *read_cluster_entries(FILE *image, uint64_t offset, uint32_t cluster_bytes,
                                        EntryList *entries, bool *end) {
    if (fseek(image, (long)offset, SEEK_SET) != 0) {
        return "could not seek to directory";
    }
    for (uint32_t pos = 0; pos < cluster_bytes; pos += ENTRY_BYTES) {
        uint8_t record[ENTRY_BYTES];
        if (fread(record, 1, sizeof(record), image) != sizeof(record)) {
            return "could not read directory entry";
        }
        if (record[0] == 0x00) {
            *end = true;
            break;
        }
        // Long-name records (attribute 0x0f) also have the volume-label bit set.
        if (record[0] == 0xe5 || (record[11] & ATTR_VOLUME_LABEL)) {
            continue;
        }
        if (!append_entry(entries, decode_entry(record))) {
            return "out of memory reading directory";
        }
    }
    return NULL;
}

static const char *read_directory(const FAT32 *fat32, EntryList *entries) {
    const BPB *bpb = &fat32->bpb;
    uint32_t count = data_cluster_count(bpb);
    uint32_t cluster_bytes = (uint32_t)bpb->BPB_BytsPerSec * bpb->BPB_SecPerClus;
    uint64_t data_start = first_data_sector(bpb) * bpb->BPB_BytsPerSec;
    uint64_t fat_bytes = (uint64_t)bpb->BPB_FATSz32 * bpb->BPB_BytsPerSec;
    // When mirroring is disabled, the low four flag bits select the active FAT.
    unsigned active_fat = (bpb->BPB_ExtFlags & 0x80) ? (bpb->BPB_ExtFlags & 0x0f) : 0;
    if (active_fat >= bpb->BPB_NumFATs || (uint64_t)count + 2 > fat_bytes / 4 ||
        cluster_bytes % ENTRY_BYTES != 0) {
        return "invalid FAT32 directory layout";
    }
    uint64_t fat_start = (uint64_t)bpb->BPB_RsvdSecCnt * bpb->BPB_BytsPerSec + active_fat * fat_bytes;
    uint32_t cluster = fat32->current_cluster;
    if (!valid_cluster(cluster, count)) {
        return "invalid directory cluster";
    }

    uint8_t *visited = calloc(((size_t)count + 7) / 8, 1);
    if (visited == NULL) {
        return "out of memory reading directory";
    }
    const char *error = NULL;
    while (true) {
        uint32_t index = cluster - 2;  // Cluster 2 is the first data cluster.
        uint8_t mask = 1u << (index % 8);
        if (visited[index / 8] & mask) {
            error = "cycle in directory cluster chain";
            break;
        }
        visited[index / 8] |= mask;
        uint64_t offset = data_start + (uint64_t)index * cluster_bytes;
        bool end = false;
        error = read_cluster_entries(fat32->image, offset, cluster_bytes, entries, &end);
        if (error || end) {
            break;
        }

        uint8_t link[4];
        uint64_t fat_offset = fat_start + (uint64_t)cluster * sizeof(link);
        if (fseek(fat32->image, (long)fat_offset, SEEK_SET) != 0 ||
            fread(link, 1, sizeof(link), fat32->image) != sizeof(link)) {
            error = "could not read FAT entry";
            break;
        }
        cluster = read_le32(link) & FAT32_MASK;
        if (cluster >= FAT32_END) {
            break;
        }
        if (!valid_cluster(cluster, count)) {
            error = "invalid directory cluster chain";
            break;
        }
    }
    free(visited);
    return error;
}

void print_prompt(const char *image_name, const EntryList *path) {
    printf("%s/", image_name);
    for (size_t i = 0; i < path->count; i++) {
        printf("%s/", path->items[i].name);
    }
    putchar('>');
}

void command_ls(const FAT32 *fat32) {
    EntryList entries = {0};
    const char *error = read_directory(fat32, &entries);
    if (error) {
        printf("Error: %s\n", error);
    } else {
        // The FAT32 root has no on-disk dot entries.
        if (fat32->current_cluster == fat32->bpb.BPB_RootClus) {
            puts(".");
            puts("..");
        }
        for (size_t i = 0; i < entries.count; i++) {
            puts(entries.items[i].name);
        }
    }
    free(entries.items);
}

static const DirectoryEntry *find_entry(const EntryList *entries, const char *name) {
    for (size_t i = 0; i < entries->count; i++) {
        if (strcasecmp(entries->items[i].name, name) == 0) {
            return &entries->items[i];
        }
    }
    return NULL;
}

void command_cd(FAT32 *fat32, EntryList *path, const char *dirname) {
    if (strcmp(dirname, ".") == 0) {
        return;
    }
    if (strcmp(dirname, "..") == 0) {
        if (path->count > 0) {
            path->count--;
            fat32->current_cluster = path->count ? path->items[path->count - 1].cluster
                                                 : fat32->bpb.BPB_RootClus;
        }
        return;
    }

    EntryList entries = {0};
    const char *error = read_directory(fat32, &entries);
    if (error == NULL) {
        const DirectoryEntry *entry = find_entry(&entries, dirname);
        if (entry == NULL) {
            error = "directory does not exist";
        } else if (!entry->is_directory) {
            error = "not a directory";
        } else if (!valid_cluster(entry->cluster, data_cluster_count(&fat32->bpb))) {
            error = "invalid directory cluster";
        } else if (!append_entry(path, *entry)) {
            error = "out of memory changing directory";
        } else {
            fat32->current_cluster = entry->cluster;
        }
    }
    free(entries.items);
    if (error) {
        printf("Error: %s: %s\n", dirname, error);
    }
}

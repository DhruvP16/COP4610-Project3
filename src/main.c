#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "part1.h"
#include "part2.h"

int main(int argc, char *argv[]) {
    // Require exactly one FAT32 image argument
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <FAT32 image>\n", argv[0]);
        return 1;
    }

    // Open the image and load the boot sector
    FAT32 fat32;
    if (open_image(&fat32, argv[1]) != 0) {
        return 1;
    }

    // Extract only the image filename for the prompt
    const char *image_name = strrchr(argv[1], '/');
    image_name = image_name ? image_name + 1 : argv[1];
    EntryList path = {0};

    // Continue accepting commands until exit or EOF
    while (1) {
        print_prompt(image_name, &path);
        fflush(stdout);

        // Read input using the provided lexer function
        char *input = get_input();

        // Stop if the input stream has ended
        if (input == NULL || (input[0] == '\0' && feof(stdin))) {
            free(input);
            break;
        }

        // Break the input into individual words
        tokenlist *tokens = get_tokens(input);
        free(input);

        // Ignore empty input and display the prompt again
        if (tokens->size == 0) {
            free_tokens(tokens);
            continue;
        }

        // Identify which command the user entered
        if (strcmp(tokens->items[0], "info") == 0) {
            if (tokens->size != 1) {
                printf("Error: info takes no arguments\n");
            } else {
                command_info(&fat32);
            }
        }
        else if (strcmp(tokens->items[0], "ls") == 0) {
            if (tokens->size != 1) {
                printf("Error: ls takes no arguments\n");
            } else {
                command_ls(&fat32);
            }
        }
        else if (strcmp(tokens->items[0], "cd") == 0) {
            if (tokens->size != 2) {
                printf("Usage: cd DIRNAME\n");
            } else {
                command_cd(&fat32, &path, tokens->items[1]);
            }
        }
        else if (strcmp(tokens->items[0], "exit") == 0) {
            if (tokens->size != 1) {
                printf("Error: exit takes no arguments\n");
            } else {
                free_tokens(tokens);
                break;
            }
        }
        else {
            printf("Error: unknown command\n");
        }

        // Free the memory allocated for the tokens
        free_tokens(tokens);
    }

    // Close the image before terminating
    free(path.items);
    close_image(&fat32);

    return 0;
}


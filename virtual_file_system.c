#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define NAME_SIZE 51
#define COMMAND_SIZE 4096

typedef struct FreeBlock {
    int blockIndex;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct FileNode {
    char name[NAME_SIZE];
    int isDirectory;
    int *blockPointers;
    int blockCount;
    int contentSize;
    struct FileNode *parent;
    struct FileNode *children;
    struct FileNode *next;
    struct FileNode *prev;
} FileNode;

unsigned char (*virtualDisk)[BLOCK_SIZE];
FreeBlock *freeBlockHead = NULL;
FreeBlock *freeBlockTail = NULL;
FileNode *rootDirectory = NULL;
FileNode *currentDirectory = NULL;
int systemRunning = 1;

void addFreeBlock(int blockIndex) {
    FreeBlock *newBlock = malloc(sizeof(FreeBlock));

    if (newBlock == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    newBlock->blockIndex = blockIndex;
    newBlock->next = NULL;
    newBlock->prev = freeBlockTail;

    if (freeBlockHead == NULL) {
        freeBlockHead = newBlock;
        freeBlockTail = newBlock;
    } else {
        freeBlockTail->next = newBlock;
        freeBlockTail = newBlock;
    }
}

int getFreeBlock() {
    if (freeBlockHead == NULL) {
        return -1;
    }

    FreeBlock *usedBlock = freeBlockHead;
    int blockIndex = usedBlock->blockIndex;

    freeBlockHead = freeBlockHead->next;

    if (freeBlockHead == NULL) {
        freeBlockTail = NULL;
    } else {
        freeBlockHead->prev = NULL;
    }

    free(usedBlock);
    return blockIndex;
}

int countFreeBlocks() {
    int freeCount = 0;
    FreeBlock *currentBlock = freeBlockHead;

    while (currentBlock != NULL) {
        freeCount++;
        currentBlock = currentBlock->next;
    }

    return freeCount;
}

void addEntry(FileNode *directory, FileNode *newEntry) {
    newEntry->parent = directory;

    if (directory->children == NULL) {
        directory->children = newEntry;
        newEntry->next = newEntry;
        newEntry->prev = newEntry;
    } else {
        FileNode *lastEntry = directory->children->prev;

        lastEntry->next = newEntry;
        newEntry->prev = lastEntry;
        newEntry->next = directory->children;
        directory->children->prev = newEntry;
    }
}

void removeEntry(FileNode *directory, FileNode *entry) {
    if (entry->next == entry) {
        directory->children = NULL;
    } else {
        entry->prev->next = entry->next;
        entry->next->prev = entry->prev;

        if (directory->children == entry) {
            directory->children = entry->next;
        }
    }
}

FileNode *findEntry(FileNode *directory, char entryName[]) {
    if (directory->children == NULL) {
        return NULL;
    }

    FileNode *currentEntry = directory->children;

    do {
        if (strcmp(currentEntry->name, entryName) == 0) {
            return currentEntry;
        }

        currentEntry = currentEntry->next;

    } while (currentEntry != directory->children);

    return NULL;
}

void initializeSystem() {
    virtualDisk = calloc(NUM_BLOCKS, sizeof(*virtualDisk));
    rootDirectory = calloc(1, sizeof(FileNode));

    if (virtualDisk == NULL || rootDirectory == NULL) {
        printf("Memory allocation failed.\n");
        free(virtualDisk);
        free(rootDirectory);
        exit(1);
    }

    strcpy(rootDirectory->name, "/");
    rootDirectory->isDirectory = 1;
    rootDirectory->next = rootDirectory;
    rootDirectory->prev = rootDirectory;

    currentDirectory = rootDirectory;

    for (int blockNumber = 0; blockNumber < NUM_BLOCKS; blockNumber++) {
        addFreeBlock(blockNumber);
    }

    printf("Virtual File System Initialized.\n");
}

void createDirectory(char directoryName[]) {
    if (strlen(directoryName) == 0 || strlen(directoryName) >= NAME_SIZE) {
        printf("Invalid directory name.\n");
        return;
    }

    if (findEntry(currentDirectory, directoryName) != NULL) {
        printf("Name already exists.\n");
        return;
    }

    FileNode *newDirectory = calloc(1, sizeof(FileNode));

    if (newDirectory == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    strcpy(newDirectory->name, directoryName);
    newDirectory->isDirectory = 1;

    addEntry(currentDirectory, newDirectory);

    printf("Directory created successfully.\n");
}

void createFile(char fileName[]) {
    if (strlen(fileName) == 0 || strlen(fileName) >= NAME_SIZE) {
        printf("Invalid file name.\n");
        return;
    }

    if (findEntry(currentDirectory, fileName) != NULL) {
        printf("Name already exists.\n");
        return;
    }

    FileNode *newFile = calloc(1, sizeof(FileNode));

    if (newFile == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    strcpy(newFile->name, fileName);
    newFile->isDirectory = 0;

    addEntry(currentDirectory, newFile);

    printf("File created successfully.\n");
}

void writeFile(char fileName[], char content[]) {
    FileNode *file = findEntry(currentDirectory, fileName);

    if (file == NULL || file->isDirectory) {
        printf("File not found.\n");
        return;
    }

    int contentSize = strlen(content);
    int requiredBlocks = (contentSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int availableBlocks = countFreeBlocks() + file->blockCount;

    if (requiredBlocks > availableBlocks) {
        printf("Not enough disk space.\n");
        return;
    }

    int *newBlockPointers = NULL;

    if (requiredBlocks > 0) {
        newBlockPointers = malloc(requiredBlocks * sizeof(int));

        if (newBlockPointers == NULL) {
            printf("Memory allocation failed.\n");
            return;
        }
    }

    for (int blockNumber = 0; blockNumber < file->blockCount; blockNumber++) {
        addFreeBlock(file->blockPointers[blockNumber]);
    }

    free(file->blockPointers);

    for (int blockNumber = 0; blockNumber < requiredBlocks; blockNumber++) {
        newBlockPointers[blockNumber] = getFreeBlock();
    }

    int copiedBytes = 0;

    for (int blockNumber = 0; blockNumber < requiredBlocks; blockNumber++) {
        int bytesToCopy = contentSize - copiedBytes;

        if (bytesToCopy > BLOCK_SIZE) {
            bytesToCopy = BLOCK_SIZE;
        }

        memcpy(virtualDisk[newBlockPointers[blockNumber]],
               content + copiedBytes, bytesToCopy);

        copiedBytes += bytesToCopy;
    }

    file->blockPointers = newBlockPointers;
    file->blockCount = requiredBlocks;
    file->contentSize = contentSize;

    printf("Data written successfully.\n");
}

void readFile(char fileName[]) {
    FileNode *file = findEntry(currentDirectory, fileName);

    if (file == NULL || file->isDirectory) {
        printf("File not found.\n");
        return;
    }

    if (file->contentSize == 0) {
        printf("File is empty.\n");
        return;
    }

    int remainingBytes = file->contentSize;

    for (int blockNumber = 0; blockNumber < file->blockCount; blockNumber++) {
        int bytesToPrint = remainingBytes;

        if (bytesToPrint > BLOCK_SIZE) {
            bytesToPrint = BLOCK_SIZE;
        }

        fwrite(virtualDisk[file->blockPointers[blockNumber]],
               1, bytesToPrint, stdout);

        remainingBytes -= bytesToPrint;
    }

    printf("\n");
}

void deleteFile(char fileName[]) {
    FileNode *file = findEntry(currentDirectory, fileName);

    if (file == NULL || file->isDirectory) {
        printf("File not found.\n");
        return;
    }

    for (int blockNumber = 0; blockNumber < file->blockCount; blockNumber++) {
        addFreeBlock(file->blockPointers[blockNumber]);
    }

    removeEntry(currentDirectory, file);

    free(file->blockPointers);
    free(file);

    printf("File deleted successfully.\n");
}

void removeDirectory(char directoryName[]) {
    FileNode *directory = findEntry(currentDirectory, directoryName);

    if (directory == NULL || !directory->isDirectory) {
        printf("Directory not found.\n");
        return;
    }

    if (directory->children != NULL) {
        printf("Directory is not empty.\n");
        return;
    }

    removeEntry(currentDirectory, directory);
    free(directory);

    printf("Directory removed successfully.\n");
}

void listDirectory() {
    if (currentDirectory->children == NULL) {
        printf("(empty)\n");
        return;
    }

    FileNode *currentEntry = currentDirectory->children;

    do {
        if (currentEntry->isDirectory) {
            printf("[DIR] %s\n", currentEntry->name);
        } else {
            printf("[FILE] %s\n", currentEntry->name);
        }

        currentEntry = currentEntry->next;

    } while (currentEntry != currentDirectory->children);
}

void changeDirectory(char directoryName[]) {
    if (strcmp(directoryName, "..") == 0) {
        if (currentDirectory->parent != NULL) {
            currentDirectory = currentDirectory->parent;
        }
        return;
    }

    FileNode *directory = findEntry(currentDirectory, directoryName);

    if (directory == NULL || !directory->isDirectory) {
        printf("Directory not found.\n");
        return;
    }

    currentDirectory = directory;
}

void printPath(FileNode *directory) {
    if (directory == rootDirectory) {
        printf("/");
        return;
    }

    printPath(directory->parent);

    if (directory->parent != rootDirectory) {
        printf("/");
    }

    printf("%s", directory->name);
}

void showCurrentPath() {
    printPath(currentDirectory);
    printf("\n");
}

void showDiskUsage() {
    int freeCount = countFreeBlocks();
    int usedCount = NUM_BLOCKS - freeCount;
    double usagePercentage = (double)usedCount * 100 / NUM_BLOCKS;

    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", usedCount);
    printf("Free Blocks: %d\n", freeCount);
    printf("Disk Usage: %.2f%%\n", usagePercentage);
}

void freeDirectoryTree(FileNode *directory) {
    if (directory->children == NULL) {
        return;
    }

    FileNode *firstEntry = directory->children;
    FileNode *currentEntry = firstEntry;

    do {
        FileNode *nextEntry = currentEntry->next;

        if (currentEntry->isDirectory) {
            freeDirectoryTree(currentEntry);
        } else {
            free(currentEntry->blockPointers);
        }

        free(currentEntry);
        currentEntry = nextEntry;

    } while (currentEntry != firstEntry);
}

void cleanupSystem() {
    freeDirectoryTree(rootDirectory);
    free(rootDirectory);

    while (freeBlockHead != NULL) {
        FreeBlock *currentBlock = freeBlockHead;
        freeBlockHead = freeBlockHead->next;
        free(currentBlock);
    }

    free(virtualDisk);

    printf("Memory released. Exiting program...\n");
}

void runSystem() {
    char commandLine[COMMAND_SIZE];
    char command[NAME_SIZE];
    char entryName[NAME_SIZE];

    while (systemRunning) {
        printPath(currentDirectory);
        printf(" > ");

        if (fgets(commandLine, sizeof(commandLine), stdin) == NULL) {
            break;
        }

        commandLine[strcspn(commandLine, "\n")] = '\0';

        if (sscanf(commandLine, "%50s", command) != 1) {
            continue;
        }

        if (strcmp(command, "mkdir") == 0) {
            if (sscanf(commandLine, "%*s %50s", entryName) == 1) {
                createDirectory(entryName);
            } else {
                printf("Directory name required.\n");
            }
        }

        else if (strcmp(command, "create") == 0) {
            if (sscanf(commandLine, "%*s %50s", entryName) == 1) {
                createFile(entryName);
            } else {
                printf("File name required.\n");
            }
        }

        else if (strcmp(command, "write") == 0) {
            char *content = commandLine;

            while (*content != '\0' && *content != ' ') content++;
            while (*content == ' ') content++;
            while (*content != '\0' && *content != ' ') content++;
            while (*content == ' ') content++;

            if (sscanf(commandLine, "%*s %50s", entryName) == 1 && *content != '\0') {
                if (*content == '"') {
                    content++;
                    char *lastQuote = strrchr(content, '"');

                    if (lastQuote != NULL) {
                        *lastQuote = '\0';
                    }
                }

                writeFile(entryName, content);
            } else {
                printf("Use: write filename content\n");
            }
        }

        else if (strcmp(command, "read") == 0) {
            if (sscanf(commandLine, "%*s %50s", entryName) == 1) {
                readFile(entryName);
            } else {
                printf("File name required.\n");
            }
        }

        else if (strcmp(command, "delete") == 0) {
            if (sscanf(commandLine, "%*s %50s", entryName) == 1) {
                deleteFile(entryName);
            } else {
                printf("File name required.\n");
            }
        }

        else if (strcmp(command, "rmdir") == 0) {
            if (sscanf(commandLine, "%*s %50s", entryName) == 1) {
                removeDirectory(entryName);
            } else {
                printf("Directory name required.\n");
            }
        }

        else if (strcmp(command, "ls") == 0) {
            listDirectory();
        }

        else if (strcmp(command, "cd") == 0) {
            if (sscanf(commandLine, "%*s %50s", entryName) == 1) {
                changeDirectory(entryName);
            } else {
                printf("Directory name required.\n");
            }
        }

        else if (strcmp(command, "pwd") == 0) {
            showCurrentPath();
        }

        else if (strcmp(command, "df") == 0) {
            showDiskUsage();
        }

        else if (strcmp(command, "exit") == 0) {
            systemRunning = 0;
        }

        else {
            printf("Unknown command.\n");
        }
    }
}

int main() {
    initializeSystem();
    runSystem();
    cleanupSystem();
    return 0;
}

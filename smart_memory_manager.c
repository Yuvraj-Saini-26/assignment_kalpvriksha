#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POOL_SIZE 1000000
#define INITIAL_CAPACITY 4
#define FREE_ID -1
#define NOT_FOUND -1
#define INPUT_BUFFER_SIZE 64

enum Strategy {
    FIRST_FIT = 1,
    BEST_FIT,
    WORST_FIT
};

struct Block {
    int start;
    int size;
    int id;
    bool isAllocated;
};

static char *memory = NULL;
static struct Block *blocks = NULL;
static int poolSize = 0;
static int blockCount = 0;
static int blockCapacity = 0;
static int nextId = 1;
static enum Strategy strategy = FIRST_FIT;

static void releaseResources(void) {
    free(memory);
    free(blocks);
    memory = NULL;
    blocks = NULL;
}

/*
 * Reads one line with fgets and parses it with strtol.
 * Rejects empty input, trailing garbage (e.g. "5abc"), overflow and
 * values outside [min, max]. At EOF the program exits cleanly.
 */
static int readIntInRange(const char *prompt, int min, int max) {
    char line[INPUT_BUFFER_SIZE];

    while (true) {
        printf("%s", prompt);

        if (fgets(line, sizeof line, stdin) == NULL) {
            printf("\nEnd of input. Exiting.\n");
            releaseResources();
            exit(0);
        }

        size_t length = strlen(line);
        bool hasNewline = length > 0 && line[length - 1] == '\n';
        bool isTooLong = !hasNewline && length == sizeof line - 1;

        if (isTooLong) {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF);
        } else {
            char *end;

            errno = 0;
            long value = strtol(line, &end, 10);

            if (end != line && errno == 0) {
                while (isspace((unsigned char)*end)) {
                    end++;
                }

                if (*end == '\0' && value >= min && value <= max) {
                    return (int)value;
                }
            }
        }

        printf("Invalid input!\n");
    }
}

static bool growIfFull(void) {
    if (blockCount < blockCapacity) {
        return true;
    }

    struct Block *newBlocks = realloc(blocks, blockCapacity * 2 * sizeof(struct Block));

    if (newBlocks == NULL) {
        return false;
    }

    blocks = newBlocks;
    blockCapacity = blockCapacity * 2;
    return true;
}

static void insertFreeBlock(int position, int start, int size) {
    memmove(&blocks[position + 1], &blocks[position],
            (blockCount - position) * sizeof(struct Block));

    blocks[position].start = start;
    blocks[position].size = size;
    blocks[position].id = FREE_ID;
    blocks[position].isAllocated = false;
    blockCount++;
}

static void removeBlock(int position) {
    memmove(&blocks[position], &blocks[position + 1],
            (blockCount - position - 1) * sizeof(struct Block));

    blockCount--;
}

static void mergeFreeBlocks(void) {
    int blockNo = 0;

    while (blockNo < blockCount - 1) {
        if (!blocks[blockNo].isAllocated && !blocks[blockNo + 1].isAllocated) {
            blocks[blockNo].size += blocks[blockNo + 1].size;
            removeBlock(blockNo + 1);
        } else {
            blockNo++;
        }
    }
}

static int findBlockById(int id) {
    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (blocks[blockNo].isAllocated && blocks[blockNo].id == id) {
            return blockNo;
        }
    }

    return NOT_FOUND;
}

static int findFirstFit(int size) {
    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated && blocks[blockNo].size >= size) {
            return blockNo;
        }
    }

    return NOT_FOUND;
}

static int findBestFit(int size) {
    int bestNo = NOT_FOUND;

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated && blocks[blockNo].size >= size) {
            if (bestNo == NOT_FOUND || blocks[blockNo].size < blocks[bestNo].size) {
                bestNo = blockNo;
            }
        }
    }

    return bestNo;
}

static int findWorstFit(int size) {
    int worstNo = NOT_FOUND;

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated && blocks[blockNo].size >= size) {
            if (worstNo == NOT_FOUND || blocks[blockNo].size > blocks[worstNo].size) {
                worstNo = blockNo;
            }
        }
    }

    return worstNo;
}

static int findFreeBlockForStrategy(int size) {
    if (strategy == FIRST_FIT) {
        return findFirstFit(size);
    } else if (strategy == BEST_FIT) {
        return findBestFit(size);
    }

    return findWorstFit(size);
}

static void allocateAt(int blockNo, int size, int id) {
    int remaining = blocks[blockNo].size - size;

    if (remaining > 0) {
        insertFreeBlock(blockNo + 1, blocks[blockNo].start + size, remaining);
    }

    blocks[blockNo].size = size;
    blocks[blockNo].id = id;
    blocks[blockNo].isAllocated = true;
}

static int getTotalFree(void) {
    int total = 0;

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated) {
            total += blocks[blockNo].size;
        }
    }

    return total;
}

static int getLargestFree(void) {
    int largest = 0;

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated && blocks[blockNo].size > largest) {
            largest = blocks[blockNo].size;
        }
    }

    return largest;
}

static int countBlocksByState(bool isAllocated) {
    int count = 0;

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (blocks[blockNo].isAllocated == isAllocated) {
            count++;
        }
    }

    return count;
}

static int countAllocatedBlocks(void) {
    return countBlocksByState(true);
}

static int countFreeBlocks(void) {
    return countBlocksByState(false);
}

static void reportFragmentation(int requestedSize) {
    int totalFree = getTotalFree();

    if (totalFree < requestedSize) {
        return;
    }

    printf("Note: The request for %d bytes failed even though %d bytes are free, because the free memory is fragmented.\n",
           requestedSize, totalFree);
    printf("Free block sizes:");

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated) {
            printf(" %d", blocks[blockNo].size);
        }
    }

    printf("\n");
}

static void changeStrategy(void) {
    int choice = readIntInRange(
        "Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ",
        FIRST_FIT, WORST_FIT);

    strategy = (enum Strategy)choice;

    if (strategy == FIRST_FIT) {
        printf("Strategy set to First Fit.\n");
    } else if (strategy == BEST_FIT) {
        printf("Strategy set to Best Fit.\n");
    } else {
        printf("Strategy set to Worst Fit.\n");
    }
}

static void allocateMemory(void) {
    int size = readIntInRange("Requested size: ", 1, poolSize);
    int blockNo = findFreeBlockForStrategy(size);

    if (blockNo == NOT_FOUND) {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        reportFragmentation(size);
        return;
    }

    bool needsSplit = blocks[blockNo].size > size;

    if (needsSplit && !growIfFull()) {
        printf("Allocation failed: not enough memory for the block table.\n");
        return;
    }

    int id = nextId;
    nextId++;

    allocateAt(blockNo, size, id);
    memset(memory + blocks[blockNo].start, id, size);

    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n", id, blocks[blockNo].start, size);
}

static void releaseMemory(void) {
    int id = readIntInRange("Allocation ID: ", 1, INT_MAX);
    int blockNo = findBlockById(id);

    if (blockNo == NOT_FOUND) {
        printf("Error: Allocation ID %d does not exist or is already released.\n", id);
        return;
    }

    blocks[blockNo].isAllocated = false;
    blocks[blockNo].id = FREE_ID;
    mergeFreeBlocks();

    printf("Allocation released.\n");
}

static bool shrinkBlock(int blockNo, int newSize) {
    int freedBytes = blocks[blockNo].size - newSize;
    int nextNo = blockNo + 1;

    if (nextNo < blockCount && !blocks[nextNo].isAllocated) {
        blocks[nextNo].start -= freedBytes;
        blocks[nextNo].size += freedBytes;
    } else {
        if (!growIfFull()) {
            return false;
        }

        insertFreeBlock(nextNo, blocks[blockNo].start + newSize, freedBytes);
    }

    blocks[blockNo].size = newSize;
    return true;
}

static bool expandInPlace(int blockNo, int newSize) {
    int extraBytes = newSize - blocks[blockNo].size;
    int nextNo = blockNo + 1;

    if (nextNo >= blockCount || blocks[nextNo].isAllocated || blocks[nextNo].size < extraBytes) {
        return false;
    }

    blocks[blockNo].size = newSize;
    blocks[nextNo].start += extraBytes;
    blocks[nextNo].size -= extraBytes;

    if (blocks[nextNo].size == 0) {
        removeBlock(nextNo);
    }

    return true;
}

static bool moveBlock(int blockNo, int newSize) {
    int targetNo = findFreeBlockForStrategy(newSize);

    if (targetNo == NOT_FOUND) {
        return false;
    }

    bool willSplit = blocks[targetNo].size > newSize;

    if (willSplit && !growIfFull()) {
        return false;
    }

    int oldStart = blocks[blockNo].start;
    int oldSize = blocks[blockNo].size;
    int id = blocks[blockNo].id;
    int newStart = blocks[targetNo].start;

    allocateAt(targetNo, newSize, id);

    if (willSplit && targetNo < blockNo) {
        blockNo++;
    }

    memmove(memory + newStart, memory + oldStart, oldSize);

    blocks[blockNo].isAllocated = false;
    blocks[blockNo].id = FREE_ID;
    return true;
}

static void resizeAllocation(void) {
    bool isResized;
    int id = readIntInRange("Allocation ID: ", 1, INT_MAX);
    int blockNo = findBlockById(id);

    if (blockNo == NOT_FOUND) {
        printf("Error: Allocation ID %d does not exist or is already released.\n", id);
        return;
    }

    int newSize = readIntInRange("New size: ", 1, poolSize);

    if (newSize == blocks[blockNo].size) {
        printf("Allocation size is unchanged.\n");
        return;
    }

    if (newSize < blocks[blockNo].size) {
        isResized = shrinkBlock(blockNo, newSize);
    } else {
        isResized = expandInPlace(blockNo, newSize) || moveBlock(blockNo, newSize);
    }

    if (!isResized) {
        printf("Resize failed: allocation %d is unchanged.\n", id);
        reportFragmentation(newSize);
        return;
    }

    mergeFreeBlocks();

    int resizedNo = findBlockById(id);
    printf("Resize successful. ID: %d, Start: %d, Size: %d\n", id, blocks[resizedNo].start, blocks[resizedNo].size);
}

static void displayMemoryMap(void) {
    printf("Memory Map (%d bytes):\n", poolSize);

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (blocks[blockNo].isAllocated) {
            printf("[Allocated #%d: %d bytes, Start: %d]\n", blocks[blockNo].id, blocks[blockNo].size, blocks[blockNo].start);
        } else {
            printf("[Free: %d bytes, Start: %d]\n", blocks[blockNo].size, blocks[blockNo].start);
        }
    }
}

static void displayStatistics(void) {
    int totalFree = getTotalFree();

    printf("Total memory: %d bytes\n", poolSize);
    printf("Allocated: %d bytes\n", poolSize - totalFree);
    printf("Free: %d bytes\n", totalFree);
    printf("Allocated blocks: %d\n", countAllocatedBlocks());
    printf("Free blocks: %d\n", countFreeBlocks());
    printf("Largest free block: %d bytes\n", getLargestFree());
}

static void showMenu(void) {
    printf("\nMenu:\n");
    printf("1. Allocate memory\n");
    printf("2. Release memory\n");
    printf("3. Resize allocation\n");
    printf("4. Display memory map\n");
    printf("5. Display memory statistics\n");
    printf("6. Change allocation strategy\n");
    printf("7. Exit\n");
}

int main(void) {
    int choice;

    poolSize = readIntInRange("Enter memory pool size: ", 1, MAX_POOL_SIZE);

    memory = malloc(poolSize);

    if (memory == NULL) {
        printf("Error: Failed to allocate the memory pool.\n");
        return 1;
    }

    blocks = malloc(INITIAL_CAPACITY * sizeof(struct Block));

    if (blocks == NULL) {
        printf("Error: Failed to allocate the block table.\n");
        releaseResources();
        return 1;
    }

    blockCapacity = INITIAL_CAPACITY;
    blockCount = 1;
    blocks[0].start = 0;
    blocks[0].size = poolSize;
    blocks[0].id = FREE_ID;
    blocks[0].isAllocated = false;

    changeStrategy();

    do {
        showMenu();
        choice = readIntInRange("\nChoice: ", 1, 7);

        switch (choice) {
            case 1:
                allocateMemory();
                break;
            case 2:
                releaseMemory();
                break;
            case 3:
                resizeAllocation();
                break;
            case 4:
                displayMemoryMap();
                break;
            case 5:
                displayStatistics();
                break;
            case 6:
                changeStrategy();
                break;
            case 7:
                printf("Goodbye!\n");
                break;
        }
    } while (choice != 7);

    releaseResources();
    return 0;
}

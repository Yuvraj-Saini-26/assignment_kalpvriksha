#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POOL_SIZE 1000000
#define INITIAL_CAPACITY 4
#define FIRST_FIT 1
#define BEST_FIT 2
#define WORST_FIT 3
#define FREE_ID -1
#define NOT_FOUND -1

struct Block {
    int start;
    int size;
    int id;
    int isAllocated;
};

char *memory = NULL;
struct Block *blocks = NULL;
int poolSize = 0;
int blockCount = 0;
int blockCapacity = 0;
int nextId = 1;
int strategy = FIRST_FIT;

int growIfFull(void) {
    if (blockCount < blockCapacity) {
        return 1;
    }

    struct Block *newBlocks = realloc(blocks, blockCapacity * 2 * sizeof(struct Block));

    if (newBlocks == NULL) {
        return 0;
    }

    blocks = newBlocks;
    blockCapacity = blockCapacity * 2;
    return 1;
}

void insertFreeBlock(int position, int start, int size) {
    for (int blockNo = blockCount; blockNo > position; blockNo--) {
        blocks[blockNo] = blocks[blockNo - 1];
    }

    blocks[position].start = start;
    blocks[position].size = size;
    blocks[position].id = FREE_ID;
    blocks[position].isAllocated = 0;
    blockCount++;
}

void removeBlock(int position) {
    for (int blockNo = position; blockNo < blockCount - 1; blockNo++) {
        blocks[blockNo] = blocks[blockNo + 1];
    }

    blockCount--;
}

void mergeFreeBlocks(void) {
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

int findBlockById(int id) {
    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (blocks[blockNo].isAllocated && blocks[blockNo].id == id) {
            return blockNo;
        }
    }

    return NOT_FOUND;
}

int findFirstFit(int size) {
    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated && blocks[blockNo].size >= size) {
            return blockNo;
        }
    }

    return NOT_FOUND;
}

int findBestFit(int size) {
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

int findWorstFit(int size) {
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

int findBlock(int size) {
    if (strategy == FIRST_FIT) {
        return findFirstFit(size);
    } else if (strategy == BEST_FIT) {
        return findBestFit(size);
    }

    return findWorstFit(size);
}

void allocateAt(int blockNo, int size, int id) {
    int remaining = blocks[blockNo].size - size;

    if (remaining > 0) {
        insertFreeBlock(blockNo + 1, blocks[blockNo].start + size, remaining);
    }

    blocks[blockNo].size = size;
    blocks[blockNo].id = id;
    blocks[blockNo].isAllocated = 1;
}

int getTotalFree(void) {
    int total = 0;

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated) {
            total += blocks[blockNo].size;
        }
    }

    return total;
}

int getLargestFree(void) {
    int largest = 0;

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (!blocks[blockNo].isAllocated && blocks[blockNo].size > largest) {
            largest = blocks[blockNo].size;
        }
    }

    return largest;
}

int countBlocks(int isAllocated) {
    int count = 0;

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (blocks[blockNo].isAllocated == isAllocated) {
            count++;
        }
    }

    return count;
}

void reportFragmentation(int requestedSize) {
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

void changeStrategy(void) {
    printf("Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ");

    while (scanf("%d", &strategy) != 1 || strategy < FIRST_FIT || strategy > WORST_FIT) {
        printf("Invalid input!\nSelect allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ");
        while (getchar() != '\n');
    }

    if (strategy == FIRST_FIT) {
        printf("Strategy set to First Fit.\n");
    } else if (strategy == BEST_FIT) {
        printf("Strategy set to Best Fit.\n");
    } else {
        printf("Strategy set to Worst Fit.\n");
    }
}

void allocateMemory(void) {
    int size;

    printf("Requested size: ");

    while (scanf("%d", &size) != 1 || size < 1 || size > poolSize) {
        printf("Invalid input!\nRequested size: ");
        while (getchar() != '\n');
    }

    int blockNo = findBlock(size);

    if (blockNo == NOT_FOUND) {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        reportFragmentation(size);
        return;
    }

    if (!growIfFull()) {
        printf("Allocation failed: not enough memory for the block table.\n");
        return;
    }

    int id = nextId;
    nextId++;

    allocateAt(blockNo, size, id);
    memset(memory + blocks[blockNo].start, id, size);

    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n", id, blocks[blockNo].start, size);
}

void releaseMemory(void) {
    int id;

    printf("Allocation ID: ");

    while (scanf("%d", &id) != 1 || id < 1) {
        printf("Invalid input!\nAllocation ID: ");
        while (getchar() != '\n');
    }

    int blockNo = findBlockById(id);

    if (blockNo == NOT_FOUND) {
        printf("Error: Allocation ID %d does not exist or is already released.\n", id);
        return;
    }

    blocks[blockNo].isAllocated = 0;
    blocks[blockNo].id = FREE_ID;
    mergeFreeBlocks();

    printf("Allocation released.\n");
}

int shrinkBlock(int blockNo, int newSize) {
    int freedBytes = blocks[blockNo].size - newSize;
    int nextNo = blockNo + 1;

    if (nextNo < blockCount && !blocks[nextNo].isAllocated) {
        blocks[nextNo].start -= freedBytes;
        blocks[nextNo].size += freedBytes;
    } else {
        if (!growIfFull()) {
            return 0;
        }

        insertFreeBlock(nextNo, blocks[blockNo].start + newSize, freedBytes);
    }

    blocks[blockNo].size = newSize;
    return 1;
}

int expandInPlace(int blockNo, int newSize) {
    int extraBytes = newSize - blocks[blockNo].size;
    int nextNo = blockNo + 1;

    if (nextNo >= blockCount || blocks[nextNo].isAllocated || blocks[nextNo].size < extraBytes) {
        return 0;
    }

    blocks[blockNo].size = newSize;
    blocks[nextNo].start += extraBytes;
    blocks[nextNo].size -= extraBytes;

    if (blocks[nextNo].size == 0) {
        removeBlock(nextNo);
    }

    return 1;
}

int moveBlock(int blockNo, int newSize) {
    int targetNo = findBlock(newSize);

    if (targetNo == NOT_FOUND || !growIfFull()) {
        return 0;
    }

    int oldStart = blocks[blockNo].start;
    int oldSize = blocks[blockNo].size;
    int id = blocks[blockNo].id;
    int newStart = blocks[targetNo].start;
    int willSplit = blocks[targetNo].size > newSize;

    allocateAt(targetNo, newSize, id);

    if (willSplit && targetNo < blockNo) {
        blockNo++;
    }

    memmove(memory + newStart, memory + oldStart, oldSize);

    blocks[blockNo].isAllocated = 0;
    blocks[blockNo].id = FREE_ID;
    return 1;
}

void resizeAllocation(void) {
    int id;
    int newSize;
    int isResized;

    printf("Allocation ID: ");

    while (scanf("%d", &id) != 1 || id < 1) {
        printf("Invalid input!\nAllocation ID: ");
        while (getchar() != '\n');
    }

    int blockNo = findBlockById(id);

    if (blockNo == NOT_FOUND) {
        printf("Error: Allocation ID %d does not exist or is already released.\n", id);
        return;
    }

    printf("New size: ");

    while (scanf("%d", &newSize) != 1 || newSize < 1 || newSize > poolSize) {
        printf("Invalid input!\nNew size: ");
        while (getchar() != '\n');
    }

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

void displayMemoryMap(void) {
    printf("Memory Map (%d bytes):\n", poolSize);

    for (int blockNo = 0; blockNo < blockCount; blockNo++) {
        if (blocks[blockNo].isAllocated) {
            printf("[Allocated #%d: %d bytes, Start: %d]\n", blocks[blockNo].id, blocks[blockNo].size, blocks[blockNo].start);
        } else {
            printf("[Free: %d bytes, Start: %d]\n", blocks[blockNo].size, blocks[blockNo].start);
        }
    }
}

void displayStatistics(void) {
    int totalFree = getTotalFree();

    printf("Total memory: %d bytes\n", poolSize);
    printf("Allocated: %d bytes\n", poolSize - totalFree);
    printf("Free: %d bytes\n", totalFree);
    printf("Allocated blocks: %d\n", countBlocks(1));
    printf("Free blocks: %d\n", countBlocks(0));
    printf("Largest free block: %d bytes\n", getLargestFree());
}

void showMenu(void) {
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
    int choice = 0;

    printf("Enter memory pool size: ");

    while (scanf("%d", &poolSize) != 1 || poolSize < 1 || poolSize > MAX_POOL_SIZE) {
        printf("Invalid input!\nEnter memory pool size: ");
        while (getchar() != '\n');
    }

    memory = malloc(poolSize);

    if (memory == NULL) {
        printf("Error: Failed to allocate the memory pool.\n");
        return 1;
    }

    blocks = malloc(INITIAL_CAPACITY * sizeof(struct Block));

    if (blocks == NULL) {
        printf("Error: Failed to allocate the block table.\n");
        free(memory);
        return 1;
    }

    blockCapacity = INITIAL_CAPACITY;
    blockCount = 1;
    blocks[0].start = 0;
    blocks[0].size = poolSize;
    blocks[0].id = FREE_ID;
    blocks[0].isAllocated = 0;

    changeStrategy();
    showMenu();

    do {
        printf("\nChoice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\n");
            while (getchar() != '\n');
            choice = 0;
            continue;
        }

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
            default:
                printf("Invalid choice! Enter a number from 1 to 7.\n");
        }
    } while (choice != 7);

    free(memory);
    free(blocks);
    return 0;
}

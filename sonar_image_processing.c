#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_SIZE 2
#define MAX_SIZE 10
#define MAX_INTENSITY 255

int **getCell(int **grid, int size, int rowNo, int columnNo) {
    return grid + rowNo * size + columnNo;
}

void generateMatrix(int *values, int **grid, int size) {
    for (int cellNo = 0; cellNo < size * size; cellNo++) {
        *(values + cellNo) = rand() % (MAX_INTENSITY + 1);
        *(grid + cellNo) = values + cellNo;
    }
}

void displayMatrix(int **grid, int size) {
    for (int rowNo = 0; rowNo < size; rowNo++) {
        for (int columnNo = 0; columnNo < size; columnNo++) {
            printf("%5d", **getCell(grid, size, rowNo, columnNo));
        }

        printf("\n");
    }
}

void rotateMatrix(int **grid, int size) {
    for (int layerNo = 0; layerNo < size / 2; layerNo++) {
        int last = size - 1 - layerNo;

        for (int offsetNo = layerNo; offsetNo < last; offsetNo++) {
            int **topCell = getCell(grid, size, layerNo, offsetNo);
            int **rightCell = getCell(grid, size, offsetNo, last);
            int **bottomCell = getCell(grid, size, last, size - 1 - offsetNo);
            int **leftCell = getCell(grid, size, size - 1 - offsetNo, layerNo);

            int *temp = *leftCell;
            *leftCell = *bottomCell;
            *bottomCell = *rightCell;
            *rightCell = *topCell;
            *topCell = temp;
        }
    }
}

void loadRow(int *destination, int **grid, int size, int rowNo) {
    for (int columnNo = 0; columnNo < size; columnNo++) {
        *(destination + columnNo) = **getCell(grid, size, rowNo, columnNo);
    }
}

void copyRow(int *destination, int *source, int size) {
    for (int columnNo = 0; columnNo < size; columnNo++) {
        *(destination + columnNo) = *(source + columnNo);
    }
}

int sumWindow(int *row, int size, int columnNo, int *count) {
    int sum = 0;

    for (int neighborNo = columnNo - 1; neighborNo <= columnNo + 1; neighborNo++) {
        if (neighborNo >= 0 && neighborNo < size) {
            sum += *(row + neighborNo);
            (*count)++;
        }
    }

    return sum;
}

void smoothMatrix(int **grid, int size) {
    int previousRow[MAX_SIZE];
    int currentRow[MAX_SIZE];
    int nextRow[MAX_SIZE];

    for (int rowNo = 0; rowNo < size; rowNo++) {
        loadRow(currentRow, grid, size, rowNo);

        if (rowNo < size - 1) {
            loadRow(nextRow, grid, size, rowNo + 1);
        }

        for (int columnNo = 0; columnNo < size; columnNo++) {
            int count = 0;
            int sum = sumWindow(currentRow, size, columnNo, &count);

            if (rowNo > 0) {
                sum += sumWindow(previousRow, size, columnNo, &count);
            }

            if (rowNo < size - 1) {
                sum += sumWindow(nextRow, size, columnNo, &count);
            }

            **getCell(grid, size, rowNo, columnNo) = sum / count;
        }

        copyRow(previousRow, currentRow, size);
    }
}

int main(void) {
    int size;
    int values[MAX_SIZE * MAX_SIZE];
    int *grid[MAX_SIZE * MAX_SIZE];

    printf("Enter matrix size (%d-%d): ", MIN_SIZE, MAX_SIZE);

    if (scanf("%d", &size) != 1 || size < MIN_SIZE || size > MAX_SIZE) {
        printf("Error: Matrix size must be between %d and %d.\n", MIN_SIZE, MAX_SIZE);
        return 1;
    }

    srand((unsigned int)time(NULL));
    generateMatrix(values, grid, size);

    printf("\nOriginal Randomly Generated Matrix:\n");
    displayMatrix(grid, size);

    rotateMatrix(grid, size);

    printf("\nMatrix after 90 degree Clockwise Rotation:\n");
    displayMatrix(grid, size);

    smoothMatrix(grid, size);

    printf("\nMatrix after Applying 3x3 Smoothing Filter:\n");
    displayMatrix(grid, size);

    return 0;
}

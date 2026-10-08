#include <stdio.h>

#define STUDENTS 100
#define SUBJECT 3

struct Student {
    int roll;
    char name[50];
    int marks[SUBJECT];
};

int totalStudents = 0;

int calculateTotal(int marks[]) {
    int total = 0;

    for (int i = 0; i < SUBJECT; i++) {
        total += marks[i];
    }

    return total;
}

float calculateAverage(int total) {
    return total / (float)SUBJECT;
}

char calculateGrade(float average) {
    if (average >= 85) {
        return 'A';
    }
    else if (average >= 70) {
        return 'B';
    }
    else if (average >= 50) {
        return 'C';
    }
    else if (average >= 35) {
        return 'D';
    }
    return 'F';
}

int starCount(char grade) {
    switch (grade) {
        case 'A':
            return 5;
        case 'B':
            return 4;
        case 'C':
            return 3;
        case 'D':
            return 2;
        default:
            return 0;
    }
}

void printRolls(struct Student students[], int index) {
    if (index == totalStudents) {
        return;
    }

    if (index > 0) {
        printf(" ");
    }

    printf("%d", students[index].roll);
    printRolls(students, index + 1);
}

int main() {
    struct Student students[STUDENTS];

    if (scanf("%d", &totalStudents) != 1 || totalStudents < 1 || totalStudents > STUDENTS) {
        printf("Error: Number of students must be between 1 and %d.\n", STUDENTS);
        return 1;
    }

    for (int i = 0; i < totalStudents; i++) {
        if (scanf("%d %49s %d %d %d",
                  &students[i].roll,
                  students[i].name,
                  &students[i].marks[0],
                  &students[i].marks[1],
                  &students[i].marks[2]) != 5) {
            printf("Error: Invalid student details.\n");
            return 1;
        }

        for (int j = 0; j < SUBJECT; j++) {
            if (students[i].marks[j] < 0 || students[i].marks[j] > 100) {
                printf("Error: Marks must be between 0 and 100.\n");
                return 1;
            }
        }
    }

    for (int i = 0; i < totalStudents; i++) {
        int total = calculateTotal(students[i].marks);
        float average = calculateAverage(total);
        char grade = calculateGrade(average);

        printf("Roll: %d\n", students[i].roll);
        printf("Name: %s\n", students[i].name);
        printf("Total: %d\n", total);
        printf("Average: %.2f\n", average);
        printf("Grade: %c\n", grade);

        if (average < 35) {
            continue;
        }

        printf("Performance: ");

        for (int j = 0; j < starCount(grade); j++) {
            printf("*");
        }

        printf("\n");
        printf("\n");
    }

    printf("List of Roll Numbers (via recursion): ");
    printRolls(students, 0);
    printf("\n");
    


    return 0;
}

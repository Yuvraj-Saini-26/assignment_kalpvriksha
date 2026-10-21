#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_EMPLOYEES 100
#define MAX_REPORTS 20
#define MAX_TEXT_LENGTH 49
#define NUMBER_LINE_SIZE 12
#define NO_MANAGER_ID 0
#define INDENT_SPACES 4

enum MenuChoice {
    ADD_EMPLOYEE = 1,
    REMOVE_EMPLOYEE = 2,
    DISPLAY_EMPLOYEE = 3,
    DISPLAY_COMPLETE = 4,
    EXIT_PROGRAM = 5
};

struct Employee {
    int id;
    char name[MAX_TEXT_LENGTH + 1];
    char designation[MAX_TEXT_LENGTH + 1];
    struct Employee *manager;
    struct Employee *reports[MAX_REPORTS];
    int reportCount;
    int isActive;
};

struct Employee employees[MAX_EMPLOYEES];

void clearInputLine(void) {
    int character = getchar();

    while (character != '\n' && character != EOF) {
        character = getchar();
    }
}

int readText(const char *prompt, char *text) {
    char line[MAX_TEXT_LENGTH + 2];

    printf("%s", prompt);

    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 0;
    }

    size_t length = strlen(line);

    if (length > 0 && line[length - 1] == '\n') {
        line[length - 1] = '\0';
        length--;
    } else if (length == sizeof(line) - 1) {
        clearInputLine();
        printf("Error: Text must be at most %d characters.\n", MAX_TEXT_LENGTH);
        return 0;
    }

    if (length == 0) {
        printf("Error: Text cannot be empty.\n");
        return 0;
    }

    strcpy(text, line);
    return 1;
}

int readNumber(const char *prompt, int *number) {
    char line[NUMBER_LINE_SIZE];
    char *end;

    printf("%s", prompt);

    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 0;
    }

    size_t length = strlen(line);

    if (length > 0 && line[length - 1] != '\n') {
        clearInputLine();
        printf("Error: Invalid number.\n");
        return 0;
    }

    long value = strtol(line, &end, 10);

    if (end == line || (*end != '\n' && *end != '\0')) {
        printf("Error: Invalid number.\n");
        return 0;
    }

    *number = (int)value;
    return 1;
}

struct Employee *findEmployee(int id) {
    for (int employeeNo = 0; employeeNo < MAX_EMPLOYEES; employeeNo++) {
        if (employees[employeeNo].isActive && employees[employeeNo].id == id) {
            return &employees[employeeNo];
        }
    }

    return NULL;
}

struct Employee *findFreeSlot(void) {
    for (int employeeNo = 0; employeeNo < MAX_EMPLOYEES; employeeNo++) {
        if (!employees[employeeNo].isActive) {
            return &employees[employeeNo];
        }
    }

    return NULL;
}

int countUnder(struct Employee *employee) {
    int count = 0;

    for (int reportNo = 0; reportNo < employee->reportCount; reportNo++) {
        count += 1 + countUnder(employee->reports[reportNo]);
    }

    return count;
}

void displayHierarchy(struct Employee *employee, int level) {
    printf("%*s%d - %s - %s\n", level * INDENT_SPACES, "", employee->id, employee->name, employee->designation);

    for (int reportNo = 0; reportNo < employee->reportCount; reportNo++) {
        displayHierarchy(employee->reports[reportNo], level + 1);
    }
}

struct Employee *findSiblingWithFewestReports(struct Employee *employee) {
    struct Employee *manager = employee->manager;
    struct Employee *bestSibling = NULL;

    for (int siblingNo = 0; siblingNo < manager->reportCount; siblingNo++) {
        struct Employee *sibling = manager->reports[siblingNo];

        if (sibling == employee) {
            continue;
        }

        if (bestSibling == NULL || sibling->reportCount < bestSibling->reportCount) {
            bestSibling = sibling;
        }
    }

    return bestSibling;
}

void removeFromManager(struct Employee *employee) {
    struct Employee *manager = employee->manager;
    int position = 0;

    while (manager->reports[position] != employee) {
        position++;
    }

    for (int reportNo = position; reportNo < manager->reportCount - 1; reportNo++) {
        manager->reports[reportNo] = manager->reports[reportNo + 1];
    }

    manager->reportCount--;
}

void moveReports(struct Employee *employee, struct Employee *newManager) {
    for (int reportNo = 0; reportNo < employee->reportCount; reportNo++) {
        struct Employee *report = employee->reports[reportNo];

        report->manager = newManager;
        newManager->reports[newManager->reportCount] = report;
        newManager->reportCount++;
    }
}

void addEmployee(void) {
    int id;
    int managerId;
    char name[MAX_TEXT_LENGTH + 1];
    char designation[MAX_TEXT_LENGTH + 1];
    struct Employee *manager = NULL;

    if (!readNumber("Enter Employee ID: ", &id)) {
        return;
    }

    if (id <= NO_MANAGER_ID) {
        printf("Error: Employee ID must be a positive number.\n");
        return;
    }

    if (findEmployee(id) != NULL) {
        printf("Error: Employee ID already exists.\n");
        return;
    }

    if (!readText("Enter Name: ", name) || !readText("Enter Designation: ", designation)) {
        return;
    }

    if (!readNumber("Enter Reporting Manager ID (0 for none): ", &managerId)) {
        return;
    }

    if (managerId != NO_MANAGER_ID) {
        manager = findEmployee(managerId);

        if (manager == NULL) {
            printf("Error: Reporting manager not found.\n");
            return;
        }

        if (manager->reportCount >= MAX_REPORTS) {
            printf("Error: Manager already has the maximum number of reports.\n");
            return;
        }
    }

    struct Employee *employee = findFreeSlot();

    if (employee == NULL) {
        printf("Error: Organization is full.\n");
        return;
    }

    employee->id = id;
    strcpy(employee->name, name);
    strcpy(employee->designation, designation);
    employee->manager = manager;
    employee->reportCount = 0;
    employee->isActive = 1;

    if (manager != NULL) {
        manager->reports[manager->reportCount] = employee;
        manager->reportCount++;
    }

    printf("Employee added successfully.\n");
}

void removeEmployee(void) {
    int id;

    if (!readNumber("Enter Employee ID to remove: ", &id)) {
        return;
    }

    struct Employee *employee = findEmployee(id);

    if (employee == NULL) {
        printf("Error: Employee not found.\n");
        return;
    }

    if (employee->manager == NULL) {
        printf("Error: Top-level employee (CEO) cannot be removed.\n");
        return;
    }

    struct Employee *newManager = findSiblingWithFewestReports(employee);

    if (newManager == NULL) {
        newManager = employee->manager;
    }

    int spaceAvailable = MAX_REPORTS - newManager->reportCount;

    if (newManager == employee->manager) {
        spaceAvailable++;
    }

    if (employee->reportCount > spaceAvailable) {
        printf("Error: Not enough space to transfer reports to %s.\n", newManager->name);
        return;
    }

    removeFromManager(employee);
    moveReports(employee, newManager);

    if (employee->reportCount > 0) {
        printf("%d report(s) transferred to %d - %s.\n", employee->reportCount, newManager->id, newManager->name);
    }

    employee->isActive = 0;
    printf("Employee removed successfully.\n");
}

void displayEmployeeHierarchy(void) {
    int id;

    if (!readNumber("Enter Employee ID: ", &id)) {
        return;
    }

    struct Employee *employee = findEmployee(id);

    if (employee == NULL) {
        printf("Error: Employee not found.\n");
        return;
    }

    printf("\n");
    displayHierarchy(employee, 0);
    printf("\n");
    printf("Total Employees Under %d: %d\n", employee->id, countUnder(employee));
}

void displayCompleteHierarchy(void) {
    int total = 0;

    printf("\n===== ORGANIZATION HIERARCHY =====\n\n");

    for (int employeeNo = 0; employeeNo < MAX_EMPLOYEES; employeeNo++) {
        struct Employee *employee = &employees[employeeNo];

        if (employee->isActive && employee->manager == NULL) {
            displayHierarchy(employee, 0);
            total += 1 + countUnder(employee);
        }
    }

    printf("\n");
    printf("Total Employees: %d\n", total);
}

void showMenu(void) {
    printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n\n");
    printf("1. Add Employee\n");
    printf("2. Remove Employee\n");
    printf("3. Display Employee Hierarchy\n");
    printf("4. Display Complete Hierarchy\n");
    printf("5. Exit\n\n");
}

int main(void) {
    int choice = 0;

    do {
        showMenu();

        if (!readNumber("Enter your choice: ", &choice)) {
            if (feof(stdin)) {
                break;
            }

            choice = 0;
            continue;
        }

        switch (choice) {
            case ADD_EMPLOYEE:
                addEmployee();
                break;
            case REMOVE_EMPLOYEE:
                removeEmployee();
                break;
            case DISPLAY_EMPLOYEE:
                displayEmployeeHierarchy();
                break;
            case DISPLAY_COMPLETE:
                displayCompleteHierarchy();
                break;
            case EXIT_PROGRAM:
                printf("Goodbye!\n");
                break;
            default:
                printf("Error: Invalid choice. Please try again.\n");
        }
    } while (choice != EXIT_PROGRAM);

    return 0;
}

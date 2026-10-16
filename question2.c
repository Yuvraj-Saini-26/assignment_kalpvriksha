#include <stdio.h>
#include <string.h>

struct user {
    int id;
    char name[60];
    int age;
};

void replaceSpaces(char *text) {
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == ' ') {
            text[i] = '_';
        }
    }
}

int checkId(int id) {
    struct user user;

    FILE *fp = fopen("users.txt", "r");

    if (fp == NULL) {
        printf("Error: Unable to open file.\n");
        return 0;
    }

    while (fscanf(fp, "%d %59s %d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

void createUser() {
    struct user user;

    printf("User's ID: ");
    scanf("%d", &user.id);

    if (checkId(user.id) == 1) {
        printf("This ID already exists.\n");
        return;
    }

    printf("User's Name: ");
    scanf(" %59[^\n]", user.name);
    replaceSpaces(user.name);

    printf("User's Age: ");
    scanf("%d", &user.age);

    FILE *fp = fopen("users.txt", "a");

    if (fp == NULL) {
        printf("Error: Unable to open file.\n");
        return;
    }

    fprintf(fp, "%d %s %d\n", user.id, user.name, user.age);

    fclose(fp);

    printf("User created successfully.\n");
}

void readUsers() {
    struct user user;

    FILE *fp = fopen("users.txt", "r");

    if (fp == NULL) {
        printf("Error: Unable to open file.\n");
        return;
    }

    printf("\n%-10s %-15s %-5s\n", "ID", "Name", "Age");
    printf("\n");

    while (fscanf(fp, "%d %59s %d", &user.id, user.name, &user.age) == 3) {
        printf("%-10d %-15s %-5d\n", user.id, user.name, user.age);
    }

    fclose(fp);
}

void deleteUser() {
    struct user user;
    int id;

    printf("User's ID to delete: ");
    scanf("%d", &id);

    if (checkId(id) == 0) {
        printf("ID does not exist.\n");
        return;
    }

    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL) {
        printf("Error: Unable to open file.\n");

        if (fp != NULL)
            fclose(fp);

        if (temp != NULL)
            fclose(temp);

        return;
    }

    while (fscanf(fp, "%d %59s %d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            continue;
        }

        fprintf(temp, "%d %s %d\n", user.id, user.name, user.age);
    }

    fclose(fp);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    printf("User deleted successfully.\n");
}

void updateUser() {
    struct user user;
    int id;
    char newName[60];
    int newAge;

    printf("User's ID to update: ");
    scanf("%d", &id);

    if (checkId(id) == 0) {
        printf("ID does not exist.\n");
        return;
    }

    printf("Enter new name: ");
    scanf(" %59[^\n]", newName);
    replaceSpaces(newName);

    printf("Enter new age: ");
    scanf("%d", &newAge);

    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL) {
        printf("Error: Unable to open file.\n");

        if (fp != NULL)
            fclose(fp);

        if (temp != NULL)
            fclose(temp);

        return;
    }

    while (fscanf(fp, "%d %59s %d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            fprintf(temp, "%d %s %d\n", user.id, newName, newAge);
        } else {
            fprintf(temp, "%d %s %d\n", user.id, user.name, user.age);
        }
    }

    fclose(fp);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    printf("User updated successfully.\n");
}

int main() {
    FILE *fp = fopen("users.txt", "a");

    if (fp == NULL) {
        printf("Error: Unable to open file.\n");
        return 1;
    }

    fclose(fp);

    int choice;

    do {
        printf("\nPlease make a choice:\n");
        printf("1. Add User\n");
        printf("2. Display Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input, exiting.\n");
            return 1;
        }

        switch (choice) {
            case 1:
                createUser();
                break;
            case 2:
                readUsers();
                break;
            case 3:
                updateUser();
                break;
            case 4:
                deleteUser();
                break;
            case 5:
                printf("Goodbye!\n");
                break;
            default:
                printf("Invalid choice, try again.\n");
        }
    } while (choice != 5);

    return 0;
}

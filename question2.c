#include <stdio.h>
#include <string.h>

struct user{
    int id;
    char name[60];
    int age;
};

int checkId(int id) {
    struct user insaan;

    FILE *fp = fopen("users.txt", "r");

    while (fscanf(fp, "%d %s %d",&insaan.id,insaan.name,&insaan.age) == 3) {

        if (insaan.id == id) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

void create() {
    struct user insaan;

    printf("User's ID: ");
    scanf("%d", &insaan.id);

    if (checkId(insaan.id) == 1) {
        printf("This ID already exist :(\n");
        return;
    }

    printf("User's Name (instead of spaces use '_' eg: ram_dua ): ");
    scanf("%59s", insaan.name);

    printf("User's Age: ");
    scanf("%d", &insaan.age);

    FILE *fp = fopen("users.txt", "a");

    fprintf(fp, "%d %s %d\n", insaan.id, insaan.name, insaan.age);

    fclose(fp);

    printf("User created successfully :)\n");
}

void read() {
    struct user insaan;

    FILE *fp = fopen("users.txt", "r");

    printf("\n%-10s %-15s %-5s\n", "ID", "Name", "Age");
    printf("\n");

    while (fscanf(fp, "%d %s %d",&insaan.id,insaan.name,&insaan.age) == 3) {
        printf("%-10d %-15s %-5d\n",insaan.id,insaan.name,insaan.age);
    }

    fclose(fp);
}

void delete() {
    struct user insaan;
    int id;

    printf("User's ID to be delete: ");
    scanf("%d", &id);

    if (checkId(id) == 0) {
        printf("ID does not exist :(\n");
        return;
    }

    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL) {
        printf("Pls try again File didn't responded :(\n");

        if (fp != NULL)
            fclose(fp);

        if (temp != NULL)
            fclose(temp);

        return;
    }

    while (fscanf(fp, "%d %s %d",&insaan.id,insaan.name,&insaan.age) == 3) {

        if (insaan.id == id) {
            continue;
        }

        fprintf(temp, "%d %s %d\n",insaan.id,insaan.name,insaan.age);
    }

    fclose(fp);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    printf("User deleted successfully :)\n");
}

void update() {
    struct user insaan;
    int id;
    char newName[60];
    int newAge;

    printf("User ID to update: ");
    scanf("%d", &id);

    if (checkId(id) == 0) {
        printf("ID does not exist :(\n");
        return;
    }

    printf("Enter new name (instead of spaces use '_' eg: ram_dua ): ");
    scanf("%59s", newName);

    printf("Enter new age: ");
    scanf("%d", &newAge);

    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL) {
        printf("File open nahi hui\n");

        if (fp != NULL)
            fclose(fp);

        if (temp != NULL)
            fclose(temp);

        return;
    }

    while (fscanf(fp, "%d %s %d",&insaan.id,insaan.name,&insaan.age) == 3) {

        if (insaan.id == id) {
            fprintf(temp, "%d %s %d\n",insaan.id,newName,newAge);
        }
        else {
            fprintf(temp, "%d %s %d\n",insaan.id,insaan.name,insaan.age);
        }
    }

    fclose(fp);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    printf("Update successful :)\n");
}

int main(){
    FILE *fp = fopen("users.txt", "a");
    fclose(fp);
    int choice;
    do{
        printf("\nPls make a choice :)\n");
        printf("1. Add User\n");
        printf("2. Display Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice){
            case 1:
                create();
                break;
            case 2:
                read();
                break;
            case 3:
                update();
                break;
            case 4:
                delete();
                break;
            case 5:
                printf("Bye! Hope to see u sonn :) \n");
                break;
            default:
                printf("Invalid choice, try again\n");
        }
    } while(choice != 5);
    
}

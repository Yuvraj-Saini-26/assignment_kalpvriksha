#include <stdio.h>
#include <string.h>

#define MIN_TICKETS 1
#define MAX_TICKETS 20
#define MAX_NAME_LENGTH 49
#define CATEGORY_COUNT 3
#define DISCOUNT_MIN_TICKETS 5
#define DISCOUNT_PERCENT 10.0
#define PERCENT_BASE 100.0
#define STANDARD_PRICE 150.0
#define PREMIUM_PRICE 250.0
#define RECLINER_PRICE 400.0

enum TicketCategory {
    STANDARD = 1,
    PREMIUM = 2,
    RECLINER = 3
};

struct Category {
    const char *name;
    double price;
};

struct Category getCategory(int categoryChoice) {
    struct Category category;

    switch (categoryChoice) {
        case STANDARD:
            category.name = "Standard";
            category.price = STANDARD_PRICE;
            break;
        case PREMIUM:
            category.name = "Premium";
            category.price = PREMIUM_PRICE;
            break;
        case RECLINER:
            category.name = "Recliner";
            category.price = RECLINER_PRICE;
            break;
        default:
            category.name = "Invalid";
            category.price = 0;
            break;
    }

    return category;
}

double calculateDiscount(double subtotal, int ticketCount) {
    if (ticketCount >= DISCOUNT_MIN_TICKETS) {
        return subtotal * DISCOUNT_PERCENT / PERCENT_BASE;
    } else {
        return 0;
    }
}

void showCategories(void) {
    printf("%-8s %-12s %s\n", "Choice", "Category", "Price per Ticket");

    for (int categoryChoice = STANDARD; categoryChoice <= CATEGORY_COUNT; categoryChoice++) {
        struct Category category = getCategory(categoryChoice);
        printf("%-8d %-12s %.0f\n", categoryChoice, category.name, category.price);
    }

    printf("\n");
}

int main(void) {
    char customerName[MAX_NAME_LENGTH + 2];
    int ticketCount;
    int categoryChoice;

    showCategories();

    printf("Enter customer name: ");
    if (fgets(customerName, sizeof(customerName), stdin) == NULL) {
        printf("Error: Invalid customer name.\n");
        return 1;
    }

    size_t nameLength = strlen(customerName);

    if (nameLength > 0 && customerName[nameLength - 1] == '\n') {
        customerName[nameLength - 1] = '\0';
        nameLength--;
    } else if (nameLength == sizeof(customerName) - 1) {
        printf("Error: Customer name must be at most %d characters.\n", MAX_NAME_LENGTH);
        return 1;
    }

    if (nameLength == 0) {
        printf("Error: Customer name cannot be empty.\n");
        return 1;
    }

    printf("Enter number of tickets (%d-%d): ", MIN_TICKETS, MAX_TICKETS);
    if (scanf("%d", &ticketCount) != 1 ||
        ticketCount < MIN_TICKETS ||
        ticketCount > MAX_TICKETS) {
        printf("Error: Number of tickets must be between %d and %d.\n", MIN_TICKETS, MAX_TICKETS);
        return 1;
    }

    printf("Enter ticket category (%d-%d): ", STANDARD, CATEGORY_COUNT);
    if (scanf("%d", &categoryChoice) != 1 ||
        categoryChoice < STANDARD ||
        categoryChoice > CATEGORY_COUNT) {
        printf("Error: Ticket category must be between %d and %d.\n", STANDARD, CATEGORY_COUNT);
        return 1;
    }

    struct Category category = getCategory(categoryChoice);
    double subtotal = category.price * ticketCount;
    double discount = calculateDiscount(subtotal, ticketCount);
    double finalBill = subtotal - discount;

    printf("\n");
    printf("Customer: %s\n", customerName);
    printf("Ticket Category: %s\n", category.name);
    printf("Tickets: %d\n", ticketCount);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", finalBill);

    return 0;
}

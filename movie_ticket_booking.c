#include <stdio.h>

#define MIN_TICKETS 1
#define MAX_TICKETS 20
#define CATEGORY_COUNT 3
#define DISCOUNT_MIN_TICKETS 5
#define DISCOUNT_PERCENT 10

const float STANDARD_PRICE = 150.0f;
const float PREMIUM_PRICE = 250.0f;
const float RECLINER_PRICE = 400.0f;

float getTicketPrice(int categoryChoice) {
    switch (categoryChoice) {
        case 1:
            return STANDARD_PRICE;
        case 2:
            return PREMIUM_PRICE;
        default:
            return RECLINER_PRICE;
    }
}

const char *getCategoryName(int categoryChoice) {
    switch (categoryChoice) {
        case 1:
            return "Standard";
        case 2:
            return "Premium";
        default:
            return "Recliner";
    }
}

float calculateDiscount(float subtotal, int ticketCount) {
    if (ticketCount >= DISCOUNT_MIN_TICKETS) {
        return subtotal * DISCOUNT_PERCENT / 100;
    }
    else {
        return 0;
    }
}

void showCategories() {
    printf("\n");
    printf("Choice   Category     Price per Ticket\n");
    printf("1        Standard     %.0f\n", STANDARD_PRICE);
    printf("2        Premium      %.0f\n", PREMIUM_PRICE);
    printf("3        Recliner     %.0f\n", RECLINER_PRICE);
    printf("\n");
}

int main() {
    char customerName[50];
    int ticketCount;
    int categoryChoice;

    

    printf("Enter customer name: ");
    if (scanf("%49s", customerName) != 1) {
        printf("Error: Invalid customer name.\n");
        return 1;
    }

    showCategories();

    printf("Enter number of tickets (%d-%d): ", MIN_TICKETS, MAX_TICKETS);
    if (scanf("%d", &ticketCount) != 1 || ticketCount < MIN_TICKETS || ticketCount > MAX_TICKETS) {
        printf("Error: Number of tickets must be between %d and %d.\n", MIN_TICKETS, MAX_TICKETS);
        return 1;
    }

    printf("Enter ticket category (1-%d): ", CATEGORY_COUNT);
    if (scanf("%d", &categoryChoice) != 1 || categoryChoice < 1 || categoryChoice > CATEGORY_COUNT) {
        printf("Error: Ticket category must be 1, 2 or 3.\n");
        return 1;
    }

    float subtotal = getTicketPrice(categoryChoice) * ticketCount;
    float discount = calculateDiscount(subtotal, ticketCount);
    float finalBill = subtotal - discount;

    printf("\n");
    printf("Customer: %s\n", customerName);
    printf("Ticket Category: %s\n", getCategoryName(categoryChoice));
    printf("Tickets: %d\n", ticketCount);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", finalBill);

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 100

enum CharType {
    CHAR_INVALID = -1,
    CHAR_OPERATOR = 0,
    CHAR_DIGIT = 1,
    CHAR_SPACE = 2
};

int val[MAX_SIZE];
int valTop = -1;

char opr[MAX_SIZE];
int oprTop = -1;

int classifyChar(char ch) {
    if (ch >= '0' && ch <= '9') {
        return CHAR_DIGIT;
    }
    if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
        return CHAR_OPERATOR;
    }
    if (ch == ' ') {
        return CHAR_SPACE;
    }
    return CHAR_INVALID;
}

int parser(char exp[]) {
    int expectNumber = 1;
    int spaceAfterDigit = 0;
    size_t length = strlen(exp);

    for (size_t i = 0; i < length; i++) {
        int type = classifyChar(exp[i]);

        if (type == CHAR_SPACE) {
            if (expectNumber == 0) {
                spaceAfterDigit = 1;
            }
        }
        else if (type == CHAR_DIGIT) {
            if (spaceAfterDigit == 1) {
                return 0;
            }
            expectNumber = 0;
        }
        else if (type == CHAR_OPERATOR) {
            if (expectNumber == 1) {
                return 0;
            }
            expectNumber = 1;
            spaceAfterDigit = 0;
        }
        else {
            return 0;
        }
    }

    if (expectNumber == 1) {
        return 0;
    }
    return 1;
}

int precedence(char op) {
    if (op == '*' || op == '/') {
        return 2;
    }
    return 1;
}

int calculate(int a, char op, int b) {
    if (op == '+') return a + b;
    if (op == '-') return a - b;
    if (op == '*') return a * b;
    if (op == '/' && b == 0) {
        printf("Error: Division by zero.\n");
        exit(1);
    }
    return a / b;
}

void pushValue(int value) {
    if (valTop >= MAX_SIZE - 1) {
        printf("Error: Expression is too long.\n");
        exit(1);
    }
    valTop++;
    val[valTop] = value;
}

void pushOperator(char op) {
    if (oprTop >= MAX_SIZE - 1) {
        printf("Error: Expression is too long.\n");
        exit(1);
    }
    oprTop++;
    opr[oprTop] = op;
}

void solveTop() {
    int b = val[valTop];
    valTop--;
    int a = val[valTop];
    valTop--;

    char op = opr[oprTop];
    oprTop--;

    pushValue(calculate(a, op, b));
}

int main() {
    int number = 0;
    char exp[MAX_SIZE];

    printf("Enter expression: ");

    if (fgets(exp, sizeof(exp), stdin) == NULL) {
        printf("Error: Unable to read input.\n");
        return 1;
    }

    size_t length = strlen(exp);

    if (length > 0 && exp[length - 1] == '\n') {
        exp[length - 1] = '\0';
        length--;
    }
    else if (length == sizeof(exp) - 1) {
        printf("Error: Expression is too long.\n");
        return 1;
    }

    if (parser(exp) == 0) {
        printf("Error: Invalid expression.\n");
        return 1;
    }

    size_t i = 0;
    while (i < length) {
        int type = classifyChar(exp[i]);

        if (type == CHAR_DIGIT) {
            number = number * 10 + exp[i] - '0';
            i++;
        }
        else if (type == CHAR_OPERATOR) {
            pushValue(number);
            number = 0;

            while (oprTop != -1 && precedence(opr[oprTop]) >= precedence(exp[i])) {
                solveTop();
            }

            pushOperator(exp[i]);
            i++;
        }
        else {
            i++;
        }
    }

    pushValue(number);

    while (oprTop != -1) {
        solveTop();
    }

    printf("Result = %d\n", val[valTop]);
    return 0;
}

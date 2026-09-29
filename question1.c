#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int val[100];
int valTop = -1;

int opr[100];
int oprTop = -1;

int isDigit(char ch){
    if(ch>='0' && ch <='9'){
        return 1;
    }
    else if(ch=='+' || ch=='-' || ch=='*' || ch=='/'){
        return 0;
    }
    else if(ch==' '){
        return 2;
    }
    return -1;
}

int parser(char exp[]){
    int expectNumber = 1;
    for(int i=0; i<strlen(exp); i++){
        int type = isDigit(exp[i]);
        if(type == 2){
            continue;
        }
        else if(type == 1){
            expectNumber = 0;
        }
        else if(type == 0){
            if(expectNumber == 1){
                return 0;
            }
            expectNumber = 1;
        }
        else{
            return 0;
        }
    }
    if(expectNumber == 1){
        return 0;
    }
    return 1;
}

int precedence(char op){
    if(op == '*' || op == '/'){
        return 2;
    }
    return 1;
}

int calculate(int a, char op, int b){
    if(op == '+') return a + b;
    if(op == '-') return a - b;
    if(op == '*') return a * b;
    if(op == '/' && b == 0){
        printf("Error: Division by zero :) ");
        exit(1);
    }
    return a / b;
}

void solveTop(){
    int b = val[valTop];
    valTop--;
    int a = val[valTop];
    valTop--;

    char op = opr[oprTop];
    oprTop--;

    valTop++;
    val[valTop] = calculate(a, op, b);
}

int main(){
    // char exp[] = "3-2*4";
    int number = 0;
    char exp[100];
    scanf("%s",exp);

    if(parser(exp) == 0){
        printf("Error: Invalid expression [parsing error :)]");
        return 1;
    }

    int i = 0;
    while(i < strlen(exp)){
        if(isDigit(exp[i]) == 1){
            number = number*10 + exp[i] - '0';
            i++;
        }
        else if(isDigit(exp[i]) == 0){
            valTop++;
            val[valTop] = number;
            number = 0;

            while(oprTop != -1 && precedence(opr[oprTop]) >= precedence(exp[i])){
                solveTop();
            }

            oprTop++;
            opr[oprTop] = exp[i];

            i++;
        }
        else{
            i++;
        }
    }

    valTop++;
    val[valTop] = number;

    while(oprTop != -1){
        solveTop();
    }

    printf("Result = %d", val[valTop]);
    return 0;
}
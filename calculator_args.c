#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_EXPR_LEN 100

char available_items[] = "0123456789+-*/()";

int precedence(char op) {
    if(op == '+' || op == '-')
        return 1;
    if(op == '*' || op == '/')
        return 2;
    return 0;
}

double applyOperation(double a, double b, char op) {
    switch(op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
    }
    return 0;
}

double evaluateExpression(const char* expression) {
    double values[MAX_EXPR_LEN];
    char ops[MAX_EXPR_LEN];
    int valTop = -1, opsTop = -1;

    for(int i = 0; expression[i]; i++) {
        if(expression[i] == ' ')
            continue;

        if(isdigit(expression[i]) || expression[i] == '.') {
            char buffer[16];
            int bufIndex = 0;

            while(i < strlen(expression) && (isdigit(expression[i]) || expression[i] == '.')) {
                buffer[bufIndex++] = expression[i++];
            }
            buffer[bufIndex] = '\0';
            i--;

            values[++valTop] = atof(buffer);
        } else if(expression[i] == '(') {
            ops[++opsTop] = expression[i];
        } else if(expression[i] == ')') {
            while(opsTop != -1 && ops[opsTop] != '(') {
                double val2 = values[valTop--];
                double val1 = values[valTop--];
                char op = ops[opsTop--];
                values[++valTop] = applyOperation(val1, val2, op);
            }
            opsTop--; 
        } else {
            while(opsTop != -1 && precedence(ops[opsTop]) >= precedence(expression[i])) {
                double val2 = values[valTop--];
                double val1 = values[valTop--];
                char op = ops[opsTop--];
                values[++valTop] = applyOperation(val1, val2, op);
            }
            ops[++opsTop] = expression[i];
        }
    }

    while(opsTop != -1) {
        double val2 = values[valTop--];
        double val1 = values[valTop--];
        char op = ops[opsTop--];
        values[++valTop] = applyOperation(val1, val2, op);
    }

    return values[valTop];
}

int main(int argc, char** argv) {
    char* expression = (char*) malloc(0);

    for (int i = 1; i < argc; i++) {
        char* addiction_string = argv[i];
        size_t addiction_string_len = strlen(addiction_string);

        char* full_addiction_string = (char*) malloc(addiction_string_len + 2);

        strcpy(full_addiction_string, addiction_string);

        if (i != argc-1) {
            strcat(full_addiction_string, " ");
        } else {
            full_addiction_string[addiction_string_len + 1] = '\0';
        }

        full_addiction_string[addiction_string_len + 2] = '\0';

        // add full_addiction_string to full_message
        size_t total_len = strlen(full_addiction_string) + strlen(expression) + 1;

        char* new_message = (char*) malloc(total_len);
        strcpy(new_message, expression);
        strcat(new_message, full_addiction_string);

        new_message[total_len] = '\0';
        expression = (char*) malloc(total_len);

        strcpy(expression, new_message);
        free(new_message);
        free(full_addiction_string);
    }

    int is_good = 1;
    for (int i = 0; expression[i]; i++) {
        if (strchr(available_items, expression[i]) == NULL && !isspace(expression[i])) {
            printf("Invalid symbol.\n");
            is_good = 0;
            break;
        }
    }

    if (!is_good || strlen(expression) == 0) {
        free(expression);
        return 1;
    }

    double result = evaluateExpression(expression);
    printf("Result: %f\n", result);
    free(expression);

    return 0;
}

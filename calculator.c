#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

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
    char* expression;
    while (1) {
        expression = readline("Enter expression (press ctrl+C to exit): ");
        if (expression == NULL) {
            break;
        }

        add_history(expression);

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
            continue;
        }

        double result = evaluateExpression(expression);
        printf("Result: %f\n", result);
        free(expression);
    }

    return 0;
}

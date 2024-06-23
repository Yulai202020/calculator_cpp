#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <readline/readline.h>
#include <readline/history.h>

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

double evaluateExpression(const std::string expression) {
    std::stack<double> values;
    std::stack<char> ops;

    for(size_t i = 0; i < expression.size(); i++) {
        if(expression[i] == ' ')
            continue;

        if(isdigit(expression[i])) {
            std::stringstream ss;
            while(i < expression.size() && (isdigit(expression[i]) || expression[i] == '.')) {
                ss << expression[i++];
            }
            i--;
            double val;
            ss >> val;
            values.push(val);
        }
        else if(expression[i] == '(') {
            ops.push(expression[i]);
        }
        else if(expression[i] == ')') {
            while(!ops.empty() && ops.top() != '(') {
                double val2 = values.top();
                values.pop();
                double val1 = values.top();
                values.pop();
                char op = ops.top();
                ops.pop();
                values.push(applyOperation(val1, val2, op));
            }
            ops.pop();
        }
        else {
            while(!ops.empty() && precedence(ops.top()) >= precedence(expression[i])) {
                double val2 = values.top();
                values.pop();
                double val1 = values.top();
                values.pop();
                char op = ops.top();
                ops.pop();
                values.push(applyOperation(val1, val2, op));
            }
            ops.push(expression[i]);
        }
    }

    while(!ops.empty()) {
        double val2 = values.top();
        values.pop();
        double val1 = values.top();
        values.pop();
        char op = ops.top();
        ops.pop();
        values.push(applyOperation(val1, val2, op));
    }

    return values.top();
}

int main(int argc, char** argv) {
    std::string expression;

    for (;;) {
        expression = readline("Enter expression (press ctrl+C for exit): ");
        add_history(expression.c_str());

        expression.erase(std::remove_if(expression.begin(), expression.end(), [](unsigned char c) { return std::isspace(c); }), expression.end());

        if (expression == "") {
            continue;
        }

        double result = evaluateExpression(expression);
        std::cout << "Result: " << result << std::endl;
    }

    return 0;
}

#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <cctype>

using namespace std;

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

double evaluateExpression(const string& expression) {
    stack<double> values;
    stack<char> ops;

    for(size_t i = 0; i < expression.size(); i++) {
        if(expression[i] == ' ')
            continue;

        if(isdigit(expression[i])) {
            stringstream ss;
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
    string expression;
    for (;;) {
        cout << "Enter example (for exit type exit) : ";
        cin >> expression;

        if (expression == "exit"){
            break;
        }

        double result = evaluateExpression(expression);
        cout << "Result: " << result << endl;
    }

    return 0;
}
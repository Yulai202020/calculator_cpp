#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <readline/readline.h>
#include <readline/history.h>

#include <nlohmann/json.hpp>
#include "shunting-yard.h"

using json = nlohmann::json;
using namespace cparse;

// expression max lenght
#define MAX_EXPR_LEN 100
packToken power(const packToken& left, const packToken& right, evaluationData* data) {
    double lhs = left.asDouble();
    double rhs = right.asDouble();
    return std::pow(lhs, rhs);
}

struct MyStartup {
    MyStartup() {
        // TokenMap& global = TokenMap::default_global();
        // global["fibonacci"] = CppFunction(&fibonacci, args_t{"N"});

        opMap_t& opMap = calculator::Default().opMap;
        opMap.add({NUM, "^", NUM}, &power);
    }
} MyStartupInstance;

// available symbols
int main(int argc, char** argv) {
    // declare new function
    TokenMap vars;
    char* expression;

    // load file

    if (argc > 1) {
        std::string filename = argv[1];
        std::ifstream inputFile(filename);
        if (!inputFile.is_open()) {
            std::cerr << "Could not open the file!" << std::endl;
            return 1;
        }

        // load variables like a, b
        json j;
        inputFile >> j;
    
        for (auto& [key, value] : j.items()) {
            try {
                double intValue = value.get<double>();  // attempt to convert to int
                vars[key] = intValue;
            } catch (const nlohmann::json::type_error& e) {
                std::cout << key << " is not an int, skipping or handling differently." << std::endl;
            }
        }
    }


    while (1) {
        // get expression
        expression = readline("Enter expression (press ctrl+C to exit): ");
        if (expression == NULL) {
            break;
        }

        // add in history
        add_history(expression);

        // get and print result
        calculator c1(expression);
        std::cout << "Result: " << c1.eval(vars) << std::endl;

        free(expression);
    }

    return 0;
}
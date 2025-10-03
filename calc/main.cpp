#include <string>
#include <fstream>
#include <iostream>
#include <regex>
#include <cstdint>
#include <vector>
#include <readline/readline.h>
#include <readline/history.h>

#include <nlohmann/json.hpp>
#include "headers/shunting-yard.h"
// my lib
#include "cef/cef_lib.h"

using json = nlohmann::json;
using namespace cparse;

// expression max lenght
#define MAX_EXPR_LEN 100

// 
bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
           str.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// custom operators
packToken power(const packToken& left, const packToken& right, evaluationData* data) {
    double lhs = left.asDouble();
    double rhs = right.asDouble();

    return std::pow(lhs, rhs);
}

packToken match(const packToken& left, const packToken& right, evaluationData* data) {
    std::string text = left.asString();
    std::string pattern_string = right.asString();

    std::regex pattern(pattern_string);

    std::smatch matches;

    std::sregex_iterator begin(text.begin(), text.end(), pattern);
    std::sregex_iterator end;

    std::vector<std::string> results;
    for (auto it = begin; it != end; ++it) {
        results.push_back(it->str());
    }

    // Join with \n
    std::ostringstream oss;
    for (size_t i = 0; i < results.size(); ++i) {
        oss << results[i];
        if (i < results.size() - 1) {
            oss << "\n";
        }
    }

    return oss.str();
}

packToken unary_not(const packToken& left, const packToken& value, evaluationData* data) {
    bool rhs = value.asBool();

    return !rhs;
}

// functions
packToken contains(TokenMap scope) {
    std::string str = scope["str"].asString();
    std::string sub = scope["sub"].asString();

    bool found = str.find(sub) != std::string::npos;
    return packToken(found ? 1 : 0);
}

packToken startswith(TokenMap scope) {
    std::string str = scope["str"].asString();
    std::string sub = scope["sub"].asString();

    bool found = starts_with(str, sub);
    return packToken(found ? 1 : 0);
}

packToken endswith(TokenMap scope) {
    std::string str = scope["str"].asString();
    std::string sub = scope["sub"].asString();

    bool found = ends_with(str, sub);
    return packToken(found ? 1 : 0);
}

packToken fibonacci(TokenMap scope) {
    int n = scope["N"].asInt();

    if (n <= 1) return n;

    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int tmp = a + b;
        a = b;
        b = tmp;
    }
    return b;
}

struct MyStartup {
    MyStartup() {
        TokenMap& global = TokenMap::default_global();
        global["fibonacci"] = CppFunction(&fibonacci, args_t{"N"});
        global["contains"] = CppFunction(&contains, args_t{"str", "sub"});
        global["startswith"] = CppFunction(&startswith, args_t{"str", "sub"});
        global["endswith"] = CppFunction(&endswith, args_t{"str", "sub"});

        opMap_t& opMap = calculator::Default().opMap;
        opMap.add({NUM, "^", NUM}, &power);
        opMap.add({NUM, "~", NUM}, &match);
        opMap.add({NONE, "!", NUM}, &unary_not);
    }
} MyStartupInstance;

void calculate(const char* expression, TokenMap vars) {
    calculator c1(expression);
    std::cout << "Result: " << c1.eval(vars) << std::endl;
}

int add_vars(TokenMap& vars, std::string filename) {
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
            double intValue = value.get<double>();
            vars[key] = intValue;
        } catch (const nlohmann::json::type_error& e) {
            std::cout << key << " is not an int, skipping." << std::endl;
        }
    }

    return 0;
}

// available symbols
int main(int argc, char** argv) {
    // declare new function
    TokenMap vars;
    char* expression;

    // load file
    if (argc > 1) {
        std::string filename = argv[1];

        if (add_vars(vars, filename)) {
            return 1;
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

        if (starts_with(expression, "exec")) {
            std::string filename = ((std::string) expression).substr(5);
            std::filesystem::path input_filepath(filename);

            if (input_filepath.extension() == ".json") {
                std::ifstream inputFile(filename);
                            
                if (!inputFile.is_open()) {
                    std::cerr << "Could not open the file!" << std::endl;
                    continue;
                }

                json j;
                inputFile >> j;

                std::vector<std::string> values = j.get<std::vector<std::string>>();

                // Print list
                for (std::string value : values) {
                    calculate(value.c_str(), vars);
                }
            } else if (input_filepath.extension() == ".log") {
                vector<CEFEvent> events = parse_file(filename);

                print_events(events);
            } else {
                cout << "Unknown file type." << std::endl;
            }

        } else if (starts_with(expression, "load_json")) {
            std::string filename = ((std::string) expression).substr(5);

            if (add_vars(vars, filename)) {
                return 1;
            }

            std::cout << "Loaded variables from file." << std::endl;
        } else {
            // get and print result
            calculate(expression, vars);
        }

        free(expression);
    }

    return 0;
}
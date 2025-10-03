#include "cef_lib.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include <bits/stdc++.h>
#include <nlohmann/json.hpp>

using namespace std;
using namespace nlohmann;

// Unescapes CEF-style sequences like \=, \|, \\ and \n to their original characters.
string cef_unescape(const string &s) {
    string out;

    // pre-allocate memory (get memory)
    out.reserve(s.size());

    // cycle for a string
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];

        // if its back slash and here more symbol
        if (c == '\\' && i + 1 < s.size()) {
            // next symbol
            char n = s[i+1];

            // double backslash, back slash with | and back slash with = turn to original
            if (n == '\\' || n == '|' || n == '=' ) {
                out.push_back(n);
                ++i;
                continue;
            // if its back slash n turn into new line
            } else if (n == 'n') {
                out.push_back('\n');
                ++i;
                continue;
            }

            // else add symbol
            out.push_back(n);
            ++i;

            continue;
        }

        // idk
        out.push_back(c);
    }

    // return output
    return out;
}

// Parse header
bool parse_header_fields(const string& s, CEFEvent& evt, size_t& pos_after_header) {
    // if its not starts with CEF: return false (not valid)
    if (s.rfind("CEF:", 0) != 0) return false;

    // including version
    size_t i = 4;

    size_t next = s.find('|', i);
    // not empty string
    if (next == string::npos) return false;
    // full version string (with CEF)
    string verstr = s.substr(i, next - i);
    try {
        // get version (convert to int)
        evt.version = stoi(verstr);
    } catch (...) {
        return false;
    }

    // skip |
    i = next + 1;

    // create var
    vector<string> fields;

    fields.reserve(6);

    for (int f = 0; f < 6; ++f) {
        // find next |
        next = s.find('|', i);
        // its unvalid if in string no | its not valid
        if (next == string::npos) return false;
        // get raw data
        string raw = s.substr(i, next - i);
        // write parsed data
        fields.push_back(cef_unescape(raw));

        // 
        i = next + 1;
    }

    // create object
    evt.deviceVendor = fields[0];
    evt.deviceProduct = fields[1];
    evt.deviceVersion = fields[2];
    evt.signatureID = fields[3];
    evt.name = fields[4];
    evt.severity = fields[5];

    // change latest pose (for parsing extensions)
    pos_after_header = i;

    // return object
    return true;
}

// Parse extension -
unordered_map<string, string> parse_extension(const string& s, size_t pos) {
    // init output
    unordered_map<string, string> out;
    // set lenght
    size_t n = s.size();
    // set pose
    size_t i = pos;

    // Skip initial spaces
    while (i < n && isspace((unsigned char)s[i])) ++i;

    // parse extensions
    while (i < n) {
        // extract keys
        size_t key_start = i;
        // get to keys end position (space counts as end)
        while (i < n && s[i] != '=' && !isspace((unsigned char)s[i])) ++i;
        // break if we didnt found symbol =
        if (i >= n || s[i] != '=') break;
        // get key
        string key = s.substr(key_start, i - key_start);
        // skip =
        ++i;

        // init vars
        string rawval;
        bool in_escape = false;
        while (i < n) {
            // get current char
            char c = s[i];

            if (!in_escape && c == '\\') {
                in_escape = true;
                // add back slash
                rawval.push_back('\\');
                ++i;
                continue;
            }
            if (!in_escape && c == ' ') {
                size_t j = i + 1;
                while (j < n && s[j] == ' ') ++j;
                size_t k = j;
                bool foundEq = false;
                while (k < n && s[k] != ' ') {
                    if (s[k] == '=') { foundEq = true; break; }
                    ++k;
                }
                if (foundEq) break;
                rawval.push_back(' ');
                i = j;
                continue;
            }
            rawval.push_back(c);
            in_escape = false;
            ++i;
        }

        // get value
        string val = cef_unescape(rawval);
        // add to output
        out.emplace(move(key), move(val));

        // skip spaces
        while (i < n && isspace((unsigned char)s[i])) ++i;
    }

    // return output
    return out;
}

// Parse cef line
CEFEvent parse_cef_line(const string& line) {
    // create object
    CEFEvent evt;
    // create position
    size_t pos = 0;

    // check is cef line valid
    if (!parse_header_fields(line, evt, pos)) {
        // return object with invalid option
        evt.valid = false;
        return evt;
    }

    // parse extensions
    evt.extension = parse_extension(line, pos);
    evt.valid = true;

    // return object
    return evt;
}

// Print data of CEFEvent
void print_event(const CEFEvent e) {
    // check if is event valid
    if (!e.valid) {
        cout << "<invalid CEF>\n";
        return;
    }

    // printing data
    cout << "version: " << e.version << "\n";
    cout << "deviceVendor: " << e.deviceVendor << "\n";
    cout << "deviceProduct: " << e.deviceProduct << "\n";
    cout << "deviceVersion: " << e.deviceVersion << "\n";
    cout << "signatureID: " << e.signatureID << "\n";
    cout << "name: " << e.name << "\n";
    cout << "severity: " << e.severity << "\n";
    cout << "extensions (" << e.extension.size() << "):\n";

    // extensions
    for (auto &kv : e.extension) {
        cout << "  " << kv.first << " = '" << kv.second << "'\n";
    }
}

// print list of events
void print_events(const vector<CEFEvent> events) {
    for (CEFEvent e : events) {
        print_event(e);
    }
}

// File reading
vector<CEFEvent> parse_file(const string filename) {
    // get file
    ifstream in(filename);
    if (!in) {
        cerr << "Cannot open file: " << filename << "\n";
        return {};
    }

    // read file line by line and parse it
    string line;
    size_t lineno = 0;
    vector<CEFEvent> list;

    while (getline(in, line)) {
        ++lineno;
        if (line.empty()) continue;
        cout << "--- Line " << lineno << " ---\n";

        // get event
        CEFEvent e = parse_cef_line(line);

        list.push_back(e);
    }

    return list;
}

// convert cef file to json file
void convert_cef_to_json_file(const string input_filename, const string output_filename) {
    // make json array
    json j = json::array();
    vector<CEFEvent> events = parse_file(input_filename);

    // convert cef struct to json struct
    for (CEFEvent e : events) {
        json tmp;
        tmp["deviceProduct"] = e.deviceProduct;
        j.push_back(tmp);
    }

    // dump json and write to file
    ofstream output_file(output_filename);
    string str = j.dump();
    output_file << str;
}

// Basic tests
void test_one(const string &line, const string &desc) {
    cout << "---\nTest: " << desc << "\nLine: " << line << "\n";
    auto e = parse_cef_line(line);
    print_event(e);
}
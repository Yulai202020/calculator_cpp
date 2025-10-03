// cef_lib.h
#ifndef CEF_LIB_H_
#define CEF_LIB_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

using namespace std;
using namespace nlohmann;

// CEF String example
// CEF:<version>|<deviceVendor>|<deviceProduct>|<deviceVersion>|<signatureID>|<name>|<severity>|(extensions as key=value and devided by space)

struct CEFEvent {
    int version = 0;
    string deviceVendor;
    string deviceProduct;
    string deviceVersion;
    string signatureID;
    string name;
    string severity;
    unordered_map<string,string> extension;
    bool valid = false;
};

string cef_unescape(const string &s);
bool parse_header_fields(const string& s, CEFEvent& evt, size_t& pos_after_header);
unordered_map<string,string> parse_extension(const string& s, size_t pos);
CEFEvent parse_cef_line(const string& line);
void print_event(const CEFEvent e);
void print_events(const vector<CEFEvent> events);
vector<CEFEvent> parse_file(const string filename);
void convert_cef_to_json_file(const string input_filename, const string output_filename);
void test_one(const string &line, const string &desc);

#endif
